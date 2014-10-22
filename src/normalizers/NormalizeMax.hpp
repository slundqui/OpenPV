/*
 * NormalizeMax.hpp
 *
 *  Created on: Apr 8, 2013
 *      Author: pschultz
 */

#ifndef NORMALIZEMAX_HPP_
#define NORMALIZEMAX_HPP_

#include "NormalizeBase.hpp"

namespace PV {

class NormalizeMax: public PV::NormalizeBase {
// Member functions
public:
   NormalizeMax(const char * name, HyPerCol * hc, HyPerConn ** connectionList, int numConnections);
   virtual ~NormalizeMax();

   virtual int ioParamsFillGroup(enum ParamsIOFlag ioFlag);
   virtual int normalizeWeights();

protected:
   NormalizeMax();
   int initialize(const char * name, HyPerCol * hc, HyPerConn ** connectionList, int numConnections);

   virtual void ioParam_minMaxTolerated(enum ParamsIOFlag ioFlag);

private:
   int initialize_base();

// Member variables
protected:
   float minMaxTolerated; // Error if abs(sum(weights)) in any patch is less than this amount.
};

} /* namespace PV */
#endif /* NORMALIZEMAX_HPP_ */
