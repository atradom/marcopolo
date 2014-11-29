/**************  Effects Program  *********************/
// atr 11/26/14 - tried to add an FIR filter

#include "Skini.h"
#include "SKINImsg.h"
#include "Fir.h"	//atr
#include "Messager.h"
#include "RtAudio.h"

#include <signal.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <map>
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



/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 44100 Hz

* 0 Hz - 400 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 0.9072360190440464 dB

* 1000 Hz - 22050 Hz
  gain = 0
  desired attenuation = -20 dB
  actual attenuation = -33.103519146838195 dB

*/


  #define FILTER_TAP_NUM 103
  
 

StkFloat fir[FILTER_TAP_NUM] = {
  -0.012197920805787278,
  -0.002336589140620781,
  -0.0025194598432212827,
  -0.002683416037815436,
  -0.002823429184128793,
  -0.00293418668031811,
  -0.0030104361147199605,
  -0.0030496282697988304,
  -0.0030464438222232593,
  -0.0029949163618251218,
  -0.0028940985241409376,
  -0.0027368585998133565,
  -0.002521119850728108,
  -0.0022437068528243427,
  -0.00190148468332338,
  -0.0014937632705069163,
  -0.001015820478360003,
  -0.000467246012237626,
  0.00015210956444079113,
  0.0008435515745727611,
  0.0016051013437455518,
  0.0024351944901084676,
  0.0033356412950012212,
  0.004286091976426882,
  0.005330605145711128,
  0.006401235531185365,
  0.00752717076180959,
  0.008710198231310577,
  0.009937867692188263,
  0.011197413901675534,
  0.01247898994988451,
  0.013779349043195914,
  0.015094537309054343,
  0.016412012321639538,
  0.01772865618927969,
  0.019031408043286197,
  0.020311723116909923,
  0.021560218864397934,
  0.02276716788787066,
  0.02392560968568203,
  0.025025794409314683,
  0.02605959753521299,
  0.027021192584589307,
  0.027902713776822696,
  0.02869805780399121,
  0.02940164493135505,
  0.030003523736723,
  0.030507470101810413,
  0.030895798092277625,
  0.031177528730376925,
  0.03135087856049692,
  0.03140933165628752,
  0.03135087856049692,
  0.031177528730376925,
  0.030895798092277625,
  0.030507470101810413,
  0.030003523736723,
  0.02940164493135505,
  0.028698057803991213,
  0.027902713776822696,
  0.027021192584589307,
  0.02605959753521299,
  0.025025794409314683,
  0.02392560968568203,
  0.02276716788787066,
  0.021560218864397934,
  0.020311723116909923,
  0.019031408043286197,
  0.01772865618927969,
  0.016412012321639538,
  0.015094537309054343,
  0.013779349043195914,
  0.01247898994988451,
  0.011197413901675534,
  0.009937867692188263,
  0.008710198231310577,
  0.00752717076180959,
  0.006401235531185365,
  0.005330605145711128,
  0.004286091976426882,
  0.0033356412950012212,
  0.0024351944901084676,
  0.0016051013437455518,
  0.0008435515745727611,
  0.00015210956444079113,
  -0.000467246012237626,
  -0.001015820478360003,
  -0.0014937632705069163,
  -0.00190148468332338,
  -0.0022437068528243427,
  -0.002521119850728108,
  -0.0027368585998133565,
  -0.0028940985241409376,
  -0.0029949163618251196,
  -0.0030464438222232593,
  -0.0030496282697988304,
  -0.0030104361147199605,
  -0.00293418668031811,
  -0.002823429184128793,
  -0.002683416037815436,
  -0.0025194598432212827,
  -0.002336589140620781,
  -0.012197920805787278
};

 std::vector<StkFloat> fir_coeff(fir, fir + sizeof(fir) / sizeof(StkFloat) );



bool done;
static void finish(int ignore){ done = true; }

// The TickData structure holds all the class instances and data that
// are shared by the various processing functions.
struct TickData {
  unsigned int effectId;	// current effect ID
  Fir		filter;			// id=8  *atr
  Messager messager;		// for reading and parsing control messages
  Skini::Message message;	// control message
  StkFloat lastSample;
  StkFloat t60;				// T60 decay time (for reverb effects)
  int counter;
  bool settling;			// true if in settling state
  bool haveMessage;			// if a control message is available


  // Default constructor.
  TickData()
    : effectId(0), t60(1.0), counter(0),
      settling( false ), haveMessage( false ) {
	filter.setCoefficients(fir_coeff, false);	
	}
};

#define DELTA_CONTROL_TICKS 64 // default sample frames between control input checks




// The processMessage() function encapsulates the handling of control
// messages.  It can be easily relocated within a program structure
// depending on the desired scheduling scheme.
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
      std::cout << "effect type change\n";
      int type = data->message.intValues[1];
      data->effectId = (unsigned int) type;
      break;
    }

    case 22: // effect parameter change 1
      std::cout << "effect param 1 change\n";
//     data->echo.setDelay( (unsigned long) (temp * Stk::sampleRate() * 0.95) );
//     data->lshifter.setShift( 1.4 * temp + 0.3 );
      break;

    case 23: // effect parameter change 2
      std::cout << "effect param 2 change\n";
//      data->chorus.setModDepth( temp * 0.2 );
      break;

    case 44: // effect mix
      std::cout << "effect mix change\n";
//     data->echo.setEffectMix( temp );
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
 // Effect *effect;
  int i, counter, nTicks = (int) nBufferFrames;


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
      if (data->effectId==8) {    // atr - filter
		sample = data->filter.tick(*iSamples++);
		*oSamples++ = sample; // two channels interleaved
        *oSamples++ = sample;
      }
      else { 
        *oSamples++ = 0;
        *oSamples++ = 0;
      }
      nTicks--;
    }
    if ( nTicks == 0 ) break;

    // Process control messages.
    if ( data->haveMessage ) processMessage( data );
  }

  return 0;
}

int main( int argc, char *argv[] )
{
  TickData data;
  RtAudio adac;
  int i;
  
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
  unsigned int bufferFrames = RT_BUFFER_SIZE;
  
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

  // If realtime output, set our callback function and start the dac.
  try {
	std::cout << "\n  startSteam\n";  
    adac.startStream();
  }
  catch ( RtAudioError &error ) {
    error.printMessage();
    goto cleanup;
  }

  // Setup finished.
  while ( !done ) {
    // Periodically check "done" status.
    Stk::sleep( 50 );
  }

  // Shut down the output stream.
  try {
    adac.closeStream();
  }
  catch ( RtAudioError& error ) {
    error.printMessage();
  }

 cleanup:

	std::cout << "\neffects finished ... goodbye.\n\n";
  return 0;
}
