//realtime sine wave generator using stk toolkit

#include "SineWave.h"
#include "RtWvOut.h"
#include <cstdlib>

using namespace stk;

int main()
{
	//set the global sample rate
	Stk::setSampleRate(44100.0);
	Stk::showWarnings(true);
	
	int nFrames=100000;
	SineWave sine;
	RtWvOut *dac=0;
	
	try {
		// define and open the default realtime output device for one channel playback
		dac= new RtWvOut(1);
	}
	catch (StkError &) {
		exit(1);
	}
	
	sine.setFrequency(441.0);
	
	for (int i=0; i<nFrames; i++) {
		try {
			dac->tick(sine.tick());
		}
		catch (StkError &) {
			goto cleanup;
		}
	}
	
	cleanup:
	delete dac;
	
	return 0;
}
