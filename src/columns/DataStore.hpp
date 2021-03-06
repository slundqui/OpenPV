/*
 * DataStore.hpp
 *
 *  Created on: Sep 10, 2008
 *      Author: Craig Rasmussen
 */

#ifndef DATASTORE_HPP_
#define DATASTORE_HPP_

#include "include/pv_arch.h"
#include "include/pv_types.h"
#include "layers/PVLayerCube.hpp"
#include "structures/RingBuffer.hpp"
#include "structures/SparseList.hpp"
#include <cstdlib>
#include <cstring>

namespace PV {

class DataStore {
  public:
   DataStore(int numBuffers, int numItems, int numLevels, bool isSparse);

   virtual ~DataStore() {
      delete mBuffer;
      delete mLastUpdateTimes;
      delete mNumActive;
      delete mActiveIndices;
   }

   int getNumLevels() const { return mNumLevels; }
   int getNumBuffers() const { return mNumBuffers; }
   void newLevelIndex() {
      mBuffer->newLevel();
      mLastUpdateTimes->newLevel();
      if (isSparse()) {
         mNumActive->newLevel();
         mActiveIndices->newLevel();
      }
      mCurrentLevel = (mNumLevels + mCurrentLevel - 1) % mNumLevels;
   }

   // Level (delay) spins slower than bufferId (batch element)

   float *buffer(int bufferId, int level) {
      return mBuffer->getBuffer(level, bufferId * mNumItems);
   }

   float *buffer(int bufferId) { return mBuffer->getBuffer(bufferId * mNumItems); }

   double getLastUpdateTime(int bufferId, int level) const {
      return *mLastUpdateTimes->getBuffer(level, bufferId);
   }

   double getLastUpdateTime(int bufferId) const { return *mLastUpdateTimes->getBuffer(bufferId); }

   void setLastUpdateTime(int bufferId, int level, double t) {
      *mLastUpdateTimes->getBuffer(level, bufferId) = t;
   }

   void setLastUpdateTime(int bufferId, double t) { *mLastUpdateTimes->getBuffer(bufferId) = t; }

   bool isSparse() const { return mSparseFlag; }

   SparseList<float>::Entry *activeIndicesBuffer(int bufferId, int level) {
      return mActiveIndices->getBuffer(level, bufferId * mNumItems);
   }

   SparseList<float>::Entry *activeIndicesBuffer(int bufferId) {
      return mActiveIndices->getBuffer(bufferId * mNumItems);
   }

   void setNumActive(int bufferId, long numActive) { *mNumActive->getBuffer(bufferId) = numActive; }

   long *numActiveBuffer(int bufferId, int level) { return mNumActive->getBuffer(level, bufferId); }

   long *numActiveBuffer(int bufferId) { return mNumActive->getBuffer(bufferId); }

   void markActiveIndicesOutOfSync(int bufferId, int level);

   void updateActiveIndices(int bufferId, int level);

   int getNumItems() const { return mNumItems; }

   /**
    * Returns a PVLayerCube pointing to the data at the given delay.
    * It does not check whether the PVLayerLoc is consistent with the
    * DataStore's numItems or numBuffers.
    */
   PVLayerCube createCube(PVLayerLoc const &loc, int delay);

  private:
   int mNumItems;
   int mCurrentLevel;
   int mNumLevels;
   int mNumBuffers;
   bool mSparseFlag;

   RingBuffer<float> *mBuffer                           = nullptr;
   RingBuffer<double> *mLastUpdateTimes                 = nullptr;
   RingBuffer<long> *mNumActive                         = nullptr;
   RingBuffer<SparseList<float>::Entry> *mActiveIndices = nullptr;
};

} // NAMESPACE

#endif /* DATASTORE_HPP_ */
