/*
 * KernelConn.cpp
 *
 *  Created on: Aug 6, 2009
 *      Author: gkenyon
 */

#include "KernelConn.hpp"
#include <assert.h>
#include <float.h>
#include "../io/io.h"

namespace PV {

KernelConn::KernelConn()
{
   initialize_base();
}

KernelConn::KernelConn(const char * name, HyPerCol * hc, HyPerLayer * pre,
      HyPerLayer * post, int channel)
{
   initialize_base();
   initialize(name, hc, pre, post, channel);
}

KernelConn::KernelConn(const char * name, HyPerCol * hc, HyPerLayer * pre,
      HyPerLayer * post)
{
   initialize_base();
   initialize(name, hc, pre, post, channel, NULL); // use default channel
}

// provide filename or set to NULL
KernelConn::KernelConn(const char * name, HyPerCol * hc, HyPerLayer * pre,
      HyPerLayer * post, int channel, const char * filename)
{
   initialize_base();
   initialize(name, hc, pre, post, channel, filename);
}

int KernelConn::initialize_base()
{
   kernelPatches = NULL;
   return HyPerConn::initialize_base();
}

PVPatch ** KernelConn::allocWeights(PVPatch ** patches, int nPatches, int nxPatch,
      int nyPatch, int nfPatch)
{
   const int arbor = 0;
   int numKernelPatches = numDataPatches(arbor);

   assert(kernelPatches == NULL);
   kernelPatches = (PVPatch**) calloc(sizeof(PVPatch*), numKernelPatches);
   assert(kernelPatches != NULL);

   for (int kernelIndex = 0; kernelIndex < numKernelPatches; kernelIndex++) {
      kernelPatches[kernelIndex] = pvpatch_inplace_new(nxPatch, nyPatch, nfPatch);
      assert(kernelPatches[kernelIndex] != NULL );
   }
   for (int patchIndex = 0; patchIndex < nPatches; patchIndex++) {
      patches[patchIndex] = pvpatch_new(nxPatch, nyPatch, nfPatch);
   }
   for (int patchIndex = 0; patchIndex < nPatches; patchIndex++) {
      int kernelIndex = patchIndexToKernelIndex(patchIndex);
      patches[patchIndex]->data = kernelPatches[kernelIndex]->data;
   }
   return patches;
}

/*TODO  createWeights currently breaks in this subclass if called more than once,
 * fix interface by adding extra dataPatches argument to overloaded method
 * so asserts are unnecessary
 */
PVPatch ** KernelConn::createWeights(PVPatch ** patches, int nPatches, int nxPatch,
      int nyPatch, int nfPatch)
{
   assert(numAxonalArborLists == 1);

   assert(patches == NULL);

   patches = (PVPatch**) calloc(sizeof(PVPatch*), nPatches);
   assert(patches != NULL);

   assert(kernelPatches == NULL);
   allocWeights(patches, nPatches, nxPatch, nyPatch, nfPatch);

   return patches;
}

int KernelConn::deleteWeights()
{
   const int arbor = 0;

   for (int k = 0; k < numDataPatches(arbor); k++) {
      pvpatch_inplace_delete(kernelPatches[k]);
   }
   free(kernelPatches);

   return HyPerConn::deleteWeights();
}

PVPatch ** KernelConn::initializeWeights(PVPatch ** patches, int numPatches,
      const char * filename)
{
   int arbor = 0;
   int numKernelPatches = numDataPatches(arbor);
   HyPerConn::initializeWeights(kernelPatches, numKernelPatches, filename);
   return patches;
}

PVPatch ** KernelConn::readWeights(PVPatch ** patches, int numPatches,
      const char * filename)
{
   HyPerConn::readWeights(patches, numPatches, filename);
   return HyPerConn::normalizeWeights(patches, numPatches);
}

int KernelConn::numDataPatches(int arbor)
{
   int xScaleFac = (post->clayer->xScale > pre->clayer->xScale) ? pow(2,
         post->clayer->xScale - pre->clayer->xScale) : 1;
   int yScaleFac = (post->clayer->yScale > pre->clayer->yScale) ? pow(2,
         post->clayer->yScale - pre->clayer->yScale) : 1;
   int numKernelPatches = pre->clayer->numFeatures * xScaleFac * yScaleFac;
   return numKernelPatches;
}

float KernelConn::minWeight()
{
   const int axonID = 0;
   const int numKernels = numDataPatches(axonID);
   const int numWeights = nxp * nyp * nfp;
   float min_weight = FLT_MAX;
   for (int iKernel = 0; iKernel < numKernels; iKernel++) {
      pvdata_t * kernelWeights = kernelPatches[iKernel]->data;
      for (int iWeight = 0; iWeight < numWeights; iWeight++) {
         min_weight = (min_weight < kernelWeights[iWeight]) ? min_weight
               : kernelWeights[iWeight];
      }
   }
   return min_weight;
}

float KernelConn::maxWeight()
{
   const int axonID = 0;
   const int numKernels = numDataPatches(axonID);
   const int numWeights = nxp * nyp * nfp;
   float max_weight = -FLT_MAX;
   for (int iKernel = 0; iKernel < numKernels; iKernel++) {
      pvdata_t * kernelWeights = kernelPatches[iKernel]->data;
      for (int iWeight = 0; iWeight < numWeights; iWeight++) {
         max_weight = (max_weight > kernelWeights[iWeight]) ? max_weight
               : kernelWeights[iWeight];
      }
   }
   return max_weight;
}

PVPatch ** KernelConn::normalizeWeights(PVPatch ** patches, int numPatches)
{
   const int arbor = 0;
   const int num_kernels = numDataPatches(arbor);
   HyPerConn::normalizeWeights(kernelPatches, num_kernels);
   if (num_kernels > 1) {
      assert(nfp <= num_kernels);
      symmetrizeWeights(kernelPatches, num_kernels);
   }
   return patches;
}

PVPatch ** KernelConn::symmetrizeWeights(PVPatch ** patches, int numPatches)
{
   PVPatch ** symPatches;
   symPatches = (PVPatch**) calloc(sizeof(PVPatch*), numPatches);
   assert(symPatches != NULL);
   for (int iKernel = 0; iKernel < numPatches; iKernel++) {
      symPatches[iKernel] = pvpatch_inplace_new(nxp, nyp, nfp);
      assert(symPatches[iKernel] != NULL );
   }
   const int sy = nxp * nfp;
   const float deltaTheta = PI / nfp;
   const float offsetTheta = 0.5f * deltaTheta;
   const int kyMid = nyp / 2;
   const int kxMid = nxp / 2;
   for (int iSymKernel = 0; iSymKernel < numPatches; iSymKernel++) {
      PVPatch * symWp = symPatches[iSymKernel];
      pvdata_t * symW = symWp->data;
      float symTheta = offsetTheta + iSymKernel * deltaTheta;
      for (int kySym = 0; kySym < nyp; kySym++) {
         float dySym = kySym - kyMid;
         for (int kxSym = 0; kxSym < nxp; kxSym++) {
            float dxSym = kxSym - kxMid;
            float distSym = sqrt(dxSym * dxSym + dySym * dySym);
            if (distSym > abs(kxMid > kyMid ? kxMid : kyMid)) {
               continue;
            }
            float dyPrime = dySym * cos(symTheta) - dxSym * sin(symTheta);
            float dxPrime = dxSym * cos(symTheta) + dySym * sin(symTheta);
            for (int kfSym = 0; kfSym < nfp; kfSym++) {
               int kDf = kfSym - iSymKernel;
               int iSymW = kfSym + nfp * kxSym + sy * kySym;
               for (int iKernel = 0; iKernel < nfp; iKernel++) {
                  PVPatch * kerWp = kernelPatches[iKernel];
                  pvdata_t * kerW = kerWp->data;
                  int kfRot = iKernel + kDf;
                  if (kfRot < 0) {
                     kfRot = nfp + kfRot;
                  }
                  else {
                     kfRot = kfRot % nfp;
                  }
                  float rotTheta = offsetTheta + iKernel * deltaTheta;
                  float yRot = dyPrime * cos(rotTheta) + dxPrime * sin(rotTheta);
                  float xRot = dxPrime * cos(rotTheta) - dyPrime * sin(rotTheta);
                  yRot += kyMid;
                  xRot += kxMid;
                  // should find nearest neighbors and do weighted average
                  int kyRot = yRot + 0.5f;
                  int kxRot = xRot + 0.5f;
                  int iRotW = kfRot + nfp * kxRot + sy * kyRot;
                  symW[iSymW] += kerW[iRotW] / nfp;
               } // kfRot
            } // kfSymm
         } // kxSym
      } // kySym
   } // iKernel
   const int num_weights = nfp * nxp * nyp;
   for (int iKernel = 0; iKernel < numPatches; iKernel++) {
      PVPatch * kerWp = kernelPatches[iKernel];
      pvdata_t * kerW = kerWp->data;
      PVPatch * symWp = symPatches[iKernel];
      pvdata_t * symW = symWp->data;
      for (int iW = 0; iW < num_weights; iW++) {
         kerW[iW] = symW[iW];
      }
   } // iKernel
   for (int iKernel = 0; iKernel < numPatches; iKernel++) {
      pvpatch_inplace_delete(symPatches[iKernel]);
   } // iKernel
   free(symPatches);
   return patches;
}

int KernelConn::writeWeights(float time, bool last)
{
   const int arbor = 0;
   const int numPatches = numDataPatches(arbor);
   return HyPerConn::writeWeights(kernelPatches, numPatches, NULL, time, last);
}

// one to many mapping, chose first patch index in restricted space
// kernelIndex for unit cell
// patchIndex in extended space
int KernelConn::kernelIndexToPatchIndex(int kernelIndex)
{
   int patchIndex;
   int xScaleFac = (post->clayer->xScale > pre->clayer->xScale) ? pow(2,
         post->clayer->xScale - pre->clayer->xScale) : 1;
   int yScaleFac = (post->clayer->yScale > pre->clayer->yScale) ? pow(2,
         post->clayer->yScale - pre->clayer->yScale) : 1;
   int nfPre = pre->clayer->numFeatures;
   int kxPre = kxPos(kernelIndex, xScaleFac, yScaleFac, nfPre) + pre->clayer->loc.nPad;
   int kyPre = kyPos(kernelIndex, xScaleFac, yScaleFac, nfPre) + pre->clayer->loc.nPad;
   int kfPre = featureIndex(kernelIndex, xScaleFac, yScaleFac, nfPre);
   int nxPre = pre->clayer->loc.nx + 2*pre->clayer->loc.nPad;
   int nyPre = pre->clayer->loc.ny + 2*pre->clayer->loc.nPad;
   patchIndex = kIndex(kxPre, kyPre, kfPre, nxPre, nyPre, nfPre);
   return patchIndex;
}

// many to one mapping from weight patches to kernels
// patchIndex always in extended space
// referenced to unit cell
int KernelConn::patchIndexToKernelIndex(int patchIndex)
{
   int kernelIndex;
   int nxPre = pre->clayer->loc.nx + 2*pre->clayer->loc.nPad;
   int nyPre = pre->clayer->loc.ny + 2*pre->clayer->loc.nPad;
   int nfPre = pre->clayer->numFeatures;
   int kxPre = kxPos(patchIndex, nxPre, nyPre, nfPre);
   int kyPre = kyPos(patchIndex, nxPre, nyPre, nfPre);
   int kfPre = featureIndex(patchIndex, nxPre, nyPre, nfPre);
   int xScaleFac = (post->clayer->xScale > pre->clayer->xScale) ? pow(2,
         post->clayer->xScale - pre->clayer->xScale) : 1;
   int yScaleFac = (post->clayer->yScale > pre->clayer->yScale) ? pow(2,
         post->clayer->yScale - pre->clayer->yScale) : 1;
   kxPre = kxPre % xScaleFac;
   kyPre = kyPre % yScaleFac;
   kernelIndex = kIndex(kxPre, kyPre, kfPre, xScaleFac, yScaleFac, nfPre);
   return kernelIndex;
}

} // namespace PV

