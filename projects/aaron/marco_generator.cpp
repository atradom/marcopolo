/**************  Marco Polo Distance Detector - Marco Generator Program  *********************/
// this program attempts to measure distance by sending a "marco" sound to a remote detector
// the detector responds with a "polo" sound that is hopefully detected by the sender
// the sender times this exchange and tries to estimate the distance
// * the program is a modified version of the Stk effects demo - cleaning is needed!
// this program is the Marco side of the detector

#include "Skini.h"
#include "SKINImsg.h"
#include "FileWvIn.h"		// Stk for reading wav files
#include "Fir.h"			// Stk FIR filter
#include "Messager.h"
#include "RtAudio.h"		// Stk Realtime audio

#include "MFilt.h"			// My new matched filter class derived from FIR
							// I put this in the Stk include directory (is that correct?)

#include <cstdlib>
#include <stdlib.h>
#include <signal.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <map>
#include <time.h>
using std::min;



using namespace stk;


void usage(void) {
  // Error function in case of incorrect command-line argument specifications
  std::cout << "\nuseage: effects flags \n";
  std::cout << "    where flag = -s RATE to specify a sample rate,\n";
  std::cout << "    flag = -ip for realtime SKINI input by pipe\n";
  std::cout << "           (won't work under Win95/98),\n";
  std::cout << "    and flag = -is <port> for realtime SKINI input by socket.\n";
  exit(0);
}


// global variables

/*

can get rid of this filter someday -it was used for early testing

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 44100 Hz

* 0 Hz - 4000 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 3.6483420584837547 dB

* 7000 Hz - 22050 Hz
  gain = 0
  desired attenuation = -30 dB
  actual attenuation = -31.13452537592425 dB

*/

#define FILTER_TAP_NUM 15

StkFloat fir[FILTER_TAP_NUM] = {
  -0.02566254990456756,
  -0.022132905716131836,
  -0.006844859170844624,
  0.03483590625488368,
  0.09942041492575888,
  0.17128018762092148,
  0.22789464472218476,
  0.24939961132445515,
  0.22789464472218476,
  0.17128018762092148,
  0.09942041492575888,
  0.03483590625488368,
  -0.006844859170844624,
  -0.02213290571613185,
  -0.02566254990456756
};


std::vector<StkFloat> fir_coeff(fir, fir + sizeof(fir) / sizeof(StkFloat) );

StkFrames frames;						// used to read in the marco wavfile
StkFrames poloframes;					// used to read in the polo wavfile
std::vector<StkFloat> polo;			// polo audio sequence
std::vector<StkFloat> poloFilt;		// polo matched filter coefficients


bool done;
static void finish(int ignore){ done = true; }


// function to calculate time deltas using the real time clock
// tf = ending time (timespec structure)
// ts = starting time (timespec structure)
// return value is floating point in seconds
double delta_time(timespec ts, timespec tf) {
	long int diff_sec, diff_ns;		// time differences in seconds and nanoseconds
	diff_sec = tf.tv_sec - ts.tv_sec;
	diff_ns = tf.tv_nsec - ts.tv_nsec;
	//std::cout << "tf = " << tf.tv_sec << " . " << tf.tv_nsec << "\n";
	//std::cout << "ts = " << ts.tv_sec << " . " << ts.tv_nsec << "\n";
	if (diff_ns < 0) {
		diff_ns = diff_ns + 1000000000;
		diff_sec--;
	}
	//std::cout << "diff = " << diff_sec << " . " << diff_ns << "\n";
	return (double)(diff_sec + diff_ns/1e9);
}
		



// The TickData structure holds all the class instances and data that
// are shared by the various processing functions.
struct TickData {
  unsigned int effectId;	// current effect ID
  Fir		filter;			// id=8  *atr
  MFilt		Matched_filter;
  Messager messager;		// for reading and parsing control messages
  Skini::Message message;	// control message
  StkFloat lastSample;
  StkFloat t60;				// T60 decay time (for reverb effects)
  int counter;
  bool settling;			// true if in settling state
  bool haveMessage;			// if a control message is available
  struct timespec start_time;			// for timing
  long int time_difference;		// for timing
  struct timespec gettime_now;	// for timing  (.tv_sec = seconds since 1970, .tv_nsec=nanoseconds into the current second
  StkFloat snapshot;			// instantaneous audio output for debuggin 
  FileWvIn *marcoFile;				// "Marco" sound file
  bool playing;					// playing the Marco sound
  bool triggered;			// Polo was heard

  // Default constructor.
  TickData()
    : effectId(9), t60(1.0), counter(0),
      settling( false ), haveMessage( false ),
      triggered(false) {
		  Matched_filter.setThreshold(0.06);
		  
		  }
};

#define DELTA_CONTROL_TICKS 64 // default sample frames between control input checks








// The processMessage() function encapsulates the handling of control
// messages.  It can be easily relocated within a program structure
// depending on the desired scheduling scheme.
// *atr - we may not need this in the end, but kept it for testing
void processMessage( TickData* data )
{
  register unsigned int value1 = data->message.intValues[0];
  register StkFloat value2 = data->message.floatValues[1];
  register StkFloat temp = value2 * ONE_OVER_128;
  
  
  switch( data->message.type ) {

  case __SK_Exit_:
    if ( data->settling == false ) goto settle;
    done = true;
    return;

  case __SK_NoteOn_:
    std::cout << "note on\n";
    //data->envelope.setTarget( 1.0 );
    break;

  case __SK_NoteOff_:
    std::cout << "note off\n";
    //data->envelope.setTarget( 0.0 );
    break;

  case __SK_ControlChange_:
    // Change all effect values so they are "synched" to the interface.
    switch ( value1 ) {

    case 20: { // effect type change
      std::cout << "effect type change=";
      int type = data->message.intValues[1];
      data->effectId = (unsigned int) type;
       std::cout << type << "\n";
      break;
    }

    case 22: // effect parameter change 1
      std::cout << "effect param 1 change: " << temp << "\n";
//     data->echo.setDelay( (unsigned long) (temp * Stk::sampleRate() * 0.95) );
//     data->lshifter.setShift( 1.4 * temp + 0.3 );
		data->Matched_filter.setThreshold(temp);
      break;

    case 23: // effect parameter change 2
      std::cout << "effect param 2 change\n";
//      data->chorus.setModDepth( temp * 0.2 );
      break;

    case 44: // effect mix
      std::cout << "gain change:  " << temp << "\n";
     data->Matched_filter.setGain( temp );
      break;

    default:
      break;
    }

  } // end of type switch

  data->haveMessage = false;
  return;

 settle:
  // Exit and program change messages are preceeded with a short settling period.
  //data->envelope.setTarget( 0.0 );
  data->counter = (int) (0.3 * data->t60 * Stk::sampleRate());
  data->settling = true;
}






// The tick() function handles sample computation and scheduling of
// control updates.  It will be called automatically by RtAudio when
// the system needs a new buffer of audio samples.
int tick( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *dataPointer )
{
  TickData *data = (TickData *) dataPointer;
  register StkFloat *oSamples = (StkFloat *) outputBuffer, *iSamples = (StkFloat *) inputBuffer;
  register StkFloat sample;
  StkFloat marcoSample; // used when reading the polofile one sample at a time
 // Effect *effect;
  int i, counter, nTicks = (int) nBufferFrames;
  FileWvIn *marcoFile = (FileWvIn *) data->marcoFile;   // pointer to the marco file we want to play if marco is detected


  while ( nTicks > 0 && !done ) {

    if ( !data->haveMessage ) {
      data->messager.popMessage( data->message );
      if ( data->message.type > 0 ) {
        data->counter = (long) (data->message.time * Stk::sampleRate());
        data->haveMessage = true;
      }
      else
        data->counter = DELTA_CONTROL_TICKS;
    }

    counter = min( nTicks, data->counter );
    data->counter -= counter;
    for ( i=0; i<counter; i++ ) {
      if (data->effectId==8) {    // atr - lowpass filter - remove someday
		sample = data->filter.tick(*iSamples++);
		*oSamples++ = sample; // two channels interleaved
        *oSamples++ = sample;
        
      }
      else if (data->effectId==9) {    // atr - matched filter
      
		if (data->triggered) {  // state = playing marco sound
			//std::cout << "want to play marco =" << data->triggered  << "\n"; 
			
			try {
				marcoSample = marcoFile->tick();
			}
			catch ( StkError & ) {
				exit( 1 );
			}
			
			if ( marcoFile->isFinished() ) {
				data->triggered = false;
				marcoFile->reset();
				//std::cout << "finished playing marco\n";
				marcoSample=0;
			}
			*oSamples++ = marcoSample;	// the 2 output channels are interleaved, so insert 2 copies in the output stream
			*oSamples++ = marcoSample;
			sample = data->Matched_filter.mtick(0);  // run the matched filter with no input
			data->snapshot = sample;  // put the latest sample in the Tickdata structure (for debugging/monitoring)

			data->Matched_filter.clearTrigger();   // don't trigger on matched filter while playing
		}
		else {					// state = listening for polo sound
			*oSamples++ = 0;	// send silence to speakers
			*oSamples++ = 0;
			sample = data->Matched_filter.mtick(*iSamples++);  // run the matched filter for 1 sample
			data->snapshot = sample;  // put the latest sample in the Tickdata structure (for debugging/monitoring)
			data->gettime_now = data->Matched_filter.getTriggerTime();	// get the last trigger time of the matched filter
			//if ((data->gettime_now.tv_nsec !=0)) {
			//	std::cout << "   *** polo at =  " << data->gettime_now.tv_sec << " , " << data->gettime_now.tv_nsec << " ns\n";
			//}

		}
			
        
      } // if effectId==9
      else { 
        *oSamples++ = 0;
        *oSamples++ = 0;
      }
      nTicks--;
    } // end for loop
    if ( nTicks == 0 ) break;

    // Process control messages.
    if ( data->haveMessage ) processMessage( data );
  } // end while

  return 0;
}

int main( int argc, char *argv[] )
{
  TickData data;
  RtAudio adac;
  int i;
  unsigned int channels=1;		// number of channels in the input file
  FileWvIn sndPattern;			// sound pattern we want to match
  FileWvIn marcoFile;			// Marco request sound
  double rate = 1.0;			// file data rates
  double in_rate;
  double trigger_time=0.0;		// time delay from Marco sent to Polo received
  long int	timeout = 0;		// timeout counter (if we never hear polo back from slave)
   
  RtAudio::DeviceInfo info;
  
    // Create an api map.
  std::map<int, std::string> apiMap;
  apiMap[RtAudio::MACOSX_CORE] = "OS-X Core Audio";
  apiMap[RtAudio::WINDOWS_ASIO] = "Windows ASIO";
  apiMap[RtAudio::WINDOWS_DS] = "Windows Direct Sound";
  apiMap[RtAudio::UNIX_JACK] = "Jack Client";
  apiMap[RtAudio::LINUX_ALSA] = "Linux ALSA";
  apiMap[RtAudio::LINUX_OSS] = "Linux OSS";
  apiMap[RtAudio::RTAUDIO_DUMMY] = "RtAudio Dummy";
  std::vector< RtAudio::Api > apis;
  RtAudio :: getCompiledApi( apis );

  std::cout << "\nCompiled APIs:\n";
  for ( unsigned int i=0; i<apis.size(); i++ )
    std::cout << "  " << apiMap[ apis[i] ] << std::endl;
 
   std::cout << "\nCurrent API: " << apiMap[ adac.getCurrentApi() ] << std::endl;

  unsigned int devices = adac.getDeviceCount();
  std::cout << "\nFound " << devices << " device(s) ...\n";

  for (unsigned int i=0; i<devices; i++) {
    info = adac.getDeviceInfo(i);

    std::cout << "\nDevice Name = " << info.name << '\n';
    if ( info.probed == false )
      std::cout << "Probe Status = UNsuccessful\n";
    else {
      std::cout << "Probe Status = Successful\n";
      std::cout << "Output Channels = " << info.outputChannels << '\n';
      std::cout << "Input Channels = " << info.inputChannels << '\n';
      std::cout << "Duplex Channels = " << info.duplexChannels << '\n';
      if ( info.isDefaultOutput ) std::cout << "This is the default output device.\n";
      else std::cout << "This is NOT the default output device.\n";
      if ( info.isDefaultInput ) std::cout << "This is the default input device.\n";
      else std::cout << "This is NOT the default input device.\n";
      if ( info.nativeFormats == 0 )
        std::cout << "No natively supported data formats(?)!";
      else {
        std::cout << "Natively supported data formats:\n";
        if ( info.nativeFormats & RTAUDIO_SINT8 )
          std::cout << "  8-bit int\n";
        if ( info.nativeFormats & RTAUDIO_SINT16 )
          std::cout << "  16-bit int\n";
        if ( info.nativeFormats & RTAUDIO_SINT24 )
          std::cout << "  24-bit int\n";
        if ( info.nativeFormats & RTAUDIO_SINT32 )
          std::cout << "  32-bit int\n";
        if ( info.nativeFormats & RTAUDIO_FLOAT32 )
          std::cout << "  32-bit float\n";
        if ( info.nativeFormats & RTAUDIO_FLOAT64 )
          std::cout << "  64-bit float\n";
      }
      if ( info.sampleRates.size() < 1 )
        std::cout << "No supported sample rates found!";
      else {
        std::cout << "Supported sample rates = ";
        for (unsigned int j=0; j<info.sampleRates.size(); j++)
          std::cout << info.sampleRates[j] << " ";
      }
      std::cout << std::endl;
    }
  }
  std::cout << std::endl;
 
 
 
 
  

  if ( argc < 2 || argc > 6 ) usage();

  // If you want to change the default sample rate (set in Stk.h), do
  // it before instantiating any objects!  If the sample rate is
  // specified in the command line, it will override this setting.
  Stk::setSampleRate( 44100.0 );

  // Parse the command-line arguments.
  unsigned int port = 2001;
  for ( i=1; i<argc; i++ ) {
    if ( !strcmp( argv[i], "-is" ) ) {
      if ( i+1 < argc && argv[i+1][0] != '-' ) port = atoi(argv[++i]);
      data.messager.startSocketInput( port );
    }
    else if (!strcmp( argv[i], "-ip" ) )
      data.messager.startStdInput();
    else if ( !strcmp( argv[i], "-s" ) && ( i+1 < argc ) && argv[i+1][0] != '-')
      Stk::setSampleRate( atoi(argv[++i]) );
    else
      usage();
  }

  // Allocate the adac here.
  RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
  RtAudio::StreamParameters oparameters, iparameters;
  oparameters.deviceId = adac.getDefaultOutputDevice();
  oparameters.nChannels = 2;
  iparameters.deviceId = adac.getDefaultInputDevice();
  iparameters.nChannels = 1;
  //unsigned int bufferFrames = RT_BUFFER_SIZE;
  unsigned int bufferFrames = 128;
  
    // /*atr* Let RtAudio print messages to stderr.
  adac.showWarnings( true );
  
  try {
    adac.openStream( &oparameters, &iparameters, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &tick, (void *)&data );
    std::cout << "opened the adac stream\n";
  }
  catch ( RtAudioError& error ) {
    error.printMessage();
    goto cleanup;
  }

 // data.envelope.setRate( 0.001 );

  // Install an interrupt handler function.
	(void) signal( SIGINT, finish );




// load the "Marco" request sound
  // Try to load the soundfile.
  try {
    marcoFile.openFile("Noisy_Miner_chirp_mono.wav" );
  }
  catch ( StkError & ) {
    exit( 1 );
  }
  
  // Set input read rate based on the default STK sample rate.

  in_rate =  marcoFile.getFileRate();
  rate = in_rate / Stk::sampleRate();
  marcoFile.setRate( rate );  
  data.marcoFile = &marcoFile;  // point the TickData structure to the polo input file
  
  // Find out how many channels we have.
  channels = marcoFile.channelsOut();
  std::cout << "Marco file has " << channels << " channels at "<< in_rate <<  "Hz  sample rate " << std::endl;

  std::cout << " we want to run a rate of " << Stk::sampleRate() << " and will sample the file by a factor of "  << rate << std::endl;

  std::cout << " the file size is " << marcoFile.getSize() << std::endl;


// set up the matched filter (move to a function later?)
  // Try to load the soundfile.
  try {
    sndPattern.openFile("Noisy_Miner_chirp_mono+10.wav" );
  }
  catch ( StkError & ) {
    exit( 1 );
  }
  
  // Set input read rate based on the default STK sample rate.

  in_rate =  sndPattern.getFileRate();
  rate = in_rate / Stk::sampleRate();
  sndPattern.setRate( rate );  
  
  // Find out how many channels we have.
  channels = sndPattern.channelsOut();
  std::cout << "polo sound pattern file has " << channels << " channels at "<< in_rate <<  "Hz  sample rate " << std::endl;

  std::cout << " we want to run a rate of " << Stk::sampleRate() << " and will sample the file by a factor of "  << rate << std::endl;

  std::cout << " the file size is " << sndPattern.getSize() << std::endl;

  // Resize the StkFrames object appropriately.
  frames.resize( sndPattern.getSize(), channels );


	sndPattern.tick( frames );
	std::cout << frames.size() << "\n";
  
  
 // now, store the whole input sequence into a vector 
  for ( unsigned int i=0; i<frames.size(); i++ ) {
	polo.push_back(frames[i]);
	//std::cout << frames[i] << "\n";
  }

// now create the matched filter coefficients by reversing the input sequence
  poloFilt=polo;
  std::reverse(poloFilt.begin(), poloFilt.end());
  data.Matched_filter.setCoefficients(poloFilt, false);
  data.Matched_filter.clearTrigger();	// clear the trigger indicator
  
  
  // build the lowpas FIR filter (remove this later)
  data.filter.setCoefficients(fir_coeff, false);


  // Setup finished.
  
  
    // If realtime output, set our callback function and start the dac.
  try {
	std::cout << "\n  startSteam\n";  
    adac.startStream();
  }
  catch ( RtAudioError &error ) {
    error.printMessage();
    goto cleanup;
  }

  
  while ( !done ) {
	// periodically send the Marco signal, start timing and wait for the Polo signal
    
	// store the starting time
	data.Matched_filter.clearTrigger();				// clear trigger time in the matched filter
	clock_gettime(CLOCK_REALTIME, &data.start_time); // get reference time for marco send		
	data.triggered = true;		// tell the realtime audio routine to play marco sound   
	
	
	// loop to see if the matched filter heard the polo sound (or if we timed out > 1sec)
	timeout = 0;
	while ((timeout < 100) &  (data.Matched_filter.getTriggerTime().tv_nsec == 0)) {
		Stk::sleep( 10 );			// wait 10msec
		timeout++;
	}
	
    if (data.Matched_filter.getTriggerTime().tv_nsec > 0){    // we heard the polo sound from the slave
		trigger_time = delta_time(data.start_time, data.Matched_filter.getTriggerTime());
		std::cout << trigger_time << " second delay \n";
	}
	else {
		std::cout << " timed out :( \n";
	}
	
	Stk::sleep( 500 );						// wait before sending the next Marco sound
	

  }

  // Shut down the output stream.
  try {
    adac.closeStream();
  }
  catch ( RtAudioError& error ) {
    error.printMessage();
  }

 cleanup:

	std::cout << "\nMarco Station finished ... goodbye.\n\n";
  return 0;
}
