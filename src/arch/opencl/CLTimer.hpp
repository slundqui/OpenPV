/*
 * CLTimer.hpp
 *
 * Sheng Lundquist
 */

#ifndef CLTIMER_HPP_
#define CLTIMER_HPP_

#include "pv_opencl.h"
#include "../../utils/Timer.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace PV {

/**
 * A timer class to time OpenCL events
 * Note: There is no start/stop in CLTimer. Instead, a timer event gets made that needs to get attached to the
 * OpenCL command that is being timed. Calling the blocking function accumulateTime will grab the time of the attached
 * OpenCL command. 
 */
class CLTimer : public Timer{
#ifdef PV_USE_OPENCL
   public:
      CLTimer(double init_time=0.0);
      CLTimer(const char * timermessage, double init_time=0.0);
      CLTimer(const char * objname, const char * objtype, const char * timertype, double init_time=0.0);
      virtual ~CLTimer();

      /**
       * A function to put an instruction on the GPU queue to start timing
       */
      virtual double start();
      /**
       * A function to put an instruction on the GPU queue to stop timing
       */
      virtual double stop();
      double accumulateTime();
      virtual int fprint_time(FILE * stream);
      /**
       * A setter function to set the stream to time
       */
      void setCommand(cl_command_queue commands){this->commands = commands;}
      
   private:
      cl_event startEvent;
      cl_event stopEvent;
      cl_command_queue commands;          // compute command queue
      double time;
#endif
};

}
#endif
