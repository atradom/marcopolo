#ifndef STK_MFILT_H
#define STK_MFILT_H

#include "Fir.h"
#include <time.h>		// needed for round trip timing

namespace stk {

/***************************************************/
/*! \class MFilt
    \brief STK Matched Filter class.

    This class provides a matched filter class, which is derived from the FIR filter parent class

    the matched filter is identical to an FIR filter with the addition of:
    1) a detection threshold to indicate when the pattern has been matched
    2) a timer that can be queried and reset 

    by Aaron Radomski - 2014.
*/
/***************************************************/

class MFilt : public Fir		// derived from FIR parent class
{
public:
  //! Default constructor .
  MFilt( void );

  //! Overloaded constructor which takes filter coefficients.
  /*!
    An StkError can be thrown if the coefficient vector size is
    zero.
  */
  MFilt( StkFloat trigger_threshold );

  //! Class destructor.
  ~MFilt( void );
  
  
  //! Input one sample to the filter and return one output. 
  // Set trigger time member variable if the target pattern was detected
  StkFloat mtick( StkFloat input );


  // set the trigger threshold for the matched filter (amplitude at which it says pattern is there)
  void setThreshold(StkFloat trigger_threshold);


  // clear the trigger time member variable
  void clearTrigger( void );
  
  
  // get the last time the filter triggered
  struct timespec getTriggerTime(void);
 
protected:
  StkFloat threshold;		// output amplitude that indicates we detected the pattern in the input
  struct timespec trigger_time;	// for timing  (.tv_sec = seconds since 1970, .tv_nsec=nanoseconds into the current second


};


inline StkFloat MFilt :: mtick( StkFloat input )
{
  lastFrame_[0] = 0.0;
  inputs_[0] = gain_ * input;

  for ( unsigned int i=(unsigned int)(b_.size())-1; i>0; i-- ) {
    lastFrame_[0] += b_[i] * inputs_[i];
    inputs_[i] = inputs_[i-1];
  }
  lastFrame_[0] += b_[0] * inputs_[0];

  if (lastFrame_[0] > threshold) {
	  clock_gettime(CLOCK_REALTIME, &trigger_time);
	  //std::cout << " foundit = " << lastFrame_[0] << "@" << trigger_time.tv_nsec << "\n";
  }
  return lastFrame_[0];
}


inline void MFilt :: setThreshold( StkFloat trigger_threshold)
{
	threshold = trigger_threshold;
}

inline void MFilt :: clearTrigger( void )
{
	trigger_time.tv_sec=0;
	trigger_time.tv_nsec=0;
}


inline struct timespec MFilt :: getTriggerTime( void )
{
	return trigger_time;
}


} // stk namespace

#endif
