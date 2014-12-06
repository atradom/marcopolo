// Marco polo matched filter generator using the STK toolkit
// atr 10/30/14


#include "Fir.h"			// stk FIR filter
#include <algorithm>		// needed for vector reverse
#include "FileWvOut.h"		// for writing wav files
#include "FileWvIn.h"		// for reading wav files
#include "RtAudio.h"

#include <signal.h>
#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <algorithm>		// needed for vector reverse

using namespace stk;

// globals
bool done;
StkFrames frames;
static void finish(int ignore){ done = true; }


// This tick() function handles sample computation only.  It will be
// called automatically when the system needs a new buffer of audio
// samples.
int tick( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  FileWvIn *input = (FileWvIn *) userData;
  register StkFloat *samples = (StkFloat *) outputBuffer;

  input->tick( frames );
  for ( unsigned int i=0; i<frames.size(); i++ )
    *samples++ = frames[i];

  if ( input->isFinished() ) {
    done = true;
    return 1;
  }
  else
    return 0;
}



int main()
{
  const unsigned int marcoLength = 10;
  unsigned int channels = 1;	
  const unsigned int Fs = 44100;	// audio sample rate
  FileWvOut outfile;
  FileWvIn input;
  
  // Set the global sample rate before creating class instances.
  Stk::setSampleRate( (StkFloat) Fs );

  // Initialize our WvIn and RtAudio pointers.
  RtAudio dac;


  // Try to load the soundfile.
  try {
    input.openFile("Noisy_Miner_chirp_mono.wav" );
  }
  catch ( StkError & ) {
    exit( 1 );
  }
  
  // Set input read rate based on the default STK sample rate.
  double rate = 1.0;
  double in_rate;
  in_rate =  input.getFileRate();
  rate = in_rate / Stk::sampleRate();
  input.setRate( rate );  
  
  // Find out how many channels we have.
  channels = input.channelsOut();
  std::cout << "input file has " << channels << " channels at "<< in_rate <<  "Hz  sample rate " << std::endl;

  std::cout << " we want to run a rate of " << Stk::sampleRate() << " and will sample the file by a factor of "  << rate << std::endl;

  // Figure out how many bytes in an StkFloat and setup the RtAudio stream.
  RtAudio::StreamParameters parameters;
  parameters.deviceId = dac.getDefaultOutputDevice();
  parameters.nChannels = channels;
  RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
  unsigned int bufferFrames = RT_BUFFER_SIZE;
  try {
    dac.openStream( &parameters, NULL, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &tick, (void *)&input );
  }
  catch ( RtAudioError &error ) {
    error.printMessage();
    //goto cleanup;
    return(0);
  }

  // Install an interrupt handler function.
	(void) signal(SIGINT, finish);

  // Resize the StkFrames object appropriately.
  frames.resize( bufferFrames, channels );

  try {
    dac.startStream();
  }
  catch ( RtAudioError &error ) {
    error.printMessage();
    //goto cleanup;
    return(0);
  }

  // Block waiting until callback signals done.
  while ( !done )
    Stk::sleep( 100 );
  
  // By returning a non-zero value in the callback above, the stream
  // is automatically stopped.  But we should still close it.
  try {
    dac.closeStream();
  }
  catch ( RtAudioError &error ) {
    error.printMessage();
    //goto cleanup;
    return(0);
  }


  StkFrames output( marcoLength, 1 );   // initialize StkFrames to marcoLength frames and 1 channel (default: interleaved)
  output[0] = 1.0;						// pre-charge the filter so we get impulse response


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
