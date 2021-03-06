/*
 * ArborTestForOnesProbe.cpp
 *
 *  Created on: Sep 6, 2011
 *      Author: kpeterson
 */

#include "ArborTestForOnesProbe.hpp"
#include <include/pv_arch.h>
#include <layers/HyPerLayer.hpp>
#include <string.h>
#include <utils/PVLog.hpp>

namespace PV {

ArborTestForOnesProbe::ArborTestForOnesProbe(const char *name, HyPerCol *hc) : StatsProbe() {
   initialize_base();
   initialize(name, hc);
}

ArborTestForOnesProbe::~ArborTestForOnesProbe() {}

int ArborTestForOnesProbe::initialize_base() { return PV_SUCCESS; }

int ArborTestForOnesProbe::initialize(const char *name, HyPerCol *hc) {
   return StatsProbe::initialize(name, hc);
}

int ArborTestForOnesProbe::outputState(double timed) {
   int status           = StatsProbe::outputState(timed);
   Communicator *icComm = parent->getCommunicator();
   const int rcvProc    = 0;
   if (icComm->commRank() != rcvProc) {
      return 0;
   }
   if (timed > 1.0) {
      for (int b = 0; b < parent->getNBatch(); b++) {
         FatalIf(!((fMin[b] > 0.99f) && (fMin[b] < 1.01f)), "Test failed.\n");
         FatalIf(!((fMax[b] > 0.99f) && (fMax[b] < 1.01f)), "Test failed.\n");
         FatalIf(!((avg[b] > 0.99f) && (avg[b] < 1.01f)), "Test failed.\n");
      }
   }

   return status;
}

} /* namespace PV */
