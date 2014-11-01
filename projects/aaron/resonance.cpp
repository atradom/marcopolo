// example of feeding a resonance filter with noise using stk toolkit
// atr 11/1/14

#include "BiQuad.h"
#include "Noise.h"
#include "FileWvOut.h"		// for writing wav files
using namespace stk;

int main()
{
	StkFrames output(20000,1);		// initialize frame buffer
	
	Noise noise;				// noise generator
	
	BiQuad biquad;				// resonant filter
	
	FileWvOut outfile;			// output wav file
	
	double resonantFrequency=440.0;
	double radius=0.9999;
	bool normalize=true;
	biquad.setResonance(resonantFrequency, radius, normalize);  //
	unsigned int channels = 1;
	
	  // try to open the output file
	try {
		outfile.openFile( "resonant.wav", channels, FileWrite::FILE_WAV, Stk::STK_SINT16 );
	}
	catch ( StkError & ) {
		return(0);
	}
 
 


	for (unsigned int i=0; i<output.size(); i++) {
		output[i]=biquad.tick(noise.tick());
		outfile.tick(output[i]);
	}
	
	return 0;
}
		
