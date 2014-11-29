//callback realtime sine wave generator using stk toolkit

#include "SineWave.h"
#include "RtAudio.h"


using namespace stk;


// tick() callback function
// called automatically when the system needs a new buffer of audio samples

int tick( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
		double streamTime, RtAudioStreamStatus status, void *dataPointer)
		
{
	SineWave *sine= (SineWave *) dataPointer;
	register StkFloat *samples=(StkFloat *) outputBuffer;
	
	for (unsigned int i=0; i<nBufferFrames; i++)
		*samples++ =sine->tick();
		
	return 0;
}



int main()
{
	//set the global sample rate
	Stk::setSampleRate(44100.0);
	
	SineWave sine;
	RtAudio dac;
	
	// set up RtAudio stream
	RtAudio::StreamParameters parameters;
	parameters.deviceId=dac.getDefaultOutputDevice();
	parameters.nChannels=1;
	RtAudioFormat format=(sizeof(StkFloat)==8) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
	unsigned int bufferFrames=RT_BUFFER_SIZE;
	
	try {
		// define and open the default realtime output device for one channel playback
		dac.openStream(&parameters, NULL, format, (unsigned int)Stk::sampleRate(), &bufferFrames,
			&tick, (void *)&sine);
	}
	catch (RtAudioError &error) {
		error.printMessage();
		goto cleanup;
	}
	
	sine.setFrequency(440.0);
	
	try {
		dac.startStream();
	}
	catch (RtAudioError &error) {
		error.printMessage();
		goto cleanup;
	}

	// Block waiting here
	char keyhit;
	std::cout << "\nPlaying ... press <enter> to go up an octave. \n";
	std::cin.get(keyhit);
	
	// switch to 880Hz for a while
	sine.setFrequency(880.0);
	std::cin.get(keyhit);
	std::cout << "\nPlaying ... press <enter> to quit. \n";
	
	//shutdown the output stream
	try {
		dac.closeStream();
	}
	catch (RtAudioError &error) {
		error.printMessage();
	}
	
	cleanup:
	
	return 0;
}
