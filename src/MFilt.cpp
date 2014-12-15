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

#include "MFilt.h"

namespace stk {
MFilt :: MFilt() :
  // The default constructor disables triggering for now
  threshold( 100 )  // set it high?

{}

MFilt :: MFilt( StkFloat trigger_threshold )
{
  threshold = trigger_threshold;
}

MFilt :: ~MFilt()
{
}



} // stk namespace
