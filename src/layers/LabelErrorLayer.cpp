/*
 * LabelErrorLayer.cpp
 *
 *  Created on: Nov 30, 2013
 *      Author: garkenyon
 */

#include "LabelErrorLayer.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void LabelErrorLayer_update_state(
    const int numNeurons,
    const int nx,
    const int ny,
    const int nf,
    const int nb,

    float * V,
    const float Vth,
    const float AMax,
    const float AMin,
    const float AShift,
    float * GSynHead,
    float * activity,
    float errScale,
    int isBinary);


#ifdef __cplusplus
}
#endif

namespace PV {

LabelErrorLayer::LabelErrorLayer()
{
   initialize_base();
}

LabelErrorLayer::LabelErrorLayer(const char * name, HyPerCol * hc)
{
   initialize_base();
   initialize(name, hc);
}

LabelErrorLayer::~LabelErrorLayer()
{
}

int LabelErrorLayer::initialize_base()
{
   numChannels = 2;
   errScale = 1;
   isBinary = 1;
   return PV_SUCCESS;
}

int LabelErrorLayer::initialize(const char * name, HyPerCol * hc)
{
   int status = ANNLayer::initialize(name, hc);
   assert(numChannels==2);
   return status;
}

int LabelErrorLayer::ioParamsFillGroup(enum ParamsIOFlag ioFlag) {
   int status = ANNLayer::ioParamsFillGroup(ioFlag);
   ioParam_errScale(ioFlag);
   ioParam_isBinary(ioFlag);
   return status;
}

void LabelErrorLayer::ioParam_errScale(enum ParamsIOFlag ioFlag) {
   parent->ioParamValue(ioFlag, name, "errScale", &errScale, errScale);
}

void LabelErrorLayer::ioParam_isBinary(enum ParamsIOFlag ioFlag) {
   parent->ioParamValue(ioFlag, name, "isBinary", &isBinary, isBinary);
}

int LabelErrorLayer::doUpdateState(double time, double dt, const PVLayerLoc * loc, pvdata_t * A,
      pvdata_t * V, int num_channels, pvdata_t * gSynHead, bool spiking,
      unsigned int * active_indices, unsigned int * num_active)
{
   update_timer->start();
//#ifdef PV_USE_OPENCL
//   if(gpuAccelerateFlag) {
//      updateStateOpenCL(time, dt);
//      //HyPerLayer::updateState(time, dt);
//   }
//   else {
//#endif
      int nx = loc->nx;
      int ny = loc->ny;
      int nf = loc->nf;
      int num_neurons = nx*ny*nf;
    	  LabelErrorLayer_update_state(num_neurons, nx, ny, nf, loc->nb, V, VThresh,
    			  AMax, AMin, AShift, gSynHead, A, errScale, isBinary);
      if (this->writeSparseActivity){
         updateActiveIndices();  // added by GTK to allow for sparse output, can this be made an inline function???
      }
//#ifdef PV_USE_OPENCL
//   }
//#endif

   update_timer->stop();
   return PV_SUCCESS;
}

} /* namespace PV */


#ifdef __cplusplus
extern "C" {
#endif

#ifndef PV_USE_OPENCL
#  include "../kernels/LabelErrorLayer_update_state.cl"
#else
#  undef PV_USE_OPENCL
#  include "../kernels/LabelErrorLayer_update_state.cl"
#  define PV_USE_OPENCL
#endif

#ifdef __cplusplus
}
#endif
