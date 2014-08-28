#include "accumulate_functions.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef COMPRESS_PHI
void pvpatch_accumulate(int nk, float* restrict v, float a, pvwdata_t* restrict w,
                        float* restrict m)
{
   const float scale = 33.3;
   const float inv_scale = 1.0/scale;
   const float shift = 2.0;
   int k;

   for (k = 0; k < nk; k++) {
            v[k] = (((shift + scale*v[k]) + a*w[k]*m[k])
                  - shift) * inv_scale;
      // without mask
      //      v[k] = (((shift + scale*v[k]) + a*w[k])
      //                  - shift) * inv_scale;
   }
}
#else
int pvpatch_accumulate(int nk, float* RESTRICT v, float a, pvwdata_t* RESTRICT w, void * auxPtr)
{
   int k;
   int err = 0;
   float accumval = 0;
   for (k = 0; k < nk; k++) {
      accumval = a*w[k];
      v[k] += accumval;
   }
   return err;
}
#endif

int pvpatch_accumulate_from_post(int nk, float * RESTRICT v, float * RESTRICT a, pvwdata_t * RESTRICT w, float dt_factor, void * auxPtr) {
   int status = 0;
   int k;
   float dv = 0.0f;
   for (k = 0; k < nk; k++) {
      *v += a[k]*w[k]*dt_factor;
      //dv = dv + a[k]*w[k];
   }
   //*v = *v + dt_factor*dv;
   return status;
}

int pvpatch_accumulate2(int nk, float* RESTRICT v, float a, pvwdata_t* RESTRICT w, float* RESTRICT m)
{
   int k;
   int err = 0;
   float accumval = 0;
   for (k = 0; k < nk; k++) {
      accumval = a*w[k]*m[k];
//#ifdef PV_USE_OPENMP_THREADS
//#pragma omp atomic
//#endif
      v[k] += accumval;
   }
      //v[k] += accumval;
   return err;
}

#ifdef OBSOLETE // Marked obsolete Aug 21, 2013.  Use cl_random instead of pv_random
int pvpatch_accumulate_stochastic(int nk, float* RESTRICT v, float a, pvwdata_t* RESTRICT w, void * auxPtr)
{
   int k;
   long along = (long) (a*pv_random_max());
   int err = 0;
   float accumval = 0;
   for (k = 0; k < nk; k++) {
      accumval = (pv_random()<along)*w[k];
//#ifdef PV_USE_OPENMP_THREADS
//#pragma omp atomic
//#endif
      v[k] += accumval;
   }
   return err;
}

int pvpatch_accumulate_stochastic_from_post(int nk, float * RESTRICT v, float * RESTRICT a, pvwdata_t * RESTRICT w, float dt_factor, void * auxPtr) {
   int status = 0;
   int k;
   float dv = 0.0f;
   for (k = 0; k < nk; k++) {
      long along = (long) (a[k]*pv_random_max());
      dv = pv_random()<along ? dv + a[k]*w[k] : 0.0f;
   }
   *v = *v + dt_factor*dv;
   return status;
}
#else
int pvpatch_accumulate_stochastic(int nk, float* RESTRICT v, float a, pvwdata_t* RESTRICT w, void * auxPtr)
{
   uint4 * rng = (uint4 *) auxPtr;
   long along = (long) (a*cl_random_max());
   int err = 0;
   int k;
   float accumval = 0;
   for (k = 0; k < nk; k++) {
      *rng = cl_random_get(*rng);
      accumval = (rng->s0 < along)*w[k];
//#ifdef PV_USE_OPENMP_THREADS
//#pragma omp atomic
//#endif
      v[k] += accumval;
   }
   return err;
}

int pvpatch_accumulate_stochastic_from_post(int nk, float * RESTRICT v, float * RESTRICT a, pvwdata_t * RESTRICT w, float dt_factor, void * auxPtr) {
   int status = 0;
   uint4 * rng = (uint4 *) auxPtr;
   int k;
   float dv = 0.0f;
   for (k = 0; k < nk; k++) {
      *rng = cl_random_get(*rng);
      double p = (double) rng->s0/cl_random_max(); // 0.0 < p < 1.0
      dv += (p<a[k])*w[k];
   }
   *v = *v + dt_factor*dv;
   return status;
}
#endif // OBSOLETE

#ifdef OBSOLETE // Marked obsolete Aug 19, 2013.  Nobody calls pvpatch_max and whatever WTACompressedLayer was, it's not in the code now.
// Used by WTACompressedLayer
int pvpatch_max(int nk, float * RESTRICT v, float a, pvwdata_t * RESTRICT w, int feature, int * RESTRICT maxloc) {
   int k;
   int err = 0;
   for (k = 0; k < nk; k++) {
//#ifdef PV_USE_OPENMP_THREADS
//#pragma omp critical
//      {
//#endif

      float prod = a*w[k];
      if (prod!=0 && v[k] == prod) {
         err = 1;
      }
      if (v[k] < prod) {
         v[k] = prod;
         maxloc[k] = feature;
      }
//#ifdef PV_USE_OPENMP_THREADS
//#pragma omp critical
//      }
//#endif

   }
   return err;
}
#endif

int pvpatch_max_pooling(int nk, float* RESTRICT v, float a, pvwdata_t* RESTRICT w, void * auxPtr)
{
  int k;
  int err = 0;
  float compareval;
  for (k = 0; k < nk; k++) {
//#ifdef PV_USE_OPENMP_THREADS
//#pragma omp critical
//#endif
     v[k] = v[k] > a*w[k] ? v[k] : a*w[k];

  }
  return err;
}

int pvpatch_max_pooling_from_post(int nk, float * RESTRICT v, float * RESTRICT a, pvwdata_t * RESTRICT w, float dt_factor, void * auxPtr) {
   int status = 0;
   int k;
   float vmax = *v;
   for (k = 0; k < nk; k++) {
      vmax = vmax > a[k]*w[k] ? vmax : a[k]*w[k];
   }
   *v = vmax;
   return status;
}

#ifdef __cplusplus
}
#endif
