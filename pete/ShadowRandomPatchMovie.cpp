/*
 * ShadowRandomPatchMovie.cpp
 *
 *      Author: Pete Schultz
 * Like RandomPatchMovie, but whenever instead of generating its own random numbers,
 * it copies off the worksheet of another RandomPatchMovie
 *
 * This way, you can be sure to take the same patch from the same image if
 * retina-on and retina-off images are loaded using two different catalogs of images.
 */

#include "ShadowRandomPatchMovie.hpp"

namespace PV {

ShadowRandomPatchMovie::ShadowRandomPatchMovie() {
   initialize_base();
}

ShadowRandomPatchMovie::ShadowRandomPatchMovie(const char * name, HyPerCol * hc, const char * fileOfFileNames, float defaultDisplayPeriod) {
   initialize_base();
   initialize(name, hc, fileOfFileNames, defaultDisplayPeriod);
}

ShadowRandomPatchMovie::~ShadowRandomPatchMovie() {
}

int ShadowRandomPatchMovie::initialize_base() {
   shadowedRandomPatchMovie = NULL;
   return PV_SUCCESS;
}

int ShadowRandomPatchMovie::initialize(const char * name, HyPerCol * hc, const char * fileOfFileNames, float defaultDisplayPeriod) {
   assert( this->shadowedRandomPatchMovie == NULL );
   int status = RandomPatchMovie::initialize(name, hc, fileOfFileNames, defaultDisplayPeriod);
   const char * shadowedName = hc->parameters()->stringValue(name, "shadowedRandomPatchMovie", true);
   if( shadowedName == NULL ) {
      fprintf(stderr,"ShadowRandomPatchMovie \"%s\": the string parameter shadowedRandomPatchMovie must be set.  Exiting\n", name);
      exit(EXIT_FAILURE);
   }
   HyPerLayer * shadowedLayer = hc->getLayerFromName(shadowedName);
   shadowedRandomPatchMovie = dynamic_cast<RandomPatchMovie *>(shadowedLayer);
   if( this->shadowedRandomPatchMovie == NULL ) {
      fprintf(stderr, "ShadowRandomPatchMovie \"%s\": shadowed layer \"%s\" must be a RandomPatchMovie\n", name, shadowedName);
      exit(EXIT_FAILURE);
   }
   return status;
}

int ShadowRandomPatchMovie::getRandomOffsets(const PVLayerLoc * imgloc, int * offsetXptr, int * offsetYptr) {
   *offsetXptr = shadowedRandomPatchMovie->getOffsetX();
   *offsetYptr = shadowedRandomPatchMovie->getOffsetY();
   return PV_SUCCESS;
}

int ShadowRandomPatchMovie::getRandomFileIndex() {
   return shadowedRandomPatchMovie->getFileIndex();
}

}  // end namespace PV
