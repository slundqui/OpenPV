/*
 * NormalizeContrastZeroMean.hpp
 *
 *  Created on: Apr 8, 2013
 *      Author: pschultz
 */

#ifndef NORMALIZECONTRASTZEROMEAN_HPP_
#define NORMALIZECONTRASTZEROMEAN_HPP_

#include "NormalizeBase.hpp"

namespace PV {

class NormalizeContrastZeroMean: public PV::NormalizeBase {
   // Member functions
public:
   NormalizeContrastZeroMean(HyPerConn * callingConn);
   virtual ~NormalizeContrastZeroMean();

   virtual int ioParamsFillGroup(enum ParamsIOFlag ioFlag);
   virtual int normalizeWeights(HyPerConn * conn);

protected:
   NormalizeContrastZeroMean();
   int initialize(HyPerConn * callingConn);

   virtual void ioParam_minSumTolerated(enum ParamsIOFlag ioFlag);
   virtual void ioParam_normalizeFromPostPerspective(enum ParamsIOFlag ioFlag);

   static void subtractOffsetAndNormalize(pvwdata_t * dataStartPatch, int weights_per_patch, float offset, float normalizer);
   int accumulateSumAndSumSquared(pvwdata_t * dataPatchStart, int weights_in_patch, double * sum, double * sumsq);

private:
   int initialize_base();

   // Member variables
protected:
   float minSumTolerated; // Error if abs(sum(weights)) in any patch is less than this amount.
};

} /* namespace PV */
#endif /* NORMALIZECONTRASTZEROMEAN_HPP_ */
