/*
 * HyPerLayer.cpp
 *
 *  Created on: Jul 29, 2008
 *
 */

#include "HyPerLayer.hpp"
#include "../include/pv_common.h"
#include "../include/default_params.h"
#include "../columns/HyPerCol.hpp"
#include "../io/fileio.hpp"
#include "../io/imageio.hpp"
#include "../io/io.h"
#include <assert.h>
#include <string.h>

namespace PV {

HyPerLayerParams defaultParams =
{
    V_REST, V_EXC, V_INH, V_INHB,            // V (mV)
    TAU_VMEM, TAU_EXC, TAU_INH, TAU_INHB,
    VTH_REST,  TAU_VTH, DELTA_VTH,           // tau (ms)
    250, NOISE_AMP*( 1.0/TAU_EXC ) * ( ( TAU_INH * (V_REST-V_INH) + TAU_INHB * (V_REST-V_INHB) ) / (V_EXC-V_REST) ),
    250, NOISE_AMP*1.0,
    250, NOISE_AMP*1.0                       // noise (G)
};

///////
// This constructor is protected so that only derived classes can call it.
// It should be called as the normal method of object construction by
// derived classes.  It should NOT call any virtual methods
HyPerLayer::HyPerLayer(const char* name, HyPerCol * hc)
{
   initialize_base(name, hc);
}

HyPerLayer::~HyPerLayer()
{
   if (clayer != NULL) {
      // pvlayer_finalize will free clayer
      pvlayer_finalize(clayer);
      clayer = NULL;
   }
   free(name);

   if (parent->columnId() == 0) {
      printf("%16s: total time in %6s %10s: ", name, "layer", "update");
      update_timer->elapsed_time();
      delete update_timer;
   }
}

/**
 * Primary method for derived layer initialization.  This should be called
 * after initialize_base has been called.  WARNING, very little should be
 * done here as it should be done in derived class.
 */
int HyPerLayer::initialize(PVLayerType type)
{
   int status = 0;
   float time = 0.0f;

   // IMPORTANT:
   //   - these two statements should be done in all derived classes
   //
   clayer->layerType = type;
   parent->addLayer(this);

   if (parent->parameters()->value(name, "restart", 0.0f) != 0.0f) {
      readState(name, &time);
   }

   return status;
}

int HyPerLayer::initialize_base(const char * name, HyPerCol * hc)
{
   PVLayerLoc layerLoc;

   // name should be initialized first as other methods may use it
   this->name = strdup(name);
   setParent(hc);

   this->probes = NULL;
   this->ioAppend = 0;
   this->numProbes = 0;

   this->update_timer = new Timer();

   PVParams * params = parent->parameters();

   if (params->present(name, "nx") || params->present(name, "ny")) {
      fprintf(stderr, "HyPerLayer::initialize_base: ERROR, use (nxScale,nyScale) not (nx,ny)\n");
      exit(-1);
   }

   const float nxScale = params->value(name, "nxScale", 1.0f);
   const float nyScale = params->value(name, "nyScale", 1.0f);

   const int numFeatures = (int) params->value(name, "nf", 1);
   const int margin      = (int) params->value(name, "marginWidth", 0);

   hc->setLayerLoc(&layerLoc, nxScale, nyScale, margin, numFeatures);

   float xScalef = -log2f( (float) nxScale);
   float yScalef = -log2f( (float) nyScale);

   int xScale = (int) nearbyintf(xScalef);
   int yScale = (int) nearbyintf(yScalef);

   clayer = pvlayer_new(layerLoc, xScale, yScale);
   clayer->layerType = TypeGeneric;

   writeTime = parent->simulationTime();
   writeStep = params->value(name, "writeStep", parent->getDeltaTime());

   mirrorBCflag = (bool) params->value(name, "mirrorBCflag", 0);

   return 0;
}

#ifdef PV_USE_OPENCL
/**
 * Initialize OpenCL buffers.  This must be called after PVLayer data have
 * been allocated.
 */
int HyPerLayer::initializeThreadBuffers()
{
   int status = CL_SUCCESS;

   const size_t size    = clayer->numNeurons  * sizeof(pvdata_t);
   const size_t size_ex = clayer->numExtended * sizeof(pvdata_t);

   CLDevice * device = parent->getCLDevice();

   // these buffers are shared between host and device
   //
   clBuffers.V    = device->createBuffer(CL_MEM_COPY_HOST_PTR, size, clayer->V);
   clBuffers.Vth  = device->createBuffer(CL_MEM_COPY_HOST_PTR, size, clayer->Vth);
   clBuffers.G_E  = device->createBuffer(CL_MEM_COPY_HOST_PTR, size, clayer->G_E);
   clBuffers.G_I  = device->createBuffer(CL_MEM_COPY_HOST_PTR, size, clayer->G_I);
   clBuffers.G_IB = device->createBuffer(CL_MEM_COPY_HOST_PTR, size, clayer->G_IB);

   clBuffers.phi = device->createBuffer(CL_MEM_COPY_HOST_PTR, clayer->numPhis*size, clayer->phi[0]);
   clBuffers.activity = device->createBuffer(CL_MEM_COPY_HOST_PTR, size_ex, clayer->activity->data);
   clBuffers.prevActivity = device->createBuffer(CL_MEM_COPY_HOST_PTR, size_ex, clayer->prevActivity);

   return status;
}

int HyPerLayer::initializeThreadData()
{
   return CL_SUCCESS;
}
#endif

/**
 * Initialize a few things that require a layer id
 */
int HyPerLayer::initializeLayerId(int layerId)
{
   char filename[PV_PATH_MAX];
   bool append = false;

   setLayerId(layerId);

   sprintf(filename, "%s/a%d.pvp", OUTPUT_PATH, clayer->layerId);
   clayer->activeFP = pvp_open_write_file(filename, parent->icCommunicator(), append);

   return 0;
}

#ifdef DEPRECATED
int HyPerLayer::initializeGlobal(int colId, int colRow, int colCol, int nRows, int nCols)
{
   char filename[PV_PATH_MAX];
   bool append = false;
   int status = 0;

   sprintf(filename, "%s/a%d.pvp", OUTPUT_PATH, clayer->layerId);
   clayer->activeFP = pvp_open_write_file(filename, parent->icCommunicator(), append);

#ifdef DEPRECATED
   status = pvlayer_initGlobal(clayer, colId, colRow, colCol, nRows, nCols);
#endif

   initializeThreadBuffers();

   return status;
}
#endif // DEPRECATED

int HyPerLayer::columnWillAddLayer(InterColComm * comm, int layerId)
{
   clayer->columnId = parent->columnId();
   initializeLayerId(layerId);

#ifdef DEPRECATED
   // complete initialization now that we have a parent and a communicator
   // WARNING - this must be done before addPublisher is called
   int id = parent->columnId();
   initializeGlobal(id, comm->commRow(), comm->commColumn(),
                        comm->numCommRows(), comm->numCommColumns());
#endif // DEPRECATED

   comm->addPublisher(this, clayer->activity->numItems, MAX_F_DELAY);

   return 0;
}

int HyPerLayer::initFinish()
{
   int status = pvlayer_initFinish(clayer);
   assert(clayer->numFeatures == getLayerLoc()->nBands);
   return status;
}

/**
 * Returns the activity data for the layer.  This data is in the
 * extended space (with margins).
 */
const pvdata_t * HyPerLayer::getLayerData()
{
   DataStore * store = parent->icCommunicator()->publisherStore(getLayerId());
   return (pvdata_t *) store->buffer(LOCAL);
}


// deprecated?
/**
 * returns the number of neurons in the layer or border region
 * @param borderId the id of the border region (0 for interior/self)
 **/
int HyPerLayer::numberOfNeurons(int borderId)
{
   int numNeurons = 0;
   const int nx = clayer->loc.nx;
   const int ny = clayer->loc.ny;
   const int nf = clayer->numFeatures;
   const int nxBorder = clayer->loc.nPad;
   const int nyBorder = clayer->loc.nPad;

   switch (borderId) {
   case 0:
      numNeurons = clayer->numNeurons;         break;
   case NORTHWEST:
      numNeurons = nxBorder * nyBorder * nf;   break;
   case NORTH:
      numNeurons = nx       * nyBorder * nf;   break;
   case NORTHEAST:
      numNeurons = nxBorder * nyBorder * nf;   break;
   case WEST:
      numNeurons = nxBorder * ny       * nf;   break;
   case EAST:
      numNeurons = nxBorder * ny       * nf;   break;
   case SOUTHWEST:
      numNeurons = nxBorder * nyBorder * nf;   break;
   case SOUTH:
      numNeurons = nx       * nyBorder * nf;   break;
   case SOUTHEAST:
      numNeurons = nxBorder * nyBorder * nf;   break;
   default:
      fprintf(stderr, "ERROR:HyPerLayer:numberOfBorderNeurons: bad border index %d\n", borderId);
   }

   return numNeurons;
}


/**
 * Copy cube data to the border region while applying boundary conditions
 *   - this implements mirror boundary conditions
 *   - assumes both input PVLayerCubes are of identical size and shape, typically the same struct
 */
int HyPerLayer::mirrorInteriorToBorder(int whichBorder, PVLayerCube * cube, PVLayerCube * border)
{
   assert( cube->numItems == border->numItems );
   assert( cube->loc.nx == border->loc.nx );
   assert( cube->loc.ny == border->loc.ny );
   assert( cube->loc.nBands == border->loc.nBands );

   switch (whichBorder) {
   case NORTHWEST:
      return mirrorToNorthWest(border, cube);
   case NORTH:
      return mirrorToNorth(border, cube);
   case NORTHEAST:
      return mirrorToNorthEast(border, cube);
   case WEST:
      return mirrorToWest(border, cube);
   case EAST:
      return mirrorToEast(border, cube);
   case SOUTHWEST:
      return mirrorToSouthWest(border, cube);
   case SOUTH:
      return mirrorToSouth(border, cube);
   case SOUTHEAST:
      return mirrorToSouthEast(border, cube);
   default:
      fprintf(stderr, "ERROR:HyPerLayer:copyToBorder: bad border index %d\n", whichBorder);
   }

   return -1;
}

int HyPerLayer::gatherToInteriorBuffer(unsigned char * buf)
{
   return HyPerLayer::copyToBuffer(buf, getLayerData(), getLayerLoc(), isExtended(), 255.0);
}

int HyPerLayer::copyToBuffer(unsigned char * buf, const pvdata_t * data,
                             const PVLayerLoc * loc, bool extended, float scale)
{
   const int nx = loc->nx;
   const int ny = loc->ny;
   const int nf = loc->nBands;

   int nxBorder = 0;
   int nyBorder = 0;

   if (extended) {
      nxBorder = loc->nPad;
      nyBorder = loc->nPad;
   }

   const int sf = 1;
   const int sx = nf * sf;
   const int sy = sx * (nx + 2*nxBorder);

   int ii = 0;
   for (int j = 0; j < ny; j++) {
      int jex = j + nyBorder;
      for (int i = 0; i < nx; i++) {
         int iex = i + nxBorder;
         for (int f = 0; f < nf; f++) {
            buf[ii++] = (unsigned char) (scale * data[iex*sx + jex*sy + f*sf]);
         }
      }
   }
   return 0;
}

int HyPerLayer::copyToBuffer(pvdata_t * buf, const pvdata_t * data,
                             const PVLayerLoc * loc, bool extended, float scale)
{
   const int nx = loc->nx;
   const int ny = loc->ny;
   const int nf = loc->nBands;

   int nxBorder = 0;
   int nyBorder = 0;

   if (extended) {
      nxBorder = loc->nPad;
      nyBorder = loc->nPad;
   }

   const int sf = 1;
   const int sx = nf * sf;
   const int sy = sx * (nx + 2*nxBorder);

   int ii = 0;
   for (int j = 0; j < ny; j++) {
      int jex = j + nyBorder;
      for (int i = 0; i < nx; i++) {
         int iex = i + nxBorder;
         for (int f = 0; f < nf; f++) {
            buf[ii++] = scale * data[iex*sx + jex*sy + f*sf];
         }
      }
   }
   return 0;
}

int HyPerLayer::copyFromBuffer(const unsigned char * buf, pvdata_t * data,
                               const PVLayerLoc * loc, bool extended, float scale)
{
   const int nx = loc->nx;
   const int ny = loc->ny;
   const int nf = loc->nBands;

   int nxBorder = 0;
   int nyBorder = 0;

   if (extended) {
      nxBorder = loc->nPad;
      nyBorder = loc->nPad;
   }

   const int sf = 1;
   const int sx = nf * sf;
   const int sy = sx * (nx + 2*nxBorder);

   int ii = 0;
   for (int j = 0; j < ny; j++) {
      int jex = j + nyBorder;
      for (int i = 0; i < nx; i++) {
         int iex = i + nxBorder;
         for (int f = 0; f < nf; f++) {
            data[iex*sx + jex*sy + f*sf] = scale * (pvdata_t) buf[ii++];
         }
      }
   }
   return 0;
}

int HyPerLayer::copyFromBuffer(const pvdata_t * buf, pvdata_t * data,
                               const PVLayerLoc * loc, bool extended, float scale)
{
   const int nx = loc->nx;
   const int ny = loc->ny;
   const int nf = loc->nBands;

   int nxBorder = 0;
   int nyBorder = 0;

   if (extended) {
      nxBorder = loc->nPad;
      nyBorder = loc->nPad;
   }

   const int sf = 1;
   const int sx = nf * sf;
   const int sy = sx * (nx + 2*nxBorder);

   int ii = 0;
   for (int j = 0; j < ny; j++) {
      int jex = j + nyBorder;
      for (int i = 0; i < nx; i++) {
         int iex = i + nxBorder;
         for (int f = 0; f < nf; f++) {
            data[iex*sx + jex*sy + f*sf] = scale * buf[ii++];
         }
      }
   }
   return 0;
}

int HyPerLayer::updateState(float time, float dt)
{
   // just copy accumulation buffer to membrane potential
   // and activity buffer (nonspiking)

   updateV();
   setActivity();
   resetPhiBuffers();

   return 0;
}

int HyPerLayer::updateV() {
   pvdata_t * V = getV();
   pvdata_t ** phi = getCLayer()->phi;
   pvdata_t * phiExc = phi[PHI_EXC];
   pvdata_t * phiInh = phi[PHI_INH];
   for( int k=0; k<getNumNeurons(); k++ ) {
      V[k] = phiExc[k] - phiInh[k];
#undef SET_MAX
#ifdef SET_MAX
      V[k] = V[k] > 1.0f ? 1.0f : V[k];
#endif
#undef SET_THRESH
#ifdef SET_THRESH
      V[k] = V[k] < 0.5f ? 0.0f : V[k];
#endif
   }
   return EXIT_SUCCESS;
}

int HyPerLayer::setActivity() {
   const int nx = getLayerLoc()->nx;
   const int ny = getLayerLoc()->ny;
   const int nf = getCLayer()->numFeatures;
   const int marginWidth = getLayerLoc()->nPad;
   pvdata_t * activity = getCLayer()->activity->data;
   pvdata_t * V = getV();
   for( int k=0; k<getNumExtended(); k++ ) {
      activity[k] = 0; // Would it be faster to only do the margins?
   }
   for( int k=0; k<getNumNeurons(); k++ ) {
      int kex = kIndexExtended( k, nx, ny, nf, marginWidth );
      activity[kex] = V[k];
   }
   return EXIT_SUCCESS;
}

int HyPerLayer::resetPhiBuffers() {
   pvdata_t ** phi = getCLayer()->phi;
   int n = getNumNeurons();
   resetBuffer( phi[PHI_EXC], n );
   resetBuffer( phi[PHI_INH], n );
   return EXIT_SUCCESS;
}

int HyPerLayer::resetBuffer( pvdata_t * buf, int numItems ) {
   for( int k=0; k<numItems; k++ ) buf[k] = 0.0;
   return EXIT_SUCCESS;
}

int HyPerLayer::recvSynapticInput(HyPerConn * conn, PVLayerCube * activity, int neighbor)
{
   assert(neighbor >= 0);
   const int numExtended = activity->numItems;

#ifdef DEBUG_OUTPUT
   int rank;
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   printf("[%d]: HyPerLayr::recvSyn: neighbor=%d num=%d actv=%p this=%p conn=%p\n", rank, neighbor, numExtended, activity, this, conn);
   fflush(stdout);
#endif

   for (int kPre = 0; kPre < numExtended; kPre++) {
      float a = activity->data[kPre];

      // Activity < 0 is used by generative models --pete
      if (a == 0.0f) continue;  // TODO - assume activity is sparse so make this common branch

      PVAxonalArbor * arbor = conn->axonalArbor(kPre, neighbor);
      PVPatch * phi = arbor->data;
      PVPatch * weights = arbor->weights;

      // WARNING - assumes weight and phi patches from task same size
      //         - assumes patch stride sf is 1

      int nk  = phi->nf * phi->nx;
      int ny  = phi->ny;
      int sy  = phi->sy;        // stride in layer
      int syw = weights->sy;    // stride in patch

      // TODO - unroll
      for (int y = 0; y < ny; y++) {
         pvpatch_accumulate(nk, phi->data + y*sy, a, weights->data + y*syw);
//       if (err != 0) printf("  ERROR kPre = %d\n", kPre);
      }
   }

   return 0;
}

int HyPerLayer::reconstruct(HyPerConn * conn, PVLayerCube * cube)
{
   // TODO - implement
   printf("[%d]: HyPerLayer::reconstruct: to layer %d from %d\n",
          clayer->columnId, clayer->layerId, conn->preSynapticLayer()->clayer->layerId);
   return 0;
}

int HyPerLayer::publish(InterColComm* comm, float time)
{
   if ( useMirrorBCs() ) {
      for (int borderId = 1; borderId < NUM_NEIGHBORHOOD; borderId++){
         mirrorInteriorToBorder(borderId, clayer->activity, clayer->activity);
      }
   }
   comm->publish(this, clayer->activity);
   return 0;
}

int HyPerLayer::insertProbe(LayerProbe * p)
{
   LayerProbe ** tmp;
   tmp = (LayerProbe **) malloc((numProbes + 1) * sizeof(LayerProbe *));
   assert(tmp != NULL);

   for (int i = 0; i < numProbes; i++) {
      tmp[i] = probes[i];
   }
   delete probes;

   probes = tmp;
   probes[numProbes] = p;

   return ++numProbes;
}

int HyPerLayer::outputState(float time, bool last)
{
//   char path[PV_PATH_MAX];
   int status = 0;

//   const int nx = clayer->loc.nx;
//   const int ny = clayer->loc.ny;
//   const int nf = clayer->numFeatures;

//   const int nxex = clayer->loc.nx + 2*clayer->loc.nPad;
//   const int nyex = clayer->loc.ny + 2*clayer->loc.nPad;

   for (int i = 0; i < numProbes; i++) {
      probes[i]->outputState(time, this);
   }

   PVParams * params = parent->parameters();
   int spikingFlag = (int) params->value(name, "spikingFlag", 1);

#undef WRITE_NONSPIKING_ACTIVITY
#ifdef WRITE_NONSPIKING_ACTIVITY
   float defaultWriteNonspikingActivity = 1.0;
#else
   float defaultWriteNonspikingActivity = 0.0;
#endif

   int writeNonspikingActivity =
         (int) params->value(name, "writeNonspikingActivity", defaultWriteNonspikingActivity);

   if (spikingFlag != 0) {
      status = writeActivitySparse(time);
   }
   else if (writeNonspikingActivity) {
      status = writeActivity(time);
   }

   if (time >= writeTime) {
      writeTime += writeStep;
   }

   return status;
}

int HyPerLayer::readState(const char * name, float * time)
{
   int status = 0;
   char path[PV_PATH_MAX];

   double dtime;

   bool contiguous = false;
   bool extended   = false;

   Communicator * comm = parent->icCommunicator();

   const char * last = "_last";
   const char * name_str = (name != NULL) ? name : "";

   pvdata_t * G_E = clayer->G_E;
   pvdata_t * G_I = clayer->G_I;

   pvdata_t * V   = clayer->V;
   pvdata_t * Vth = clayer->Vth;

   // TODO - this should be moved to getLayerData but can't yet because publish is call
   // as the first step and publish copies clayer->activity->data into data store.  If
   // clayer->activity is removed then we would read directly into data store.
   pvdata_t * A = clayer->activity->data;

   PVLayerLoc * loc = & clayer->loc;

   snprintf(path, PV_PATH_MAX-1, "%s%s_GE%s.pvp", OUTPUT_PATH, name_str, last);
   status = read(path, comm, &dtime, G_E, loc, PV_FLOAT_TYPE, extended, contiguous);

   snprintf(path, PV_PATH_MAX-1, "%s%s_GI%s.pvp", OUTPUT_PATH, name_str, last);
   status = read(path, comm, &dtime, G_I, loc, PV_FLOAT_TYPE, extended, contiguous);

   snprintf(path, PV_PATH_MAX-1, "%s%s_V%s.pvp", OUTPUT_PATH, name_str, last);
   status = read(path, comm, &dtime, V, loc, PV_FLOAT_TYPE, extended, contiguous);

   snprintf(path, PV_PATH_MAX-1, "%s%s_Vth%s.pvp", OUTPUT_PATH, name_str, last);
   status = read(path, comm, &dtime, Vth, loc, PV_FLOAT_TYPE, extended, contiguous);

   extended = true;
   snprintf(path, PV_PATH_MAX-1, "%s%s_A%s.pvp", OUTPUT_PATH, name_str, last);
   status = read(path, comm, &dtime, A, loc, PV_FLOAT_TYPE, extended, contiguous);

   return status;
}

int HyPerLayer::writeState(const char * name, float time, bool last)
{
   int status = 0;
   char path[PV_PATH_MAX];

   bool contiguous = false;
   bool extended   = false;

   Communicator * comm = parent->icCommunicator();

   const char * last_str = (last) ? "_last" : "";
   const char * name_str = (name != NULL) ? name : "";

   pvdata_t * G_E  = clayer->G_E;
   pvdata_t * G_I  = clayer->G_I;

   pvdata_t * V   = clayer->V;
   pvdata_t * Vth = clayer->Vth;

   const pvdata_t * A = getLayerData();

   PVLayerLoc * loc = & clayer->loc;

   snprintf(path, PV_PATH_MAX-1, "%s%s_GE%s.pvp", OUTPUT_PATH, name_str, last_str);
   status = write(path, comm, time, G_E, loc, PV_FLOAT_TYPE, extended, contiguous);

   snprintf(path, PV_PATH_MAX-1, "%s%s_GI%s.pvp", OUTPUT_PATH, name_str, last_str);
   status = write(path, comm, time, G_I, loc, PV_FLOAT_TYPE, extended, contiguous);

   snprintf(path, PV_PATH_MAX-1, "%s%s_V%s.pvp", OUTPUT_PATH, name_str, last_str);
   status = write(path, comm, time, V, loc, PV_FLOAT_TYPE, extended, contiguous);

   snprintf(path, PV_PATH_MAX-1, "%s%s_Vth%s.pvp", OUTPUT_PATH, name_str, last_str);
   status = write(path, comm, time, Vth, loc, PV_FLOAT_TYPE, extended, contiguous);

   extended = true;
   snprintf(path, PV_PATH_MAX-1, "%s%s_A%s.pvp", OUTPUT_PATH, name_str, last_str);
   status = write(path, comm, time, A, loc, PV_FLOAT_TYPE, extended, contiguous);

   return status;
}

int HyPerLayer::writeActivitySparse(float time)
{
   return PV::writeActivitySparse(clayer->activeFP, parent->icCommunicator(), time, clayer);
}

// write non-spiking activity
int HyPerLayer::writeActivity(float time)
{
   return PV::writeActivity(clayer->activeFP, parent->icCommunicator(), time, clayer);
}

// modified to enable writing of non-spiking activity as well
// use writeActivitySparse for efficient disk storage of sparse spiking activity
int HyPerLayer::writeActivity(const char * filename, float time)
{
   int status = 0;
   PVLayerLoc * loc = &clayer->loc;

   const int n = loc->nx * loc->ny * clayer->numFeatures;
   pvdata_t * buf = new pvdata_t[n];
   assert(buf != NULL);

   const bool extended = true;
   status = copyToBuffer(buf, getLayerData(), loc, extended, 1.0);

   // gather the local portions and write the image
   status = gatherImageFile(filename, parent->icCommunicator(), loc, buf);

   delete buf;

   return status;
}

int HyPerLayer::setParams(int numParams, size_t sizeParams, float * params)
{
   return pvlayer_setParams(clayer, numParams, sizeParams, params);
}

int HyPerLayer::setFuncs(void * initFunc, void * updateFunc)
{
   return pvlayer_setFuncs(clayer, initFunc, updateFunc);
}

int HyPerLayer::getParams(int * numParams, float ** params)
{
   return pvlayer_getParams(clayer, numParams, params);
}

/* copy src PVLayerCube to dest PVLayerCube */
/* initialize src, dest to beginning of data structures */
int copyDirect(pvdata_t * dest, pvdata_t * src, int nf, int nxSrc, int nySrc, int syDst, int sySrc)
{
   pvdata_t * to   = dest;
   pvdata_t * from = src;

   for (int j = 0; j < nySrc; j++) {
      to   = dest + j*syDst;
      from = src  + j*sySrc;
      for (int i = 0; i < nxSrc; i++) {
         for (int f = 0; f < nf; f++) {
            to[f] = from[f];
         }
         to   += nf;
         from += nf;
      }
   }
   return 0;
}

int HyPerLayer::mirrorToNorthWest(PVLayerCube * dest, PVLayerCube * src)
{
   int nx = clayer->loc.nx;
   int nf = clayer->numFeatures;
   int npad = dest->loc.nPad;
   int sy = nf * ( nx + 2*npad );

   pvdata_t * src0 = src-> data + npad * sy + npad * nf;
   pvdata_t * dst0 = dest->data + (npad-1) * sy + (npad-1) * nf;

   for (int ky = 0; ky < npad; ky++) {
      pvdata_t * to   = dst0 - ky*sy;
      pvdata_t * from = src0 + ky*sy;
      for (int kx = 0; kx < npad; kx++) {
         for (int kf = 0; kf < nf; kf++) {
            to[kf] = from[kf];
         }
         to -= nf;
         from += nf;
      }
   }
   return 0;
}

int HyPerLayer::mirrorToNorth(PVLayerCube * dest, PVLayerCube * src)
{
   int nx = clayer->loc.nx;
   int nf = clayer->numFeatures;
   int npad = dest->loc.nPad;
   int sy = nf * ( nx + 2*npad );

   pvdata_t * src0 = src-> data + npad * sy + npad * nf;
   pvdata_t * dst0 = dest->data + (npad-1) * sy + npad * nf;

   for (int ky = 0; ky < npad; ky++) {
      pvdata_t * to   = dst0 - ky*sy;
      pvdata_t * from = src0 + ky*sy;
      for (int kx = 0; kx < nx; kx++) {
         for (int kf = 0; kf < nf; kf++) {
            to[kf] = from[kf];
         }
         to += nf;
         from += nf;
      }
   }
   return 0;
}

int HyPerLayer::mirrorToNorthEast(PVLayerCube* dest, PVLayerCube* src)
{
   int nx = clayer->loc.nx;
   int nf = clayer->numFeatures;
   int npad = dest->loc.nPad;
   int sy = nf * ( nx + 2*npad );

   pvdata_t * src0 = src-> data + npad * sy + (nx + npad - 1) * nf;
   pvdata_t * dst0 = dest->data + (npad-1) * sy + (nx + npad)  * nf;

   for (int ky = 0; ky < npad; ky++) {
      pvdata_t * to   = dst0 - ky*sy;
      pvdata_t * from = src0 + ky*sy;
      for (int kx = 0; kx < npad; kx++) {
         for (int kf = 0; kf < nf; kf++) {
            to[kf] = from[kf];
         }
         to += nf;
         from -= nf;
      }
   }
   return 0;
}

int HyPerLayer::mirrorToWest(PVLayerCube* dest, PVLayerCube* src)
{
   int nx = clayer->loc.nx;
   int ny = clayer->loc.ny;
   int nf = clayer->numFeatures;
   int npad = dest->loc.nPad;
   int sy = nf * ( nx + 2*npad );

   pvdata_t * src0 = src-> data + npad * sy + npad * nf;
   pvdata_t * dst0 = dest->data + npad * sy + (npad-1) * nf;

   for (int ky = 0; ky < ny; ky++) {
      pvdata_t * to   = dst0 + ky*sy;
      pvdata_t * from = src0 + ky*sy;
      for (int kx = 0; kx < npad; kx++) {
         for (int kf = 0; kf < nf; kf++) {
            to[kf] = from[kf];
         }
         to -= nf;
         from += nf;
      }
   }
   return 0;
}

int HyPerLayer::mirrorToEast(PVLayerCube* dest, PVLayerCube* src)
{
   int nx = clayer->loc.nx;
   int ny = clayer->loc.ny;
   int nf = clayer->numFeatures;
   int npad = dest->loc.nPad;
   int sy = nf * ( nx + 2*npad );

   pvdata_t * src0 = src-> data + npad * sy + (nx + npad - 1) * nf;
   pvdata_t * dst0 = dest->data + npad * sy + (nx + npad) * nf;

   for (int ky = 0; ky < ny; ky++) {
      pvdata_t * to   = dst0 + ky*sy;
      pvdata_t * from = src0 + ky*sy;
      for (int kx = 0; kx < npad; kx++) {
         for (int kf = 0; kf < nf; kf++) {
            to[kf] = from[kf];
         }
         to += nf;
         from -= nf;
      }
   }
   return 0;
}

int HyPerLayer::mirrorToSouthWest(PVLayerCube* dest, PVLayerCube* src)
{
   int nx = clayer->loc.nx;
   int ny = clayer->loc.ny;
   int nf = clayer->numFeatures;
   int npad = dest->loc.nPad;
   int sy = nf * ( nx + 2*npad );

   pvdata_t * src0 = src-> data + (ny + npad - 1) * sy + npad * nf;
   pvdata_t * dst0 = dest->data + (ny + npad) * sy + (npad - 1) * nf;

   for (int ky = 0; ky < npad; ky++) {
      pvdata_t * to   = dst0 + ky*sy;
      pvdata_t * from = src0 - ky*sy;
      for (int kx = 0; kx < npad; kx++) {
         for (int kf = 0; kf < nf; kf++) {
            to[kf] = from[kf];
         }
         to -= nf;
         from += nf;
      }
   }
   return 0;
}

int HyPerLayer::mirrorToSouth(PVLayerCube* dest, PVLayerCube* src)
{
   int nx = clayer->loc.nx;
   int ny = clayer->loc.ny;
   int nf = clayer->numFeatures;
   int npad = dest->loc.nPad;
   int sy = nf * ( nx + 2*npad );

   pvdata_t * src0 = src-> data + (ny + npad -1) * sy + npad * nf;
   pvdata_t * dst0 = dest->data + (ny + npad) * sy + npad * nf;

   for (int ky = 0; ky < npad; ky++) {
      pvdata_t * to   = dst0 + ky*sy;
      pvdata_t * from = src0 - ky*sy;
      for (int kx = 0; kx < nx; kx++) {
         for (int kf = 0; kf < nf; kf++) {
            to[kf] = from[kf];
         }
         to += nf;
         from += nf;
      }
   }
   return 0;
}

int HyPerLayer::mirrorToSouthEast(PVLayerCube* dest, PVLayerCube* src)
{
   int nx = clayer->loc.nx;
   int ny = clayer->loc.ny;
   int nf = clayer->numFeatures;
   int npad = dest->loc.nPad;
   int sy = nf * ( nx + 2*npad );

   pvdata_t * src0 = src-> data + (ny + npad - 1) * sy + (nx + npad - 1) * nf;
   pvdata_t * dst0 = dest->data + (ny + npad) * sy + (nx + npad)  * nf;

   for (int ky = 0; ky < npad; ky++) {
      pvdata_t * to   = dst0 + ky*sy;
      pvdata_t * from = src0 - ky*sy;
      for (int kx = 0; kx < npad; kx++) {
         for (int kf = 0; kf < nf; kf++) {
            to[kf] = from[kf];
         }
         to += nf;
         from -= nf;
      }
   }
   return 0;
 }

} // end of PV namespace

