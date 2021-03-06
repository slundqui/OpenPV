#include "BinningTestLayer.hpp"

namespace PV {

BinningTestLayer::BinningTestLayer(const char *name, HyPerCol *hc) {
   BinningLayer::initialize(name, hc);
}

int BinningTestLayer::updateState(double timef, double dt) {
   // Do update state of Binning Layer first
   BinningLayer::updateState(timef, dt);
   // Grab layer size
   const PVLayerLoc *loc = getLayerLoc();
   int nx                = loc->nx;
   int ny                = loc->ny;
   int nf                = loc->nf;
   int kx0               = loc->kx0;
   int ky0               = loc->ky0;
   // Grab the activity layer of current layer
   const float *A = getActivity();
   // We only care about restricted space
   for (int iY = loc->halo.up; iY < ny + loc->halo.up; iY++) {
      for (int iX = loc->halo.up; iX < nx + loc->halo.lt; iX++) {
         for (int iF = 0; iF < nf; iF++) {
            int idx = kIndex(
                  iX,
                  iY,
                  iF,
                  nx + loc->halo.lt + loc->halo.rt,
                  ny + loc->halo.dn + loc->halo.up,
                  nf);
            if (getSigma() == 0) {
               // Based on the input image, X index should be the same as F index
               if (iX + kx0 == iF) {
                  if (A[idx] != 1) {
                     Fatal() << iY << "," << iX << "," << iF << ": " << A[idx] << "\n";
                  }
                  FatalIf(!(A[idx] == 1), "Test failed.\n");
               }
               else {
                  if (A[idx] != 0) {
                     Fatal() << iY << "," << iX << "," << iF << ": " << A[idx] << "\n";
                  }
                  FatalIf(!(A[idx] == 0), "Test failed.\n");
               }
            }
            else if (getSigma() == 2) {
               float temp;
               if (iX + kx0 == iF - 2 || iX + kx0 == iF + 2) {
                  temp = A[idx] - 0.121f;
                  if (temp > .00001f) {
                     Fatal() << iY << "," << iX << "," << iF << ": " << A[idx] << "\n";
                  }
                  FatalIf(!(temp <= 0.00001f), "Test failed.\n");
               }
               if (iX + kx0 == iF - 1 || iX + kx0 == iF + 1) {
                  temp = A[idx] - 0.176f;
                  if (temp > 0.00001f) {
                     Fatal() << iY << "," << iX << "," << iF << ": " << A[idx] << "\n";
                  }
                  FatalIf(!(temp <= 0.00001f), "Test failed.\n");
               }
               if (iX + kx0 == iF) {
                  temp = A[idx] - 0.199f;
                  if (temp > 0.00001f) {
                     Fatal() << iY << "," << iX << "," << iF << ": " << A[idx] << "\n";
                  }
                  FatalIf(!(temp <= 0.00001f), "Test failed.\n");
               }
            }
         }
      }
   }
   return PV_SUCCESS;
}

} /* namespace PV */
