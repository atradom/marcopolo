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
LIBRARY = -lpthread -lasound -lm

REALTIME = yes
ifeq ($(REALTIME),yes)
  PROGRAMS += marco_generator
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

sineosc: sineosc.cpp Stk.o FileRead.o FileWvIn.o FileLoop.o FileWrite.o FileWvOut.o
	$(CC) $(LDFLAGS) $(CFLAGS) $(DEFS) -o sineosc sineosc.cpp $(OBJECT_PATH)/Stk.o $(OBJECT_PATH)/FileWrite.o $(OBJECT_PATH)/FileRead.o $(OBJECT_PATH)/FileWvIn.o $(OBJECT_PATH)/FileWvOut.o $(OBJECT_PATH)/FileLoop.o $(LIBRARY)


resonance: resonance.cpp Stk.o FileWrite.o FileWvOut.o BiQuad.o Noise.o
	$(CC) $(LDFLAGS) $(CFLAGS) $(DEFS) -o resonance resonance.cpp $(OBJECT_PATH)/Stk.o $(OBJECT_PATH)/FileWrite.o $(OBJECT_PATH)/BiQuad.o $(OBJECT_PATH)/Noise.o $(OBJECT_PATH)/FileWvOut.o $(LIBRARY)


