/*
 * updateStateFunctions.h
 *
 * Static inline methods to be called by layers' updateState methods
 *
 *  Created on: Mar 7, 2012
 *      Author: pschultz
 */
#ifndef UPDATESTATEFUNCTIONS_H_
#define UPDATESTATEFUNCTIONS_H_

#ifndef PV_USE_CUDA
#include "../include/pv_types.h"
#include "../utils/conversions.h"
#endif // PV_USE_CUDA

#include "../include/pv_common.h"
#include <float.h>

#ifndef PV_USE_CUDA
#define KERNEL inline
#define MEM_GLOBAL
#define MEM_CONST
#define MEM_LOCAL
#else
#define KERNEL __device__
#define MEM_GLOBAL
#define MEM_CONST
#define MEM_LOCAL
#define getIndex() (blockIdx.x * blockDim.x) + threadIdx.x
#include "../cudakernels/conversions.hcu"
#define CHANNEL_EXC 0
#define CHANNEL_INH 1
#define CHANNEL_INHB 2
#define CHANNEL_GAP 3
#endif

// Prototypes
KERNEL
int applyGSyn_HyPerLayer1Channel(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead);
KERNEL
int applyGSyn_HyPerLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead);
KERNEL
int applyGSyn_LabelErrorLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      int isBinary);

KERNEL
int updateV_ANNLayer_vertices(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      int num_channels,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up);

#ifdef OBSOLETE // Marked obsolete Jun 30, 2017. Use updateV_ANNLayer_vertices
// updateV_PtwiseLinearTransferLayer was deprecated Jun 28, 2016, along with the
// PtwiseLinearTransferLayer class.
// Use ANNLayer with verticesA/verticesV/slopeNegInf/slopePosInf, and updateV_ANNLayer_vertices
// instead.
KERNEL
int updateV_PtwiseLinearTransferLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      int num_channels,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up) {
   return updateV_ANNLayer_vertices(
         nbatch,
         numNeurons,
         V,
         num_channels,
         GSynHead,
         activity,
         numVertices,
         verticesV,
         verticesA,
         slopes,
         nx,
         ny,
         nf,
         lt,
         rt,
         dn,
         up);
}
#endif // OBSOLETE // Marked obsolete Jun 30, 2017. Use updateV_ANNLayer_vertices

KERNEL
int updateV_ANNLayer_threshminmax(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      int num_channels,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      float VThresh,
      float AMin,
      float AMax,
      float AShift,
      float VWidth,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up);
KERNEL
int updateV_ANNErrorLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      float errScale);
KERNEL
int updateV_LabelErrorLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      float errScale,
      int isBinary);
KERNEL
int applyGSyn_HyPerLCALayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      float dt_tau,
      float selfInteract,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up);
KERNEL
int applyGSyn_HyPerLCALayer2(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      double *dtAdapt,
      float tau,
      float selfInteract,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up);
KERNEL
int applyGSyn_ISTALayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      double *dtAdapt,
      float tau,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      float VThresh);
KERNEL
int applyGSyn_ISTALayer2(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      double *dtAdapt,
      float tau,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      float VThresh);
KERNEL
int applyGSyn_ANNWhitenedLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead);

KERNEL
int updateV_HyPerLCALayer(
      int nbatch,
      int numNeurons,
      int numChannels,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes,
      double *dtAdapt,
      float tau,
      float selfInteract,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up);

KERNEL
int updateV_MomentumLCALayer(
      int nbatch,
      int numNeurons,
      int numChannels,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      MEM_GLOBAL float *prevDrive,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes,
      double *dtAdapt,
      float tau,
      float LCAMomentumRate,
      float selfInteract,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up);

KERNEL
int updateV_ISTALayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      float VThresh,
      MEM_GLOBAL double *dtAdapt,
      float tau,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      int numChannels);
KERNEL
int updateV_ANNWhitenedLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up);
KERNEL
int updateV_ANNDivInh(int nbatch, int numNeurons, MEM_GLOBAL float *V, MEM_GLOBAL float *GSynHead);
KERNEL
int updateV_ANNSquaredLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead);
KERNEL
int updateV_PoolingANNLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      float biasa,
      float biasb);
KERNEL
int updateV_PtwiseProductLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead);
KERNEL
int updateV_PtwiseQuotientLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead);
KERNEL
int updateV_SigmoidLayer();
KERNEL
int applyVMax_ANNLayer_threshminmax(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      float AMax,
      MEM_GLOBAL float *activity,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up);
KERNEL
int applyVThresh_ANNLayer_threshminmax(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      float AMin,
      float VThresh,
      float AShift,
      float VWidth,
      MEM_GLOBAL float *activity,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up);
KERNEL
int applyVThresh_ANNErrorLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      float AMin,
      float VThresh,
      float AShift,
      MEM_GLOBAL float *activity,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up);
KERNEL
int squareV_ANNSquaredLayer(int nbatch, int numNeurons, MEM_GLOBAL float *V);
KERNEL
int updateSparsityTermDeriv_LogLatWTAGenLayer(
      int nbatch,
      int numNeurons,
      int num_features,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *sparsitytermderivative);
KERNEL
float lateralCompetitionPenalty(MEM_GLOBAL float *V, int num_features);

KERNEL
int setActivity_HyPerLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *A,
      MEM_GLOBAL float *V,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up);

KERNEL
int setActivity_PtwiseLinearTransferLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *A,
      MEM_GLOBAL float *V,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes);

KERNEL
int setActivity_AccumulateLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *A,
      MEM_GLOBAL float *V,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up);
KERNEL
int setActivity_IncrementLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *A,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *Vprev,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up);
KERNEL
int setActivity_GapLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *A,
      MEM_GLOBAL float *V,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      int orig_lt,
      int orig_rt,
      int orig_dn,
      int orig_up,
      MEM_GLOBAL float *active,
      float ampSpiklet);

KERNEL
int resetGSynBuffers_HyPerLayer(
      int nbatch,
      int numNeurons,
      int num_channels,
      MEM_GLOBAL float *GSynHead);
KERNEL
int resetGSynBuffers_PoolingIndexLayer(
      int nbatch,
      int numNeurons,
      int num_channels,
      MEM_GLOBAL float *GSynHead);
KERNEL
int resetGSynBuffers_SigmoidLayer();

KERNEL
int setActivity_KmeansLayer(
      int nbatch,
      int numNeurons,
      int num_channels,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *A,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      bool trainingFlag);

// Definitions
KERNEL
int applyGSyn_HyPerLayer1Channel(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead) {
   int kbatch;
   // gSyn spins from slowest to fastest: [channel, batch, ny, nx, nf]
   MEM_GLOBAL float *GSynExc = &GSynHead[CHANNEL_EXC * nbatch * numNeurons];
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
   kbatch    = getIndex();
#endif // PV_USE_CUDA
   {
      int b                          = kbatch / numNeurons;
      int k                          = kbatch % numNeurons;
      MEM_GLOBAL float *VBatch       = V + b * numNeurons;
      MEM_GLOBAL float *GSynExcBatch = GSynExc + b * numNeurons;
      VBatch[k]                      = GSynExcBatch[k];
   }
   return PV_SUCCESS;
}

KERNEL
int applyGSyn_HyPerLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead) {
   int kbatch;
   MEM_GLOBAL float *GSynExc = &GSynHead[CHANNEL_EXC * nbatch * numNeurons];
   MEM_GLOBAL float *GSynInh = &GSynHead[CHANNEL_INH * nbatch * numNeurons];
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
   kbatch    = getIndex();
#endif // PV_USE_CUDA
   {
      int b                          = kbatch / numNeurons;
      int k                          = kbatch % numNeurons;
      MEM_GLOBAL float *VBatch       = V + b * numNeurons;
      MEM_GLOBAL float *GSynExcBatch = GSynExc + b * numNeurons;
      MEM_GLOBAL float *GSynInhBatch = GSynInh + b * numNeurons;

      VBatch[k] = GSynExcBatch[k] - GSynInhBatch[k];
   }
   return PV_SUCCESS;
}

KERNEL
int applyGSyn_LabelErrorLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      int isBinary) {
   int kbatch;
   MEM_GLOBAL float *GSynExc = &GSynHead[CHANNEL_EXC * nbatch * numNeurons];
   MEM_GLOBAL float *GSynInh = &GSynHead[CHANNEL_INH * nbatch * numNeurons];

   if (isBinary > 0) {
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
      for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
      kbatch = getIndex();
#endif // PV_USE_CUDA
      {
         int b                          = kbatch / numNeurons;
         int k                          = kbatch % numNeurons;
         MEM_GLOBAL float *VBatch       = V + b * numNeurons;
         MEM_GLOBAL float *GSynExcBatch = GSynExc + b * numNeurons;
         MEM_GLOBAL float *GSynInhBatch = GSynInh + b * numNeurons;
         VBatch[k]                      = GSynExcBatch[k] - GSynInhBatch[k];
         if (GSynExcBatch[k] > 0.0f) { // target label is positive
            VBatch[k] = VBatch[k] > 0.0f ? VBatch[k] : 0.0f;
         }
         else { // target label is negative
            VBatch[k] = VBatch[k] < 0.0f ? VBatch[k] : 0.0f;
         }
      }
   }
   else {
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
      for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
      kbatch = getIndex();
#endif // PV_USE_CUDA
      {
         int b               = kbatch / numNeurons;
         int k               = kbatch % numNeurons;
         float *VBatch       = V + b * numNeurons;
         float *GSynExcBatch = GSynExc + b * numNeurons;
         float *GSynInhBatch = GSynInh + b * numNeurons;

         float ratio = 1.0f;
         // Need to find maximum value of target label
         // If first feature, find ratio between target and guess feature val
         int iF = featureIndex(k, nx + lt + rt, ny + dn + up, nf);
         if (iF == 0) {
            float maxTargetVal = GSynExcBatch[k];
            int maxIdx         = k;
            // Find max value in feature space
            for (int iif = 1; iif < nf; iif++) {
               if (GSynExcBatch[k + iif] > maxTargetVal) {
                  maxTargetVal = GSynExcBatch[k + iif];
                  maxIdx       = k + iif;
               }
            }
            // Find ratio
            // if target label is positive and guess is over target
            if (maxTargetVal > 0 && GSynInhBatch[maxIdx] > maxTargetVal) {
               ratio = maxTargetVal / GSynInhBatch[maxIdx];
            }
            else {
               ratio = 1.0f;
            }
         }
         // Calculate V value based on target and rescaled guess
         VBatch[k] = GSynExcBatch[k] - (GSynInhBatch[k] * ratio);
         // If target label is negative, and guess is lower than target label, err = 0
         if (GSynExcBatch[k] < 0.0f) {
            VBatch[k] = VBatch[k] < 0.0f ? VBatch[k] : 0.0f;
         }
      }
   }
   return PV_SUCCESS;
}

KERNEL
int applyGSyn_HyPerLCALayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      double *dtAdapt,
      float tau,
      float selfInteract,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up) {
   int kbatch;
   MEM_GLOBAL float *GSynError = &GSynHead[0 * nbatch * numNeurons]; // weighted input
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
   kbatch    = getIndex();
#endif // PV_USE_CUDA
   {
      int b                            = kbatch / numNeurons;
      int k                            = kbatch % numNeurons;
      float exp_tau                    = (float)exp(-dtAdapt[b] / (double)tau);
      MEM_GLOBAL float *VBatch         = V + b * numNeurons;
      MEM_GLOBAL float *GSynErrorBatch = GSynError + b * numNeurons;
      // Activity extended
      MEM_GLOBAL float *activityBatch = activity + b * (nx + rt + lt) * (ny + up + dn) * nf;
      int kex                         = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
      VBatch[k]                       = exp_tau * VBatch[k]
                  + (1.0f - exp_tau) * (GSynErrorBatch[k] + selfInteract * activityBatch[kex]);
   }
   return PV_SUCCESS;
}

KERNEL
int applyGSyn_HyPerLCALayer2(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      double *dtAdapt,
      float tau,
      float selfInteract,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up) {
   int kbatch;
   MEM_GLOBAL float *GSynError  = &GSynHead[0 * nbatch * numNeurons]; // weighted input
   MEM_GLOBAL float *GSynError2 = &GSynHead[1 * nbatch * numNeurons]; // weighted input

#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
   kbatch    = getIndex();
#endif // PV_USE_CUDA
   {
      int b = kbatch / numNeurons;
      int k = kbatch % numNeurons;

      float exp_tau                     = (float)exp(-dtAdapt[b] / (double)tau);
      MEM_GLOBAL float *VBatch          = V + b * numNeurons;
      MEM_GLOBAL float *GSynErrorBatch  = GSynError + b * numNeurons;
      MEM_GLOBAL float *GSynError2Batch = GSynError2 + b * numNeurons;
      // Activity extended
      MEM_GLOBAL float *activityBatch = activity + b * (nx + rt + lt) * (ny + up + dn) * nf;

      int kex   = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
      VBatch[k] = exp_tau * VBatch[k]
                  + (1.0f - exp_tau) * (GSynErrorBatch[k] - GSynError2Batch[k]
                                        + selfInteract * activityBatch[kex]);
   }
   return PV_SUCCESS;
}

KERNEL
int applyGSyn_MomentumLCALayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      MEM_GLOBAL float *prevDrive,
      double *dtAdapt,
      float tau,
      float LCAMomentumRate,
      float selfInteract,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up) {
   int kbatch;
   MEM_GLOBAL float *GSynError = &GSynHead[0 * nbatch * numNeurons]; // weighted input
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
   kbatch    = getIndex();
#endif // PV_USE_CUDA
   {
      int b                            = kbatch / numNeurons;
      int k                            = kbatch % numNeurons;
      float exp_tau                    = expf((float)-dtAdapt[b] / tau);
      MEM_GLOBAL float *VBatch         = V + b * numNeurons;
      MEM_GLOBAL float *GSynErrorBatch = GSynError + b * numNeurons;
      MEM_GLOBAL float *prevDriveBatch = prevDrive + b * numNeurons;
      // Activity extended
      MEM_GLOBAL float *activityBatch = activity + b * (nx + rt + lt) * (ny + up + dn) * nf;
      int kex                         = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
      // Calculate current drive
      float currentDrive =
            (1.0f - exp_tau) * (GSynErrorBatch[k] + selfInteract * activityBatch[kex]);
      // Accumulate into VBatch with decay and momentum
      VBatch[k] = exp_tau * VBatch[k] + currentDrive + LCAMomentumRate * prevDriveBatch[k];
      // Update momentum buffer
      prevDriveBatch[k] = currentDrive;
   }
   return PV_SUCCESS;
}

KERNEL
int applyGSyn_MomentumLCALayer2(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      MEM_GLOBAL float *prevDrive,
      double *dtAdapt,
      float tau,
      float LCAMomentumRate,
      float selfInteract,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up) {
   int kbatch;
   MEM_GLOBAL float *GSynError  = &GSynHead[0 * nbatch * numNeurons]; // weighted input
   MEM_GLOBAL float *GSynError2 = &GSynHead[1 * nbatch * numNeurons]; // weighted input

#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
   kbatch    = getIndex();
#endif // PV_USE_CUDA
   {
      int b = kbatch / numNeurons;
      int k = kbatch % numNeurons;

      float exp_tau                     = expf((float)-dtAdapt[b] / tau);
      MEM_GLOBAL float *VBatch          = V + b * numNeurons;
      MEM_GLOBAL float *GSynErrorBatch  = GSynError + b * numNeurons;
      MEM_GLOBAL float *GSynError2Batch = GSynError2 + b * numNeurons;
      MEM_GLOBAL float *prevDriveBatch  = prevDrive + b * numNeurons;
      // Activity extended
      MEM_GLOBAL float *activityBatch = activity + b * (nx + rt + lt) * (ny + up + dn) * nf;

      int kex = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);

      float currentDrive = (1.0f - exp_tau) * ((GSynErrorBatch[k] - GSynError2Batch[k])
                                               + selfInteract * activityBatch[kex]);
      VBatch[k]         = exp_tau * VBatch[k] + currentDrive + LCAMomentumRate * prevDriveBatch[k];
      prevDriveBatch[k] = currentDrive;
   }
   return PV_SUCCESS;
}

KERNEL
int applyGSyn_ISTALayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      double *dtAdapt,
      float tau,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      float VThresh) {
   int kbatch;
   MEM_GLOBAL float *GSynError = &GSynHead[CHANNEL_EXC * nbatch * numNeurons]; // weighted input
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
   kbatch    = getIndex();
#endif // PV_USE_CUDA
   {
      int b                            = kbatch / numNeurons;
      int k                            = kbatch % numNeurons;
      MEM_GLOBAL float *VBatch         = V + b * numNeurons;
      MEM_GLOBAL float *GSynErrorBatch = GSynError + b * numNeurons;
      // Activity extended
      MEM_GLOBAL float *activityBatch = activity + b * (nx + rt + lt) * (ny + up + dn) * nf;
      int kex                         = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
      float sign                      = 0.0f;
      if (activityBatch[kex] != 0.0f) {
         sign = activityBatch[kex] / fabsf(activityBatch[kex]);
      }
      VBatch[k] += ((float)dtAdapt[b] / tau) * (GSynErrorBatch[k] - (VThresh * sign));
   }
   return PV_SUCCESS;
}

KERNEL
int applyGSyn_ISTALayer2(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      double *dtAdapt,
      float tau,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      float VThresh) {
   int kbatch;
   MEM_GLOBAL float *GSynError  = &GSynHead[0 * nbatch * numNeurons]; // weighted input
   MEM_GLOBAL float *GSynError2 = &GSynHead[1 * nbatch * numNeurons]; // weighted input

#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
   kbatch    = getIndex();
#endif // PV_USE_CUDA
   {
      int b = kbatch / numNeurons;
      int k = kbatch % numNeurons;

      MEM_GLOBAL float *VBatch          = V + b * numNeurons;
      MEM_GLOBAL float *GSynErrorBatch  = GSynError + b * numNeurons;
      MEM_GLOBAL float *GSynError2Batch = GSynError2 + b * numNeurons;
      // Activity extended
      MEM_GLOBAL float *activityBatch = activity + b * (nx + rt + lt) * (ny + up + dn) * nf;

      int kex    = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
      float sign = 0.0f;
      if (activityBatch[kex] != 0.0f) {
         sign = activityBatch[kex] / fabsf(activityBatch[kex]);
      }
      VBatch[k] += ((float)dtAdapt[b] / tau)
                   * ((GSynErrorBatch[k] - GSynError2Batch[k]) - (VThresh * sign));
   }
   return PV_SUCCESS;
}

KERNEL
int applyGSyn_ANNWhitenedLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead) {
   int k;
   MEM_GLOBAL float *GSynInput           = &GSynHead[0 * nbatch * numNeurons]; // un-whitened input
   MEM_GLOBAL float *GSynAveInput        = &GSynHead[1 * nbatch * numNeurons]; // un-whitened input
   MEM_GLOBAL float *GSynAveSquaredInput = &GSynHead[2 * nbatch * numNeurons]; // un-whitened input
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (k = 0; k < numNeurons * nbatch; k++)
#else
   k         = getIndex();
#endif // PV_USE_CUDA
   {
      // set mean to zero and standard deviation to one over patch window
      V[k] = (GSynInput[k] - GSynAveInput[k])
             / (sqrtf(GSynAveSquaredInput[k] - GSynAveInput[k] * GSynAveInput[k]) + FLT_MIN);
   }
   return PV_SUCCESS;
}

KERNEL
int updateV_ANNLayer_vertices(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      int num_channels,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up) {
   int status = PV_SUCCESS;
   if (num_channels == 1) {
      status = applyGSyn_HyPerLayer1Channel(nbatch, numNeurons, V, GSynHead);
   }
   else {
      status = applyGSyn_HyPerLayer(nbatch, numNeurons, V, GSynHead);
   }
   if (status == PV_SUCCESS) {
      status = setActivity_PtwiseLinearTransferLayer(
            nbatch,
            numNeurons,
            activity,
            V,
            nx,
            ny,
            nf,
            lt,
            rt,
            dn,
            up,
            numVertices,
            verticesV,
            verticesA,
            slopes);
   }
   return status;
}

KERNEL
int updateV_ANNLayer_threshminmax(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      int num_channels,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      float VThresh,
      float AMin,
      float AMax,
      float AShift,
      float VWidth,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up) {
   if (num_channels == 1) {
      applyGSyn_HyPerLayer1Channel(nbatch, numNeurons, V, GSynHead);
   }
   else {
      applyGSyn_HyPerLayer(nbatch, numNeurons, V, GSynHead);
   }
   setActivity_HyPerLayer(nbatch, numNeurons, activity, V, nx, ny, nf, lt, rt, dn, up);
   applyVThresh_ANNLayer_threshminmax(
         nbatch,
         numNeurons,
         V,
         VThresh,
         AMin,
         AShift,
         VWidth,
         activity,
         nx,
         ny,
         nf,
         lt,
         rt,
         dn,
         up);
   applyVMax_ANNLayer_threshminmax(
         nbatch, numNeurons, V, AMax, activity, nx, ny, nf, lt, rt, dn, up);
   return PV_SUCCESS;
}

KERNEL
int updateV_HyPerLCALayer(
      int nbatch,
      int numNeurons,
      int numChannels,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes,
      double *dtAdapt,
      float tau,
      float selfInteract,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up) {
   int status = PV_SUCCESS;
   if (numChannels == 2) {
      if (status == PV_SUCCESS)
         status = applyGSyn_HyPerLCALayer2(
               nbatch,
               numNeurons,
               V,
               GSynHead,
               activity,
               dtAdapt,
               tau,
               selfInteract,
               nx,
               ny,
               nf,
               lt,
               rt,
               dn,
               up);
   }
   else if (numChannels == 1) {
      if (status == PV_SUCCESS)
         status = applyGSyn_HyPerLCALayer(
               nbatch,
               numNeurons,
               V,
               GSynHead,
               activity,
               dtAdapt,
               tau,
               selfInteract,
               nx,
               ny,
               nf,
               lt,
               rt,
               dn,
               up);
   }

   if (status == PV_SUCCESS) {
      status = setActivity_PtwiseLinearTransferLayer(
            nbatch,
            numNeurons,
            activity,
            V,
            nx,
            ny,
            nf,
            lt,
            rt,
            dn,
            up,
            numVertices,
            verticesV,
            verticesA,
            slopes);
   }
   return status;
}

KERNEL
int updateV_MomentumLCALayer(
      int nbatch,
      int numNeurons,
      int numChannels,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      MEM_GLOBAL float *prevDrive,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes,
      double *dtAdapt,
      float tau,
      float LCAMomentumRate,
      float selfInteract,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up) {
   int status = PV_SUCCESS;
   if (numChannels == 2) {
      if (status == PV_SUCCESS)
         status = applyGSyn_MomentumLCALayer2(
               nbatch,
               numNeurons,
               V,
               GSynHead,
               activity,
               prevDrive,
               dtAdapt,
               tau,
               LCAMomentumRate,
               selfInteract,
               nx,
               ny,
               nf,
               lt,
               rt,
               dn,
               up);
   }
   else if (numChannels == 1) {
      if (status == PV_SUCCESS)
         status = applyGSyn_MomentumLCALayer(
               nbatch,
               numNeurons,
               V,
               GSynHead,
               activity,
               prevDrive,
               dtAdapt,
               tau,
               LCAMomentumRate,
               selfInteract,
               nx,
               ny,
               nf,
               lt,
               rt,
               dn,
               up);
   }

   if (status == PV_SUCCESS) {
      status = setActivity_PtwiseLinearTransferLayer(
            nbatch,
            numNeurons,
            activity,
            V,
            nx,
            ny,
            nf,
            lt,
            rt,
            dn,
            up,
            numVertices,
            verticesV,
            verticesA,
            slopes);
   }
   return status;
}

KERNEL
int updateV_ISTALayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      float VThresh,
      MEM_GLOBAL double *dtAdapt,
      float tau,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      int numChannels) {
   int status = PV_SUCCESS;
   if (numChannels == 2) {
      if (status == PV_SUCCESS)
         status = applyGSyn_ISTALayer2(
               nbatch,
               numNeurons,
               V,
               GSynHead,
               activity,
               dtAdapt,
               tau,
               nx,
               ny,
               nf,
               lt,
               rt,
               dn,
               up,
               VThresh);
   }
   else if (numChannels == 1) {
      if (status == PV_SUCCESS)
         status = applyGSyn_ISTALayer(
               nbatch,
               numNeurons,
               V,
               GSynHead,
               activity,
               dtAdapt,
               tau,
               nx,
               ny,
               nf,
               lt,
               rt,
               dn,
               up,
               VThresh);
   }
   if (status == PV_SUCCESS)
      status = setActivity_HyPerLayer(nbatch, numNeurons, activity, V, nx, ny, nf, lt, rt, dn, up);
   return status;
}

KERNEL
int updateV_ANNErrorLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      float errScale) {
   int status;
   status = applyGSyn_HyPerLayer(nbatch, numNeurons, V, GSynHead);
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (int i = 0; i < numNeurons * nbatch; i++) {
      V[i] *= errScale;
   }
   if (status == PV_SUCCESS) {
      status = setActivity_HyPerLayer(nbatch, numNeurons, activity, V, nx, ny, nf, lt, rt, dn, up);
   }
   if (status == PV_SUCCESS) {
      status = setActivity_PtwiseLinearTransferLayer(
            nbatch,
            numNeurons,
            activity,
            V,
            nx,
            ny,
            nf,
            lt,
            rt,
            dn,
            up,
            numVertices,
            verticesV,
            verticesA,
            slopes);
   }
   return status;
}

KERNEL
int updateV_LabelErrorLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      float errScale,
      int isBinary) {
   int status;
   status = applyGSyn_LabelErrorLayer(
         nbatch, numNeurons, V, GSynHead, nx, ny, nf, lt, rt, dn, up, isBinary);
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (int i = 0; i < numNeurons * nbatch; i++) {
      V[i] *= errScale;
   }
   if (status == PV_SUCCESS) {
      status = setActivity_PtwiseLinearTransferLayer(
            nbatch,
            numNeurons,
            activity,
            V,
            nx,
            ny,
            nf,
            lt,
            rt,
            dn,
            up,
            numVertices,
            verticesV,
            verticesA,
            slopes);
   }
   return status;
}

KERNEL
int updateV_ANNWhitenedLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *activity,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up) {
   int status;
   status = applyGSyn_ANNWhitenedLayer(nbatch, numNeurons, V, GSynHead);
   if (status == PV_SUCCESS) {
      status = setActivity_HyPerLayer(nbatch, numNeurons, activity, V, nx, ny, nf, lt, rt, dn, up);
   }
   if (status == PV_SUCCESS) {
      status = setActivity_PtwiseLinearTransferLayer(
            nbatch,
            numNeurons,
            activity,
            V,
            nx,
            ny,
            nf,
            lt,
            rt,
            dn,
            up,
            numVertices,
            verticesV,
            verticesA,
            slopes);
   }
   return status;
}

KERNEL
int updateV_ANNDivInh(int nbatch, int numNeurons, MEM_GLOBAL float *V, MEM_GLOBAL float *GSynHead) {
   int k;
   MEM_GLOBAL float *GSynExc    = &GSynHead[CHANNEL_EXC * nbatch * numNeurons];
   MEM_GLOBAL float *GSynInh    = &GSynHead[CHANNEL_INH * nbatch * numNeurons];
   MEM_GLOBAL float *GSynDivInh = &GSynHead[CHANNEL_INHB * nbatch * numNeurons];

#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (k = 0; k < numNeurons * nbatch; k++)
#else
   k         = getIndex();
#endif // PV_USE_CUDA
   {
      V[k] = (GSynExc[k] - GSynInh[k]) / (GSynDivInh[k] + 0.04f);
   }
   return PV_SUCCESS;
}

KERNEL
int updateV_ANNSquaredLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead) {
   int status;
   status = applyGSyn_HyPerLayer1Channel(nbatch, numNeurons, V, GSynHead);
   if (status == PV_SUCCESS)
      status = squareV_ANNSquaredLayer(nbatch, numNeurons, V);
   return status;
}

KERNEL
int updateV_PoolingANNLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead,
      float biasa,
      float biasb) {
   int k;
   MEM_GLOBAL float *GSynExc = &GSynHead[CHANNEL_EXC * nbatch * numNeurons];
   MEM_GLOBAL float *GSynInh = &GSynHead[CHANNEL_INH * nbatch * numNeurons];
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (k = 0; k < numNeurons * nbatch; k++)
#else
   k         = getIndex();
#endif // PV_USE_CUDA
   {
      V[k] = GSynExc[k] * GSynInh[k] * (biasa * GSynExc[k] + biasb * GSynInh[k]);
   }
   return PV_SUCCESS;
}

KERNEL
int updateV_PtwiseProductLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead) {
   int k;
   MEM_GLOBAL float *GSynExc = &GSynHead[CHANNEL_EXC * nbatch * numNeurons];
   MEM_GLOBAL float *GSynInh = &GSynHead[CHANNEL_INH * nbatch * numNeurons];
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (k = 0; k < numNeurons * nbatch; k++)
#else
   k         = getIndex();
#endif // PV_USE_CUDA
   {
      V[k] = GSynExc[k] * GSynInh[k];
   }
   return PV_SUCCESS;
}

KERNEL
int updateV_PtwiseQuotientLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *GSynHead) {
   int k;
   MEM_GLOBAL float *GSynExc = &GSynHead[CHANNEL_EXC * nbatch * numNeurons];
   MEM_GLOBAL float *GSynInh = &GSynHead[CHANNEL_INH * nbatch * numNeurons];
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (k = 0; k < numNeurons * nbatch; k++)
#else
   k         = getIndex();
#endif // PV_USE_CUDA
   {
      V[k] = GSynExc[k] / GSynInh[k];
   }
   return PV_SUCCESS;
}

KERNEL
int updateV_SigmoidLayer() {
   return PV_SUCCESS; // sourcelayer is responsible for updating V.
}

KERNEL
int applyVMax_ANNLayer_threshminmax(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      float AMax,
      MEM_GLOBAL float *activity,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up) {
   if (AMax < FLT_MAX) {
      int kbatch = 0;
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
      for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
      kbatch = getIndex();
#endif // PV_USE_CUDA
      {
         int b                           = kbatch / numNeurons;
         int k                           = kbatch % numNeurons;
         MEM_GLOBAL float *activityBatch = activity + b * (nx + lt + rt) * (ny + up + dn) * nf;
         int kex                         = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
         if (activityBatch[kex] > AMax) {
            activityBatch[kex] = AMax;
         }
      }
   }
   return PV_SUCCESS;
}

KERNEL
int applyVThresh_ANNLayer_threshminmax(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      float VThresh,
      float AMin,
      float AShift,
      float VWidth,
      MEM_GLOBAL float *activity,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up) {
   if (VThresh > -FLT_MAX) {
      int kbatch = 0;
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
      for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
      kbatch = getIndex();
      ;
#endif // PV_USE_CUDA
      {
         int b                           = kbatch / numNeurons;
         int k                           = kbatch % numNeurons;
         MEM_GLOBAL float *VBatch        = V + b * numNeurons;
         MEM_GLOBAL float *activityBatch = activity + b * (nx + lt + rt) * (ny + up + dn) * nf;
         int kex                         = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
         if (VBatch[k] < VThresh) {
            activityBatch[kex] = AMin;
         }
         else if (VBatch[k] < VThresh + VWidth) {
            activityBatch[kex] =
                  AMin + (VThresh + VWidth - AShift - AMin) * (VBatch[k] - VThresh) / VWidth;
         }
         else {
            activityBatch[kex] -= AShift;
         }
      }
   }
   return PV_SUCCESS;
}

KERNEL
int applyVThresh_ANNErrorLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *V,
      float AMin,
      float VThresh,
      float AShift,
      MEM_GLOBAL float *activity,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up) {
   if (VThresh > -FLT_MAX) {
      int kbatch = 0;
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
      for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
      kbatch = getIndex();
#endif // PV_USE_CUDA
      {
         int b                           = kbatch / numNeurons;
         int k                           = kbatch % numNeurons;
         MEM_GLOBAL float *VBatch        = V + b * numNeurons;
         MEM_GLOBAL float *activityBatch = activity + b * (nx + lt + rt) * (ny + up + dn) * nf;
         int kex                         = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
         if (fabsf(VBatch[k]) < VThresh)
            activityBatch[kex] = AMin;
         else
            activityBatch[kex] -= AShift;
      }
   }
   return PV_SUCCESS;
}

KERNEL
int squareV_ANNSquaredLayer(int nbatch, int numNeurons, MEM_GLOBAL float *V) {
   int k;
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (k = 0; k < numNeurons * nbatch; k++)
#else
   k         = getIndex();
#endif // PV_USE_CUDA
   {
      V[k] *= V[k];
   }
   return PV_SUCCESS;
}

KERNEL
int updateSparsityTermDeriv_LogLatWTAGenLayer(
      int nbatch,
      int numNeurons,
      int num_features,
      MEM_GLOBAL float *V,
      MEM_GLOBAL float *sparsitytermderivative) {
#ifndef PV_USE_CUDA
   int k;
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for
#endif
   for (k = 0; k < numNeurons / num_features; k++) {
      int feature_start         = k * num_features;
      float sum_across_features = 0.0f;
      int f;
      for (f = 0; f < num_features; f++) {
         sum_across_features += V[feature_start + f];
      }
      float lat_wta_expr = lateralCompetitionPenalty(&V[feature_start], num_features);
      for (f = 0; f < num_features; f++) {
         sparsitytermderivative[k * num_features + f] =
               2.0f * (sum_across_features - V[k * num_features + f]) / (1.0f + lat_wta_expr);
      }
   }
#else // PV_USE_CUDA
   int k     = getIndex();
   {
      int feature_start         = k - (k % num_features);
      float sum_across_features = 0.0f;
      for (int f = 0; f < num_features; f++) {
         sum_across_features += V[feature_start + f];
      }
      float lat_wta_expr = lateralCompetitionPenalty(&V[feature_start], num_features);
      // Each block of num_features neurons will have the same sum_across_features and latWTAexpr.
      // Can we eliminate redundant calculations?
      sparsitytermderivative[k] = 2.0f * (sum_across_features - V[k]) / (1.0f + lat_wta_expr);
   }
#endif // PV_USE_CUDA

   return PV_SUCCESS;
}

KERNEL
float lateralCompetitionPenalty(MEM_GLOBAL float *V, int num_features) {
   float z = 0.0f;
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for
#endif
   for (int p = 0; p < num_features; p++) {
      for (int q = 0; q < num_features; q++) {
         if (p != q) {
            z += V[p] * V[q];
         }
      }
   }
   return z;
}

KERNEL
int setActivity_HyPerLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *A,
      MEM_GLOBAL float *V,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up) {
   int kbatch;
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
   kbatch = getIndex();
#endif // PV_USE_CUDA
   {
      int b                    = kbatch / numNeurons;
      int k                    = kbatch % numNeurons;
      MEM_GLOBAL float *ABatch = A + b * ((nx + lt + rt) * (ny + up + dn) * nf);
      MEM_GLOBAL float *VBatch = V + b * numNeurons;
      int kex                  = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
      ABatch[kex]              = VBatch[k];
   }
   return PV_SUCCESS;
}

KERNEL
int setActivity_PtwiseLinearTransferLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *A,
      MEM_GLOBAL float *V,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      int numVertices,
      float *verticesV,
      float *verticesA,
      float *slopes) {
   int kbatch;
   int last = numVertices - 1;
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for
#endif
   for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
   kbatch = getIndex();
#endif // PV_USE_CUDA
   {
      int b         = kbatch / numNeurons;
      int k         = kbatch % numNeurons;
      float *VBatch = V + b * numNeurons;
      float *ABatch = A + b * (nx + lt + rt) * (ny + up + dn) * nf;
      int kex       = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
      int v;
      float potential = VBatch[k];
      float activity  = 0.0f;

      if (potential < verticesV[0]) {
         activity = verticesA[0] + slopes[0] * (potential - verticesV[0]);
      }
      else if (potential >= verticesV[last]) {
         activity = verticesA[last] + slopes[numVertices] * (potential - verticesV[last]);
      }
      else {
         for (v = 0; v < last; v++) {
            if (potential < verticesV[v]) {
               break; // makes the jumps continuous from the right.  TODO: allow user control over
               // value at jump
            }
            if (potential == verticesV[v]) {
               activity = verticesA[v];
            }
            else if (potential > verticesV[v] && potential < verticesV[v + 1]) {
               activity = verticesA[v] + slopes[v + 1] * (potential - verticesV[v]);
            }
         }
      }
      ABatch[kex] = activity;
   }
   return PV_SUCCESS;
}

KERNEL
int setActivity_AccumulateLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *A,
      MEM_GLOBAL float *V,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up) {
   // static inline int setActivity_HyPerLayer(int numNeurons, float * A, float * V, int nx,
   // int ny, int nf, int lt, int rt, int dn, int up) {
   int kbatch;
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
   kbatch = getIndex();
#endif // PV_USE_CUDA
   {
      int b                    = kbatch / numNeurons;
      int k                    = kbatch % numNeurons;
      MEM_GLOBAL float *ABatch = A + b * ((nx + lt + rt) * (ny + up + dn) * nf);
      MEM_GLOBAL float *VBatch = V + b * numNeurons;
      int kex                  = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
      ABatch[kex] += VBatch[k];
   }
   return PV_SUCCESS;
}

KERNEL
int setActivity_GapLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *A,
      MEM_GLOBAL float *V,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      int orig_lt,
      int orig_rt,
      int orig_dn,
      int orig_up,
      MEM_GLOBAL float *checkActive,
      float ampSpikelet) {
   int kbatch;
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
   kbatch = getIndex();
#endif // PV_USE_CUDA
   {
      int b                              = kbatch / numNeurons;
      int k                              = kbatch % numNeurons;
      MEM_GLOBAL float *ABatch           = A + b * ((nx + lt + rt) * (ny + up + dn) * nf);
      MEM_GLOBAL float *VBatch           = V + b * numNeurons;
      MEM_GLOBAL float *checkActiveBatch = checkActive + b * numNeurons;
      int kex                            = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
      int kexorig = kIndexExtended(k, nx, ny, nf, orig_lt, orig_rt, orig_dn, orig_up);
      ABatch[kex] = VBatch[k];
      if (checkActiveBatch[kexorig] > 0.0f) {
         ABatch[kex] += ampSpikelet;
      }
   }
   return PV_SUCCESS;
}

KERNEL
int setActivity_SigmoidLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *A,
      MEM_GLOBAL float *V,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      float VthRest,
      float Vrest,
      float sigmoid_alpha,
      bool sigmoid_flag,
      bool inverse_flag,
      float dt) {
   float Vth       = (VthRest + Vrest) / 2.0f;
   float sig_scale = -logf(1.0f / sigmoid_alpha - 1.0f) / (Vth - Vrest);
   if (!sigmoid_flag) {
      sig_scale = sig_scale / logf(3.0f);
      // If sigmoid_flag is off, A is a piecewise linear function of V, with slope of sig_scale/2 at
      // V=Vth, truncated to have minimum value 0 and maximum value 1.
      // The log(3) term makes alpha=1/4 have the slope such that V reaches 0 at Vrest, and V
      // reaches 1 at VthRest.  Without it, that alpha happens at 0.26894...
   }

   int kbatch;
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
   for (kbatch = 0; kbatch < numNeurons; kbatch++)
#else
   kbatch = getIndex();
#endif // PV_USE_CUDA
   {
      int b                    = kbatch / numNeurons;
      int k                    = kbatch % numNeurons;
      MEM_GLOBAL float *ABatch = A + b * ((nx + lt + rt) * (ny + up + dn) * nf);
      MEM_GLOBAL float *VBatch = V + b * numNeurons;
      int kex                  = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
      float activity           = 0.0f;
      if (!sigmoid_flag) {
         activity = 0.5f - (VBatch[k] - Vth) * sig_scale / 2.0f;
         activity = activity < 0.0f ? 0.0f : activity;
         activity = activity > 1.0f ? 1.0f : activity;
      }
      else {
         activity = 1.0f / (1.0f + expf(2.0f * (VBatch[k] - Vth) * sig_scale));
      }
      ABatch[kex] = activity;
      if (inverse_flag) {
         ABatch[kex] = 1.0f - ABatch[kex];
      }
   }
   return PV_SUCCESS;
}

KERNEL
int resetGSynBuffers_HyPerLayer(
      int nbatch,
      int numNeurons,
      int num_channels,
      MEM_GLOBAL float *GSynHead) {
   for (int ch = 0; ch < num_channels; ch++) {
      MEM_GLOBAL float *channelStart = &GSynHead[ch * nbatch * numNeurons];
      int k;
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
      for (k = 0; k < numNeurons * nbatch; k++)
#else
      k   = getIndex();
#endif // PV_USE_CUDA
      {
         channelStart[k] = 0.0f;
      }
   }
   return PV_SUCCESS;
}

// TODO merge this with resetGSynBuffers_HyPerLayer with a template
KERNEL
int resetGSynBuffers_PoolingIndexLayer(
      int nbatch,
      int numNeurons,
      int num_channels,
      MEM_GLOBAL float *GSynHead) {
   for (int ch = 0; ch < num_channels; ch++) {
      MEM_GLOBAL float *channelStart = &GSynHead[ch * nbatch * numNeurons];
      int k;
#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
      for (k = 0; k < numNeurons * nbatch; k++)
#else
      k   = getIndex();
#endif // PV_USE_CUDA
      {
         channelStart[k] = -1.0f;
      }
   }
   return PV_SUCCESS;
}

KERNEL
int resetGSynBuffers_SigmoidLayer() {
   return PV_SUCCESS; // V is cloned from sourcelayer, so Sigmoid Layer doesn't use the GSynBuffers
}

/* KmeansLayer does not use V */
KERNEL
int setActivity_KmeansLayer(
      int nbatch,
      int numNeurons,
      int num_channels,
      MEM_GLOBAL float *GSynHead,
      MEM_GLOBAL float *A,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      bool trainingFlag) {
   MEM_GLOBAL float *GSynExc = &GSynHead[CHANNEL_EXC * nbatch * numNeurons];
   for (int b = 0; b < nbatch; b++) {
      MEM_GLOBAL float *ABatch       = A + b * ((nx + lt + rt) * (ny + up + dn) * nf);
      MEM_GLOBAL float *GSynExcBatch = GSynExc + b * numNeurons;

      for (int i = 0; i < ny; i++) {
         for (int j = 0; j < nx; j++) {
            if (trainingFlag) {
               int max = 0, maxIndex = 0;

               // look for the maximum value
               for (int k = 0; k < nf; k++) {
                  int kk = (i * nx + j) * nf + k;

                  if (GSynExcBatch[kk] > max) {
                     max      = GSynExcBatch[kk];
                     maxIndex = kk;
                  }
               }

#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
               for (int k = 0; k < nf; k++) {
                  int kk  = (i * nx + j) * nf + k;
                  int kex = kIndexExtended(kk, nx, ny, nf, lt, rt, dn, up);
                  if (kk == maxIndex)
                     ABatch[kex] = 1.0f;
                  else
                     ABatch[kex] = 0.0f;
               }
            }
            else {
               // compute mean
               float mean = 0.0f;

               for (int k = 0; k < nf; k++) {
                  int kk = (i * nx + j) * nf + k;
                  mean += GSynExcBatch[kk];
               }

               mean /= nf;

#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for schedule(static)
#endif
               for (int k = 0; k < nf; k++) {
                  int kk  = (i * nx + j) * nf + k;
                  int kex = kIndexExtended(kk, nx, ny, nf, lt, rt, dn, up);
                  if (GSynExcBatch[kk] >= mean) {
                     ABatch[kex] = GSynExcBatch[kk];
                  }
                  else {
                     ABatch[kex] = 0.0f;
                  }
               }
            }
         }
      }
   }

   return PV_SUCCESS;
}

KERNEL
int setActivity_MLPSigmoidLayer(
      int nbatch,
      int numNeurons,
      MEM_GLOBAL float *A,
      MEM_GLOBAL float *V,
      float linear_alpha,
      bool *dropout_buf,
      int nx,
      int ny,
      int nf,
      int lt,
      int rt,
      int dn,
      int up,
      float dt) {
   int kbatch;

#ifndef PV_USE_CUDA
#ifdef PV_USE_OPENMP_THREADS
#pragma omp parallel for
#endif
   for (kbatch = 0; kbatch < numNeurons * nbatch; kbatch++)
#else
   kbatch = getIndex();
#endif // !PV_USE_CUDA
   {
      int b              = kbatch / numNeurons;
      int k              = kbatch % numNeurons;
      float *VBatch      = V + b * nx * ny * nf;
      float *ABatch      = A + b * (nx + lt + rt) * (ny + up + dn) * nf;
      bool *dropoutBatch = dropout_buf + b * nx * ny * nf;

      int kex        = kIndexExtended(k, nx, ny, nf, lt, rt, dn, up);
      float activity = 1.7159f * tanhf(2.0f / 3.0f * VBatch[k]) + linear_alpha * VBatch[k];
      // Set explicitly to 0 if that neuron was dropped out
      ABatch[kex] = dropoutBatch[k] ? 0.0f : activity;
   }
   return PV_SUCCESS;
}
#endif /* UPDATESTATEFUNCTIONS_H_ */
