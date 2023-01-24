

#ifndef CSOUND

#define CSOUND

#if defined (_WIN32) || defined( _WIN64)
#include <windows.h>
#endif
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>

#include <string>
#include <iostream>
#include <AL/al.h>
#include <AL/alc.h>
using namespace std;

class csound {

public:

          ALuint source;                                                              //Is the name of source (where the sound come from)
          ALuint buffer;                                                           //Stores the sound data
          ALuint frequency;                                               //The Sample Rate of the WAVE file
          ALenum format;                                                            //The audio format (bits per sample, number of channels)
          FILE *fp;
          unsigned char* buf;

          int endWithError(char* msg, int error);
          int isPlaying(ALuint source);
          int init_sound(char *sfilename, ALCdevice *device, ALCcontext *context);
          int close_sound(void);
          int play(void);

} ;

#endif
