### marco polo programs using stk toolkit
### atr 10/31/2014

PROGRAMS = filter-demo fir-demo sineosc resonance
RM = /bin/rm
SRC_PATH = ../../src
OBJECT_PATH = Release
vpath %.o $(OBJECT_PATH)

INCLUDE = 
ifeq ($(strip $(INCLUDE)), )
	INCLUDE = ../../include
endif
vpath %.h $(INCLUDE)

CC       = g++
DEFS     =   -DHAVE_GETTIMEOFDAY -D__LINUX_ALSA__
DEFS    += -D__LITTLE_ENDIAN__
CFLAGS   = -O3 -Wall
CFLAGS  += -I$(INCLUDE) -I$(INCLUDE)/../src/include
LIBRARY = -lpthread -lasound -lm -lrt

REALTIME = yes
ifeq ($(REALTIME),yes)
  PROGRAMS += marco_generator marco_detector marco_tester rtsine crtsine 
endif


%.o : $(SRC_PATH)/%.cpp $(OBJECT_PATH)/.placeholder
	$(CC) $(CFLAGS) $(DEFS) -c $(<) -o $(OBJECT_PATH)/$@

%.o : ../../src/include/%.cpp $(OBJECT_PATH)/.placeholder
	$(CC) $(CFLAGS) $(DEFS) -c $(<) -o $(OBJECT_PATH)/$@

all : $(PROGRAMS)

$(OBJECT_PATH)/.placeholder:
	mkdir -vp $(OBJECT_PATH)
	touch $(OBJECT_PATH)/.placeholder

$(OBJECTS) : Stk.h

clean : 
	$(RM) -f $(OBJECT_PATH)/*.o
	$(RM) -f $(PROGRAMS) *.exe
	$(RM) -fR *~ *.dSYM

distclean: clean
	$(RM) Makefile

strip : 
	strip $(PROGRAMS)

filter-demo: filter-demo.cpp Stk.o Iir.o
	$(CC) $(LDFLAGS) $(CFLAGS) $(DEFS) -o filter-demo filter-demo.cpp $(OBJECT_PATH)/*.o $(LIBRARY)
	
fir-demo: fir-demo.cpp Stk.o Fir.o FileWrite.o FileWvOut.o
	$(CC) $(LDFLAGS) $(CFLAGS) $(DEFS) -o fir-demo fir-demo.cpp $(OBJECT_PATH)/Stk.o $(OBJECT_PATH)/Fir.o $(OBJECT_PATH)/FileWrite.o $(OBJECT_PATH)/FileWvOut.o $(LIBRARY)

marco_generator: marco_generator.cpp Stk.o Fir.o FileRead.o FileWrite.o FileWvIn.o FileWvOut.o RtAudio.o
	$(CC) $(LDFLAGS) $(CFLAGS) $(DEFS) -o marco_generator marco_generator.cpp $(OBJECT_PATH)/Stk.o $(OBJECT_PATH)/Fir.o $(OBJECT_PATH)/FileWrite.o $(OBJECT_PATH)/FileRead.o $(OBJECT_PATH)/FileWvIn.o $(OBJECT_PATH)/FileWvOut.o $(OBJECT_PATH)/RtAudio.o $(LIBRARY)

marco_detector: marco_detector.cpp Stk.o Mutex.o Thread.o Skini.o Socket.o TcpServer.o Fir.o FileRead.o FileWrite.o FileWvIn.o FileWvOut.o RtMidi.o RtAudio.o Messager.o MFilt.o
	$(CC) $(LDFLAGS) $(CFLAGS) $(DEFS) -o marco_detector marco_detector.cpp $(OBJECT_PATH)/Stk.o $(OBJECT_PATH)/Mutex.o $(OBJECT_PATH)/Fir.o $(OBJECT_PATH)/FileWrite.o $(OBJECT_PATH)/FileRead.o $(OBJECT_PATH)/FileWvIn.o $(OBJECT_PATH)/FileWvOut.o $(OBJECT_PATH)/Skini.o  $(OBJECT_PATH)/Thread.o $(OBJECT_PATH)/Socket.o $(OBJECT_PATH)/RtMidi.o $(OBJECT_PATH)/TcpServer.o $(OBJECT_PATH)/Messager.o $(OBJECT_PATH)/RtAudio.o $(OBJECT_PATH)/MFilt.o $(LIBRARY)

marco_tester: marco_tester.cpp Stk.o Mutex.o Thread.o Skini.o Socket.o TcpServer.o Fir.o FileRead.o FileWrite.o FileWvIn.o FileWvOut.o RtMidi.o RtAudio.o Messager.o
	$(CC) $(LDFLAGS) $(CFLAGS) $(DEFS) -o marco_tester marco_tester.cpp $(OBJECT_PATH)/Stk.o $(OBJECT_PATH)/Mutex.o $(OBJECT_PATH)/Fir.o $(OBJECT_PATH)/FileWrite.o $(OBJECT_PATH)/FileRead.o $(OBJECT_PATH)/FileWvIn.o $(OBJECT_PATH)/FileWvOut.o $(OBJECT_PATH)/Skini.o  $(OBJECT_PATH)/Thread.o $(OBJECT_PATH)/Socket.o $(OBJECT_PATH)/RtMidi.o $(OBJECT_PATH)/TcpServer.o $(OBJECT_PATH)/Messager.o $(OBJECT_PATH)/RtAudio.o $(LIBRARY)




sineosc: sineosc.cpp Stk.o FileRead.o FileWvIn.o FileLoop.o FileWrite.o FileWvOut.o
	$(CC) $(LDFLAGS) $(CFLAGS) $(DEFS) -o sineosc sineosc.cpp $(OBJECT_PATH)/Stk.o $(OBJECT_PATH)/FileWrite.o $(OBJECT_PATH)/FileRead.o $(OBJECT_PATH)/FileWvIn.o $(OBJECT_PATH)/FileWvOut.o $(OBJECT_PATH)/FileLoop.o $(LIBRARY)


resonance: resonance.cpp Stk.o FileWrite.o FileWvOut.o BiQuad.o Noise.o
	$(CC) $(LDFLAGS) $(CFLAGS) $(DEFS) -o resonance resonance.cpp $(OBJECT_PATH)/Stk.o $(OBJECT_PATH)/FileWrite.o $(OBJECT_PATH)/BiQuad.o $(OBJECT_PATH)/Noise.o $(OBJECT_PATH)/FileWvOut.o $(LIBRARY)

rtsine: rtsine.cpp Stk.o SineWave.o RtWvOut.o RtAudio.o Mutex.o
	$(CC) $(LDFLAGS) $(CFLAGS) $(DEFS) -o rtsine rtsine.cpp $(OBJECT_PATH)/Stk.o $(OBJECT_PATH)/Mutex.o $(OBJECT_PATH)/SineWave.o $(OBJECT_PATH)/RtWvOut.o $(OBJECT_PATH)/RtAudio.o $(LIBRARY)


crtsine: crtsine.cpp Stk.o SineWave.o RtAudio.o
	$(CC) $(LDFLAGS) $(CFLAGS) $(DEFS) -o crtsine crtsine.cpp $(OBJECT_PATH)/Stk.o $(OBJECT_PATH)/Mutex.o $(OBJECT_PATH)/SineWave.o $(OBJECT_PATH)/RtAudio.o $(LIBRARY)

