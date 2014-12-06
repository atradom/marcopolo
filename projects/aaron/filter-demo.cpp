// IIR filter impulse response using the STK toolkit


#include "Iir.h"
using namespace stk;

int main()
{
  StkFrames output( 20, 1 );   // initialize StkFrames to 20 frames and 1 channel (default: interleaved)
  output[0] = 1.0;				// pre-charge the filter so we get impulse response

  std::vector<StkFloat> numerator( 5, 0.1 ); // create and initialize numerator coefficients (5 elements of 0.1)
  
  // one way to create the denominator vector
  StkFloat den[] = {1.0, 0.3, -0.5};
  std::vector<StkFloat> denominator(den, den + sizeof(den) / sizeof(StkFloat) );
 
  // another way to create the denominator
  //std::vector<StkFloat> denominator;         // create empty denominator coefficients
  //denominator.push_back( 1.0 );              // populate our denomintor values
  //denominator.push_back( 0.3 );
  //denominator.push_back( -0.5 );


  Iir filter( numerator, denominator );		// create the filter object using the IIR constructor

  filter.tick( output );					// execute the filter on the frame
  
  // print the impulse response
  for ( unsigned int i=0; i<output.size(); i++ ) {
    //std::cout << "i = " << i << " : output = " << output[i] << std::endl;
    std::cout << output[i] << std::endl;		// write the impulse response to the standard output
  }

  return 0;
}
