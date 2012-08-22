/*
 * HyPerCol.h
 *
 *  Created on: Jul 30, 2008
 *      Author: Craig Rasmussen
 */

#ifndef HYPERCOL_HPP_
#define HYPERCOL_HPP_

#include "HyPerColRunDelegate.hpp"
#include "../layers/PVLayer.h"
#include "../connections/HyPerConn.hpp"
#include "../io/PVParams.hpp"
#include "../include/pv_types.h"
#include "../utils/Timer.hpp"
#include "../io/ColProbe.hpp"
#include <time.h>
#include <sys/stat.h>

#include "../arch/opencl/CLDevice.hpp"

namespace PV {

class HyPerLayer;
class InterColComm;
class HyPerConn;
class ColProbe;
class PVParams;

class HyPerCol {

public:

   HyPerCol(const char * name, int argc, char * argv[], PVParams * params=NULL);
   // HyPerCol(const char* name, int argc, char* argv[], const char * path); // Not defined in .cpp file
   virtual ~HyPerCol();

   int initFinish(void); // call after all layers/connections have been added
   int initializeThreads(int device);
#ifdef PV_USE_OPENCL
   int finalizeThreads();
#endif //PV_USE_OPENCL

   int run()  {return run(numSteps);}
   int run(int nTimeSteps);

   float advanceTime(float time);
   int   exitRunLoop(bool exitOnFinish);

   int loadState();
   int columnId();

//   int deliver(PVConnection* conn, PVRect preRegion, int count, float* buf);

   int addLayer(HyPerLayer * l);
   int addConnection(HyPerConn * conn);

   HyPerLayer * getLayerFromName(const char * layerName);
   HyPerConn * getConnFromName(const char * connectionName);

   HyPerLayer * getLayer(int which)       {return layers[which];}
   HyPerConn  * getConnection(int which)  {return connections[which];}
   ColProbe * getColProbe(int which)      {return probes[which];}

   char * getName()                       {return name;}
   char * getPath()                       {return path;}
   char * getOutputPath()                 {return outputPath;}
   int getNxGlobal()                      {return nxGlobal;}
   int getNyGlobal()                      {return nyGlobal;}

   CLDevice * getCLDevice()               {return clDevice;}

   InterColComm * icCommunicator()        {return icComm;}

   PVParams * parameters()                {return params;}

   bool  warmStartup()                    {return warmStart;}

   float getDeltaTime()                   {return deltaTime;}
   float simulationTime()                 {return simTime;}
   float getStopTime()                    {return stopTime;}

   int includeLayerName()                 {return filenamesContainLayerNames;}

   const char * inputFile()               {return image_file;}

   int numberOfTimeSteps()                {return numSteps;}

   int numberOfColumns();

   int numberOfLayers()                   {return numLayers;}
   int numberOfConnections()              {return numConnections;}
   int numberOfProbes()                   {return numProbes;}

   /** returns the number of border regions, either an actual image border or a neighbor **/
   int numberOfBorderRegions()            {return MAX_NEIGHBORS;}

   int commColumn(int colId);
   int commRow(int colId);
   int numCommColumns()                   {return icComm->numCommColumns();}
   int numCommRows()                      {return icComm->numCommRows();}

   // a random seed based on column id
   unsigned long getSeed() { return random_seed; }

   unsigned long getRandomSeed()
      {return (unsigned long) time((time_t *) NULL); } // Aug 21, 2012: Division by 1+columnId() moved to calling routine}

   void setDelegate(HyPerColRunDelegate * delegate)  {runDelegate = delegate;}

   int insertProbe(ColProbe * p);
   int outputState(float time);

private:
   int initialize(const char * name, int argc, char ** argv, PVParams * params);
   int ensureDirExists(const char * dirname);
   int checkDirExists(const char * dirname, struct stat * pathstat);
   int initPublishers();
   bool advanceCPWriteTime();
   int checkpointRead(const char * cpDir);
   int checkpointWrite(const char * cpDir);
   int outputParams(const char * filename);
   int checkMarginWidths();
   int zCheckMarginWidth(HyPerConn * conn, const char * dim, int patchSize, int scalePre, int scalePost, int prevStatus);
   int lCheckMarginWidth(HyPerLayer * layer, const char * dim, int layerSize, int layerGlobalSize, int prevStatus);

   int numSteps;
   int currentStep;
   int layerArraySize;
   int numLayers;
   int connectionArraySize;
   int numConnections;

   bool warmStart;
   bool isInitialized;     // true when all initialization has been completed
   bool checkpointReadFlag;    // whether to load from a checkpoint directory
   bool checkpointWriteFlag;   // whether to write from a checkpoint directory
   char * checkpointReadDir;   // name of the directory to read an initializing checkpoint from
   int cpReadDirIndex;  // checkpoint number within checkpointReadDir to read
   char * checkpointWriteDir; // name of the directory to write checkpoints to
   int cpWriteStepInterval;
   int nextCPWriteStep;
   float cpWriteTimeInterval;
   float nextCPWriteTime;
   bool deleteOlderCheckpoints; // If true, whenever a checkpoint other than the first is written, the preceding checkpoint is deleted. Default is false.
   char lastCheckpointDir[PV_PATH_MAX]; // Holds the last checkpoint directory written; used if deleteOlderCheckpoints is true.

   bool suppressLastOutput; // If checkpointWriteFlag is false and this flag is false, on exit a checkpoint is sent to the {outputPath}/Last directory.
                            // If checkpointWriteFlag is false and this flag is true, no checkpoint is done on exit.
                            // The flag has no effect if checkpointWriteFlag is true (in which case a checkpoint is written on exit to the next directory in checkpointWriteDir

   float simTime;          // current time in milliseconds
   float stopTime;         // time to stop time
   float deltaTime;        // time step interval
   int progressStep;       // How many timesteps between outputting progress

   CLDevice * clDevice;    // object for running kernels on OpenCL device

   HyPerLayer ** layers;
   HyPerConn  ** connections;

   char * name;
   char * path;
   char * outputPath;     // path to output file directory
   char * outputNamesOfLayersAndConns;  // path to file for writing list of layer names and connection names
   char * image_file;
   int nxGlobal;
   int nyGlobal;

   bool           ownsParams; // True if params was created from params file by initialize, false if params was passed in the constructor
   bool           ownsInterColComm; // True if icComm was created by initialize, false if passed in the constructor
   PVParams     * params; // manages input parameters
   InterColComm * icComm; // manages communication between HyPerColumns};

   HyPerColRunDelegate * runDelegate; // runs time loop

   Timer * runTimer;

   int numProbes;
   ColProbe ** probes;

   int filenamesContainLayerNames; // Controls the form of layers' clayer->activeFP
                                   // Value 0: layers have form a5.pvp
                                   // Value 1: layers have form a5_NameOfLayer.pvp
                                   // Value 2: layers have form NameOfLayer.pvp

   unsigned long random_seed;

}; // class HyPerCol

} // namespace PV

#endif /* HYPERCOL_HPP_ */
