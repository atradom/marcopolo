// FIR filter impulse response using the STK toolkit

#include "Fir.h"			// stk FIR filter
#include <stdlib.h>
#include <algorithm>		// needed for vector reverse
#include "FileWvOut.h"		// for writing wav files

using namespace stk;

int main()
{
 const unsigned int marcoLength = 10000;
 unsigned int channels = 1;	
  //const unsigned int Fs = 44100;	// audio sample rate
  FileWvOut outfile;
  
  StkFrames output( marcoLength, 1 );   // initialize StkFrames to marcoLength frames and 1 channel (default: interleaved)
  output[0] = 1.0;				// pre-charge the filter so we get impulse response

  //std::vector<StkFloat> numerator( 5, 0.1 ); // create and initialize numerator coefficients (5 elements of 0.1)
 
 
   // try to open the output file
   try {
    outfile.openFile( "marco.wav", channels, FileWrite::FILE_WAV, Stk::STK_SINT16 );
  }
  catch ( StkError & ) {
    return(0);
  }
 

//  std::cout << "max random number: " << RAND_MAX << std::endl;
  
  // create "Marco" audio pattern
 
  double rn;	// random number
  std::vector<StkFloat> marco;		// marco audio sequence
  for ( unsigned int ii=0; ii< marcoLength; ii++) {
	  rn = (double)rand()/RAND_MAX;
	  marco.push_back(rn);
	  //std::cout << rn << "[" << ii << "] " << ", ";
  }
  //std::cout << "end of audio pattern" << std::endl << std::endl;
  
  
  // now reverse the Marco sequence to create the matched filter
  std::vector<StkFloat> marcoFilt;		// marco matched filter coefficients
  marcoFilt=marco;
  std::reverse(marcoFilt.begin(), marcoFilt.end());
  
  std::vector<StkFloat>::iterator it1;
  std::vector<StkFloat>::iterator it2;
  it2 = marco.begin();		// pointer to start of marco sequence
  
  
  for (it1=marcoFilt.begin(); it1 !=marcoFilt.end(); it1++) { 
	  std::cout << *it1 << ", "<< *it2 << std::endl;
	  it2++; 
  } 
//  std::cout << "end of matched filter pattern" << std::endl;
  
  
  
  // one way to create the denominator vector
  //StkFloat den[] = {1.0, 0.3, -0.5};
  //std::vector<StkFloat> denominator(den, den + sizeof(den) / sizeof(StkFloat) );
 
  // another way to create the denominator
  //std::vector<StkFloat> denominator;         // create empty denominator coefficients
  //denominator.push_back( 1.0 );              // populate our denomintor values
  //denominator.push_back( 0.3 );
  //denominator.push_back( -0.5 );


  Fir filter( marcoFilt );		// create the filter object using the FIR constructor

//  std::cout << "impulse response of matched filter" << std::endl;
  filter.tick( output );					// execute the filter on the frame
  outfile.tick(output);
  
  // print the impulse response
//  for ( unsigned int i=0; i<output.size(); i++ ) {
//    std::cout << "i = " << i << " : output = " << output[i] << std::endl;
    //std::cout << output[i] << std::endl;		// write the impulse response to the standard output
//  }
  
  
  return 0;
}
