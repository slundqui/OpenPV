/*
 * NormalizeGroup.cpp
 *
 *  Created on: Jun 22, 2016
 *      Author: pschultz
 */

#include <normalizers/NormalizeGroup.hpp>

namespace PV {

NormalizeGroup::NormalizeGroup(char const * name, HyPerCol * hc) {
   initialize_base();
   initialize(name, hc);
}

NormalizeGroup::NormalizeGroup() {
}

NormalizeGroup::~NormalizeGroup() {
   free(normalizeGroupName);
}

int NormalizeGroup::initialize_base() {
   return PV_SUCCESS;
}

int NormalizeGroup::initialize(char const * name, HyPerCol * hc) {
   int status = NormalizeBase::initialize(name, hc);
   return status;
}

int NormalizeGroup::ioParamsFillGroup(enum ParamsIOFlag ioFlag) {
   int status = NormalizeBase::ioParamsFillGroup(ioFlag);
   ioParam_normalizeGroupName(ioFlag);
   return status;
}

// The NormalizeBase parameters are overridden to do nothing in NormalizeGroup.
void NormalizeGroup::ioParam_strength(enum ParamsIOFlag ioFlag) {}
void NormalizeGroup::ioParam_normalizeArborsIndividually(enum ParamsIOFlag ioFlag) {}
void NormalizeGroup::ioParam_normalizeOnInitialize(enum ParamsIOFlag ioFlag) {}
void NormalizeGroup::ioParam_normalizeOnWeightUpdate(enum ParamsIOFlag ioFlag) {}

void NormalizeGroup::ioParam_normalizeGroupName(enum ParamsIOFlag ioFlag) {
   parent->ioParamStringRequired(ioFlag, name, "normalizeGroupName", &normalizeGroupName);
}

void NormalizeGroup::communicateInitInfo() {
   groupHead = parent->getNormalizerFromName(normalizeGroupName);
   if (groupHead==nullptr) {
      if (parent->columnId()==0) {
         pvErrorNoExit().printf("%s \"%s\" error: normalizeGroupName \"%s\" is not a recognized normalizer.\n",
               getKeyword(), name, normalizeGroupName);
      }
      MPI_Barrier(parent->icCommunicator()->communicator());
      exit(EXIT_FAILURE);
   }
   HyPerConn * conn = getTargetConn();
   groupHead->addConnToList(conn);
}

int NormalizeGroup::normalizeWeights() {
   return PV_SUCCESS;
}

BaseObject * createNormalizeGroup(char const * name, HyPerCol * hc) {
   return hc==nullptr ? nullptr : new NormalizeGroup(name, hc);
}

} /* namespace PV */