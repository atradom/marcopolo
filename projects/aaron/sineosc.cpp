//sineosc.cpp
// modified to read a bird chirp and loop that instead of a sine wave

#include "FileLoop.h"
#include "FileWvOut.h"
#include <cstdlib>
using namespace stk;

int main()
{
	StkFloat d;						// sample data
	unsigned long inputSize;		// size of the input file in sample frames
	double inputDuration;			// duration of the input file loop
	unsigned int channels = 1;
	//set global sample rate
	Stk::setSampleRate(44100.0);
	
	int nFrames=100000;
	FileLoop input;
	FileWvOut output;
	
	// open the input and output files, with error checking
	try {
		// load the input file
		input.openFile("rawwaves/Noisy_Miner_chirp_stereo.wav", false);
	
		// open a 16 bit, one channel WAV formatted output
		output.openFile("many_chirps.wav", 1, FileWrite::FILE_WAV, Stk::STK_SINT16);
	}
	catch (StkError &) {
			exit(1);
	}
	
	  // Set input read rate based on the default STK sample rate.
	double rate;
	double inputRate;
	inputRate =  input.getFileRate();
	rate = inputRate / Stk::sampleRate();
	input.setRate( rate );  
  
	// Find out the length of the input file
	inputSize=input.getSize();
	inputDuration = inputSize/inputRate;
	std::cout << "input file is " << inputSize << " samples long with a duration of "  << inputDuration << " sec." << std::endl;
  
  
	// Find out how many channels we have.
	channels = input.channelsOut();
	std::cout << "input file has " << channels << " channels at "<< inputRate <<  "Hz  sample rate " << std::endl;

	std::cout << " we want to run a rate of " << Stk::sampleRate() << " and will sample the file by a factor of "  << rate << std::endl;
	
	
	// we want the chirps to be at the same frequency as they were recorded
	// so we adjust the looping rate of recorded loop
	// if you make the frequency negative, it reverses the loop!
	input.setFrequency(-1.0/inputDuration);
	
	// run the oscillator for 40000 samples, writing to the output file
	for (int i=0; i<nFrames; i++){
		try {
			d=input.tick();		// read 1 sample from the input file
			output.tick(d);		// write 1 sample to the output file
		}
		catch (StkError &) {
			exit(1);
		}
		
		// print the first 100 samples to the std output for debugging
		if (i<10){
			std::cout << d << std::endl;
		}	
	}	
		
	return 0;
	
}
