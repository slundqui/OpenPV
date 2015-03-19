/*
 * main.cpp
 *
 */


#include <columns/buildandrun.hpp>
#include "ComparisonLayer.hpp"
#include "InputLayer.hpp"
#include "GTLayer.hpp"

void * addcustomgroup(const char * keyword, const char * groupname, HyPerCol * hc);

int main(int argc, char * argv[]) {
   int rank;
   bool argerr = false;
   int reqrtn = 0;
   int threading = 0;

   if (argc > 1){
      for(int i = 1; i < argc; i++){
         //Allowed arguments are -t and --require-return
         if(strcmp(argv[i], "-t") == 0){
            if(i+1 >= argc || argv[i+1][0] == '-'){
               //Do nothing
            }
            else{
               int numthreads = atoi(argv[i+1]);
               if(numthreads != 4){
                  std::cout << "Hardcoding to 4 threads\n";
               }
               i++;
            }
            threading = 2;
            argerr |= false;
         }
         else if(strcmp(argv[i], "--require-return") == 0){
            reqrtn = 1;
            argerr |= false;
         }
         else{
            //error
            argerr = true;
         }
      }
   }
   if (argerr) {
      fprintf(stderr, "%s: run without input arguments (except for --require-return or -t); the necessary arguments are hardcoded.\n", argv[0]);
      exit(EXIT_FAILURE);
   }
#ifdef PV_USE_MPI
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif // PV_USE_MPI

#undef REQUIRE_RETURN
#ifdef REQUIRE_RETURN
   int charhit;
   fflush(stdout);
   if( rank == 0 ) {
      printf("Hit enter to begin! ");
      fflush(stdout);
      charhit = getc(stdin);
   }
#ifdef PV_USE_MPI
   MPI_Bcast(&charhit, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
#endif // REQUIRE_RETURN

   int status;
   assert(reqrtn==0 || reqrtn==1);
   assert(threading == 0 || threading == 2);
   int cl_argc = 3+reqrtn+threading;
   char * cl_args[cl_argc];
   cl_args[0] = strdup(argv[0]);
   cl_args[1] = strdup("-p");

   cl_args[2] = strdup("input/MLPTrain.params");

   if(threading && reqrtn){
      assert(cl_argc==6);
      cl_args[3] = strdup("-t");
      cl_args[4] = strdup("4");
      cl_args[5] = strdup("--require-return");
   }
   else if (threading) {
      assert(cl_argc==5);
      cl_args[3] = strdup("-t");
      cl_args[4] = strdup("4");
   }
   else if (reqrtn){
      assert(cl_argc==4);
      cl_args[3] = strdup("--require-return");
   }
   status = buildandrun(cl_argc, cl_args, NULL, NULL, &addcustomgroup);
   if( status != PV_SUCCESS ) {
      fprintf(stderr, "%s: running with params file %s returned error %d.\n", cl_args[0], cl_args[2], status);
      exit(status);
   }

   free(cl_args[2]);
   cl_args[2] = strdup("input/MLPTest.params");
   status = buildandrun(cl_argc, cl_args, NULL, NULL, &addcustomgroup);
   if( status != PV_SUCCESS ) {
      fprintf(stderr, "%s: running with params file %s returned error %d.\n", cl_args[0], cl_args[2], status);
   }

   free(cl_args[2]);
   cl_args[2] = strdup("input/AlexTrain.params");
   status = buildandrun(cl_argc, cl_args, NULL, NULL, &addcustomgroup);
   if( status != PV_SUCCESS ) {
      fprintf(stderr, "%s: running with params file %s returned error %d.\n", cl_args[0], cl_args[2], status);
   }

   free(cl_args[2]);
   cl_args[2] = strdup("input/AlexTest.params");
   status = buildandrun(cl_argc, cl_args, NULL, NULL, &addcustomgroup);
   if( status != PV_SUCCESS ) {
      fprintf(stderr, "%s: running with params file %s returned error %d.\n", cl_args[0], cl_args[2], status);
   }








#ifdef PV_USE_MPI
   MPI_Finalize();
#endif

   for (int i=0; i<cl_argc; i++) {
      free(cl_args[i]);
   }
   return status==PV_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}

void * addcustomgroup(const char * keyword, const char * groupname, HyPerCol * hc) {
   void* addedGroup= NULL;
   if ( !strcmp(keyword, "ComparisonLayer") ) {
      addedGroup = new ComparisonLayer(groupname, hc);
   }
   if ( !strcmp(keyword, "InputLayer") ) {
      addedGroup = new InputLayer(groupname, hc);
   }
   if ( !strcmp(keyword, "GTLayer") ) {
      addedGroup = new GTLayer(groupname, hc);
   }
   if (!addedGroup) {
      fprintf(stderr, "Group \"%s\": Unable to create %s\n", groupname, keyword);
      exit(EXIT_SUCCESS);
   }
   checknewobject((void *) addedGroup, keyword, groupname, hc);
   return addedGroup;
}
