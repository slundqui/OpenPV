/*
 * Retina.cpp
 *
 *  Created on: Jul 29, 2008
 *
 */

#include "HyPerLayer.hpp"
#include "Retina.hpp"
#include "../io/io.h"
#include "../include/default_params.h"
#include "../utils/cl_random.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void Retina_update_state (
    const float time,
    const float dt,
    const int nx,
    const int ny,
    const int nf,
    const int nb,
    const Retina_params * params,
    uint4 * rnd,
    float * phiExc,
    float * phiInh,
    float * activity,
    float * prevTime);

#ifdef __cplusplus
}
#endif


namespace PV {

Retina::Retina(const char * name, HyPerCol * hc)
  : HyPerLayer(name, hc, 2)
{
#ifdef OBSOLETE
   this->img = new Image("Image", hc, hc->inputFile());
#endif
   initialize(TypeRetina);
}

Retina::~Retina()
{
   free(evList);
   free(rand_state);
}

int Retina::initialize(PVLayerType type)
{
   int status = 0;
   PVLayer * l = clayer;

   clayer->layerType = type;
   setParams(parent->parameters());

   // the size of the Retina may have changed due to size of image
   //
   const int nx = l->loc.nx;
   const int ny = l->loc.ny;
   const int nf = l->loc.nf;
   const int nb = l->loc.nb;
   l->numNeurons  = nx * ny * nf;
   l->numExtended = (nx + 2*nb) * (ny + 2*nb) * nf;

   // a random state variable is needed for every neuron/clthread
   rand_state = cl_random_init(l->numNeurons);

   status = parent->addLayer(this);

   // TODO - could free other layer parameters as they are not used

#ifdef PV_USE_OPENCL
   numEvents = NUM_RETINA_EVENTS;
   evList = (cl_event *) malloc(numEvents*sizeof(cl_event));

   initializeThreadBuffers();
   initializeThreadKernels();
#endif

   return status;
}

#ifdef PV_USE_OPENCL
/**
 * Initialize OpenCL buffers.  This must be called after PVLayer data have
 * been allocated.
 */
int Retina::initializeThreadBuffers()
{
   int status = CL_SUCCESS;

   const size_t size    = clayer->numNeurons  * sizeof(pvdata_t);
   const size_t size_ex = clayer->numExtended * sizeof(pvdata_t);

   CLDevice * device = parent->getCLDevice();

   // these buffers are shared between host and device
   //
   clV     = device->createBuffer(CL_MEM_COPY_HOST_PTR, size, clayer->V);
   clPhiE  = device->createBuffer(CL_MEM_COPY_HOST_PTR, size, getChannel(CHANNEL_EXC));
   clPhiI  = device->createBuffer(CL_MEM_COPY_HOST_PTR, size, getChannel(CHANNEL_INH));
   clPhiIB = device->createBuffer(CL_MEM_COPY_HOST_PTR, size, getChannel(CHANNEL_INHB));

   clActivity = device->createBuffer(CL_MEM_COPY_HOST_PTR, size_ex, clayer->activity->data);
   clPrevTime = device->createBuffer(CL_MEM_COPY_HOST_PTR, size_ex, clayer->prevActivity);

   return status;
}

int Retina::initializeThreadKernels()
{
   int status = CL_SUCCESS;

   // create kernels
   //
   krUpdate = parent->getCLDevice()->createKernel("Retina_update_state.cl", "update_state");

   int argid = 0;

   status |= krUpdate->setKernelArg(argid++, 0.0f); // time (changed by updateState)
   status |= krUpdate->setKernelArg(argid++, 1.0f); // dt (changed by updateState)

   // TODO - add Retina_params

   status |= krUpdate->setKernelArg(argid++, clayer->loc.nx);
   status |= krUpdate->setKernelArg(argid++, clayer->loc.ny);
   status |= krUpdate->setKernelArg(argid++, clayer->loc.nf);
   status |= krUpdate->setKernelArg(argid++, clayer->loc.nb);

   // TODO - add rnd_state

   status |= krUpdate->setKernelArg(argid++, clPhiE);
   status |= krUpdate->setKernelArg(argid++, clPhiI);
   status |= krUpdate->setKernelArg(argid++, clPhiIB);
   status |= krUpdate->setKernelArg(argid++, clActivity);
   status |= krUpdate->setKernelArg(argid++, clPrevTime);

   return status;
}
#endif

int Retina::setParams(PVParams * p)
{
   float dt_sec = parent->getDeltaTime() * .001;  // seconds

   clayer->loc.nf = 1;
   clayer->loc.nb = (int) p->value(name, "marginWidth", 0.0);

   clayer->params = &rParams;

   spikingFlag = (int) p->value(name, "spikingFlag", 1);

#ifdef OBSOLETE
   fileread_params * cp = (fileread_params *) clayer->params;
   if (fparams->present(name, "invert"))  cp->invert  = fparams->value(name, "invert");
   if (fparams->present(name, "uncolor")) cp->uncolor = fparams->value(name, "uncolor");
#endif

   float probStim = p->value(name, "poissonEdgeProb" , 1.0f);
   float probBase = p->value(name, "poissonBlankProb", 0.0f);

   if (p->present(name, "noiseOnFreq")) {
      probStim = p->value(name, "noiseOnFreq") * dt_sec;
      if (probStim > 1.0) probStim = 1.0;
   }
   if (p->present(name, "noiseOffFreq")) {
      probBase = p->value(name, "noiseOffFreq") * dt_sec;
      if (probBase > 1.0) probBase = 1.0;
   }

   // default parameters
   //
   rParams.probStim  = probStim;
   rParams.probBase  = probBase;
   rParams.beginStim = p->value(name, "beginStim", 0.0f);
   rParams.endStim   = p->value(name, "endStim"  , 99999999.9f);
   rParams.burstFreq = p->value(name, "burstFreq", 40);         // frequency of bursts
   rParams.burstDuration = p->value(name, "burstDuration", 20); // duration of each burst, <=0 -> sinusoidal
   rParams.refactory_period = REFACTORY_PERIOD;
   rParams.abs_refactory_period = REFACTORY_PERIOD;

   return 0;
}

#ifdef OBSOLETE
//! Sets the V data buffer
/*!
 *
 * REMARKS:
 *      - this method is called from  updateImage()
 *      - copies from the Image data buffer into the V buffer
 *      - it normalizes the V buffer so that V <= 1.
 *      .
 *
 *
 */
int Retina::copyFromImageBuffer()
{
   const int nf = clayer->numFeatures;
   pvdata_t * V = clayer->V;

   PVLayerLoc imageLoc = img->getImageLoc();

   assert(clayer->loc.nx == imageLoc.nx && clayer->loc.ny == imageLoc.ny);

   pvdata_t * ibuf = img->getImageBuffer();

   // for now
   assert(nf == 1);

   // This is incorrect because V is extended
   // might be able to use image buffer directly
   //
   //HyPerLayer::copyToBuffer(V, ibuf, &imageLoc, true, 1.0f);

   // normalize so that V <= 1.0 (V in Retina is extended)
   pvdata_t vmax = 0;
   for (int k = 0; k < clayer->numExtended; k++) {
      V[k] = ibuf[k];
      vmax = ( V[k] > vmax ) ? V[k] : vmax;
   }
   if (vmax != 0){
      for (int k = 0; k < clayer->numExtended; k++) {
         V[k] = V[k] / vmax;
      }
   }
/*
   pvdata_t vmin = 0;
   for (int k = 0; k < clayer->numExtended; k++) {
      V[k] = ibuf[k];
      vmin = V[k] < vmin ? V[k] : vmin;
   }
   if (vmin < -1){
      for (int k = 0; k < clayer->numExtended; k++) {
         V[k] = V[k] / fabs(vmin);
      }
   }
*/

   //
   // otherwise handle OFF/ON cells

   // f[0] are OFF, f[1] are ON cells
//   const int count = imageLoc.nx * imageLoc.ny;
//   if (fireOffPixels) {
//      for (int k = 0; k < count; k++) {
//         V[2*k]   = 1 - ibuf[k] / Imax;
//         V[2*k+1] = ibuf[k] / Imax;
//      }
//   }
//   else {
//      for (int k = 0; k < count; k++) {
//         V[2*k]   = ibuf[k] / Imax;
//         V[2*k+1] = ibuf[k] / Imax;
//      }

   return 0;
}

//! updates the Image that Retina is exposed to
/*!
 *
 * REMARKS:
 *      - This depends on the Image class. The data buffer is generally modulated
 *      by the intensity at the image location.
 *
 *
 */
int Retina::updateImage(float time, float dt)
{
   bool changed = img->updateImage(time, dt);
   if (not changed) return 0;

   int status = copyFromImageBuffer();

   PVLayer  * l   = clayer;
   fileread_params * params = (fileread_params *) l->params;
   pvdata_t * V = clayer->V;

   if (params->invert) {
      for (int k = 0; k < l->numExtended; k++) {
         V[k] = 1 - V[k];
      }
   }

   if (params->uncolor) {
      for (int k = 0; k < l->numExtended; k++) {
         V[k] = (V[k] == 0.0) ? 0.0 : 1.0;
      }
   }

   return status;
}
#endif

#ifdef PV_USE_OPENCL
int Retina::updateStateOpenCL(float time, float dt)
{
   int status = CL_SUCCESS;

   // wait for memory to be copied to device
   status |= clWaitForEvents(numEvents, evList);
   for (int i = 1; i < numEvents; i++) {
      clReleaseEvent(evList[i]);
   }

   status |= krUpdate->setKernelArg(0, time);
   status |= krUpdate->setKernelArg(1, dt);
   status |= krUpdate->run(clayer->numNeurons, nxl*nyl, 0, NULL, &evUpdate);

   status |= clPhiE    ->copyFromDevice(1, &evUpdate, &evList[EV_PHI_E]);
   status |= clPhiI    ->copyFromDevice(1, &evUpdate, &evList[EV_PHI_I]);
   status |= clActivity->copyFromDevice(1, &evUpdate, &evList[EV_ACTIVITY]);

   return status;
}
#endif

int Retina::updateBorder(float time, float dt)
{
   int status = CL_SUCCESS;
#ifdef PV_USE_OPENCL
   // wait for memory to be copied from device
   status = clWaitForEvents(numEvents, evList);

   clReleaseEvent(evUpdate);               // update event will have also finished
   for (int i = 0; i < numEvents; i++) {
      clReleaseEvent(evList[i]);
   }
#endif
   // calculate active indices
   //
   int numActive = 0;
   PVLayerLoc & loc = clayer->loc;
   pvdata_t * activity = clayer->activity->data;

   for (int k = 0; k < clayer->numNeurons; k++) {
      const int kex = kIndexExtended(k, loc.nx, loc.ny, loc.nf, loc.nb);
      if (activity[kex] > 0.0) {
         clayer->activeIndices[numActive++] = globalIndexFromLocal(k, loc);
      }
      clayer->numActive = numActive;
   }

   return status;
}

int Retina::triggerReceive(InterColComm* comm)
{
   int status = CL_SUCCESS;

   // deliver calls recvSynapticInput for all presynaptic connections
   //
   comm->deliver(parent, getLayerId());

   // copy data to device
   //
   status |= clPhiE->copyToDevice(&evList[EV_PHI_E]);
   status |= clPhiI->copyToDevice(&evList[EV_PHI_I]);

   return status;
}

int Retina::waitOnPublish(InterColComm* comm)
{
   int status = CL_SUCCESS;

   // wait for MPI border transfers to complete
   status |= comm->wait(getLayerId());

   // copy activity to device
   //
   status |= clActivity->copyToDevice(&evList[EV_ACTIVITY]);

   return status;
}

//! Updates the state of the Retina
/*!
 * REMARKS:
 *      - prevActivity[] buffer holds the time when a neuron last spiked.
 *      - not used if nonspiking
 *      - it sets the probStim and probBase.
 *              - probStim = poissonEdgeProb * V[k];
 *              - probBase = poissonBlankProb
 *              .
 *      - activity[] is set to 0 or 1 depending on the return of spike()
 *      - this depends on the last time a neuron spiked as well as on V[]
 *      at the location of the neuron. This V[] is set by calling updateImage().
 *      - V points to the same memory space as data in the Image so that when Image
 *      is updated, V gets updated too.
 *      .
 * NOTES:
 *      - poissonEdgeProb = noiseOnFreq * dT
 *      - poissonBlankProb = noiseOffFreq * dT
 *      .
 *
 *
 */
int Retina::updateState(float time, float dt)
{
   update_timer->start();

#ifndef PV_USE_OPENCL
   const int nx = clayer->loc.nx;
   const int ny = clayer->loc.ny;
   const int nf = clayer->loc.nf;
   const int nb = clayer->loc.nb;

   pvdata_t * phiExc   = getChannel(CHANNEL_EXC);
   pvdata_t * phiInh   = getChannel(CHANNEL_INH);
   pvdata_t * activity = clayer->activity->data;

   if (spikingFlag == 1) {
      update_state(time, dt, nx, ny, nf, nb,
                   &rParams, rand_state,
                   phiExc, phiInh, activity, clayer->prevActivity);

   }
   else {
      // retina is non spiking, pass scaled image through to activity
      // scale by poissonEdgeProb (maxRetinalActivity)
      //
      for (int k = 0; k < clayer->numNeurons; k++) {
         const int kex = kIndexExtended(k, nx, ny, nf, nb);
         activity[kex] = rParams.probStim * (phiExc[k] - phiInh[k]);
         // TODO - get rid of this for performance
         clayer->activeIndices[k] = globalIndexFromLocal(k, clayer->loc);

         // reset accumulation buffers
         phiExc[k] = 0.0;
         phiInh[k] = 0.0;
      }
      clayer->numActive = clayer->numNeurons;
   }
#else

   updateStateOpenCL(time, dt);

#endif

   update_timer->stop();

#ifdef DEBUG_PRINT
   char filename[132];
   sprintf(filename, "r_%d.tiff", (int)(2*time));
   this->writeActivity(filename, time);

   printf("----------------\n");
   for (int k = 0; k < 6; k++) {
      printf("host:: k==%d h_exc==%f h_inh==%f\n", k, phiExc[k], phiInh[k]);
   }
   printf("----------------\n");

#endif

   return 0;
}

int Retina::writeState(const char * path, float time)
{
   HyPerLayer::writeState(path, time);

   // print activity at center of image

#ifdef DEBUG_OUTPUT
   int sx = clayer->numFeatures;
   int sy = sx*clayer->loc.nx;
   pvdata_t * a = clayer->activity->data;

   for (int k = 0; k < clayer->numExtended; k++) {
      if (a[k] == 1.0) printf("a[%d] == 1\n", k);
   }

  int n = (int) (sy*(clayer->loc.ny/2 - 1) + sx*(clayer->loc.nx/2));
  for (int f = 0; f < clayer->numFeatures; f++) {
     printf("a[%d] = %f\n", n, a[n]);
     n += 1;
  }
#endif

   return 0;
}

//! Spiking method for Retina
/*!
 * Returns 1 if an event should occur, 0 otherwise. This is a stochastic model.
 * REMARKS:
 *      - During ABS_REFACTORY_PERIOD a neuron does not spike
 *      - The neurons that correspond to stimuli (on Image pixels)
 *       spike with probability probStim.
 *      - The neurons that correspond to background image pixels
 *      spike with probability probBase.
 *      - After ABS_REFACTORY_PERIOD the spiking probability
 *        grows exponentially to probBase and probStim respectively.
 *      - The burst of the retina is periodic with period T set by
 *        T = 1000/burstFreq in miliseconds
 *      - When the time t is such that mT < t < mT + burstDuration, where m is
 *      an integer, the burstStatus is set to 1.
 *      - The burstStatus is also determined by the condition that
 *      beginStim < t < endStim. These parameters are set in the input
 *      params file params.stdp
 *      - sinAmp modulates the spiking probability only when burstDuration <= 0
 *      or burstFreq = 0
 *      - probSpike is set to probBase for all neurons.
 *      - for neurons exposed to Image on pixels, probSpike increases
 *       with probStim.
 *      - When the probability is negative, the neuron does not spike.
 *      .
 * NOTES:
 *      - time is measured in milliseconds.
 *      .
 */
#ifdef OBSOLETE
int Retina::spike(float time, float dt, float prev, float probBase, float probStim, float * probSpike)
{
   fileread_params * params = (fileread_params *) clayer->params;
   float burstStatus = 1;
   float sinAmp = 1.0;

   // see if neuron is in a refactory period
   //
   if ((time - prev) < ABS_REFACTORY_PERIOD) {
      return 0;
   }
   else {
      float delta = time - prev - ABS_REFACTORY_PERIOD;
      float refact = 1.0f - expf(-delta/REFACTORY_PERIOD);
      refact = (refact < 0) ? 0 : refact;
      probBase *= refact;
      probStim *= refact;
   }

   if (params->burstDuration < 0 || params->burstFreq == 0) {
      sinAmp = cos( 2 * PI * time * params->burstFreq / 1000. );
   }
   else {
      burstStatus = fmodf(time, 1000. / params->burstFreq);
      burstStatus = burstStatus <= params->burstDuration;
   }

   burstStatus *= (int) ( (time >= params->beginStim) && (time < params->endStim) );
   *probSpike = probBase;

   if ((int)burstStatus) {
      *probSpike += probStim * sinAmp;  // negative prob is OK
    }
   return ( pv_random_prob() < *probSpike );
}
#endif

} // namespace PV

///////////////////////////////////////////////////////
//
// implementation of Retina kernels
//

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PV_USE_OPENCL
#  include "../kernels/Retina_update_state.cl"
#endif

#ifdef __cplusplus
}
#endif
