/*
 * InitGauss2DWeights.hpp
 *
 *  Created on: Apr 8, 2013
 *      Author: garkenyon
 */

#ifndef INITGAUSS2DWEIGHTS_HPP_
#define INITGAUSS2DWEIGHTS_HPP_

#include "InitWeights.hpp"

namespace PV {

class InitGauss2DWeights : public PV::InitWeights {
  protected:
   /**
    * List of parameters needed from the InitGauss2DWeight class
    * @name InitGauss2DWeight Parameters
    * @{
    */
   virtual void ioParam_aspect(enum ParamsIOFlag ioFlag);
   virtual void ioParam_sigma(enum ParamsIOFlag ioFlag);
   virtual void ioParam_rMax(enum ParamsIOFlag ioFlag);
   virtual void ioParam_rMin(enum ParamsIOFlag ioFlag);
   virtual void ioParam_strength(enum ParamsIOFlag ioFlag);
   virtual void ioParam_numOrientationsPost(enum ParamsIOFlag ioFlag);
   virtual void ioParam_numOrientationsPre(enum ParamsIOFlag ioFlag);
   virtual void ioParam_deltaThetaMax(enum ParamsIOFlag ioFlag);
   virtual void ioParam_thetaMax(enum ParamsIOFlag ioFlag);
   virtual void ioParam_numFlanks(enum ParamsIOFlag ioFlag);
   virtual void ioParam_flankShift(enum ParamsIOFlag ioFlag);
   virtual void ioParam_rotate(enum ParamsIOFlag ioFlag);
   virtual void ioParam_bowtieFlag(enum ParamsIOFlag ioFlag);
   virtual void ioParam_bowtieAngle(enum ParamsIOFlag ioFlag);
   /** @} */

  public:
   InitGauss2DWeights(char const *name, HyPerCol *hc);
   virtual ~InitGauss2DWeights();

   virtual int ioParamsFillGroup(enum ParamsIOFlag ioFlag) override;

  protected:
   InitGauss2DWeights();
   int initialize(char const *name, HyPerCol *hc);

   void ioParam_aspectRelatedParams(enum ParamsIOFlag ioFlag);
   bool needAspectParams();
   void calcOtherParams(int patchIndex);

   virtual int
   communicateInitInfo(std::shared_ptr<CommunicateInitInfoMessage const> message) override;

   virtual void calcWeights(float *dataStart, int dataPatchIndex, int arborId) override;

   void calculateThetas(int kfPre_tmp, int patchIndex);
   float calcThPost(int fPost);
   bool checkThetaDiff(float thPost);
   bool checkColorDiff(int fPost);
   bool isSameLocOrSelf(float xDelta, float yDelta, int fPost);
   bool checkBowtieAngle(float xp, float yp);

  private:
   int initialize_base();
   void gauss2DCalcWeights(float *dataStart);

  protected:
   // params
   float mAspect            = 1.0f;
   float mSigma             = 0.8f;
   float mRMax              = 1.4f;
   float mRMin              = 0.0f;
   float mStrength          = 1.0f;
   int mNumOrientationsPost = 1;
   int mNumOrientationsPre  = 1;
   float mDeltaThetaMax     = 2.0f * PI;
   float mThetaMax          = 1.0f;
   int mNumFlanks           = 1;
   float mFlankShift        = 0.0f;
   float mRotate            = 0.0f;
   bool mBowtieFlag         = false;
   float mBowtieAngle       = 2.0f * PI;

   // calculated values
   float mRMaxSquared;
   float mRMinSquared;
   float mDeltaThetaPost;
   float mTheta0Post;
   float mThetaPre;
   int mFeaturePre;
   float mDeltaTheta;

}; // class InitGauss2DWeights

} /* namespace PV */
#endif /* INITGAUSS2DWEIGHTS_HPP_ */
