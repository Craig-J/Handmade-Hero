#if !defined(PLATFORM_H)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// File:		|	platform.h																			//
// Author:		|	Craig Jeffrey (craigjeffrey3@gmail.com)												//
// Date:		|	25/07/2015																			//
// Description:	|	Platform-independent adaption.														//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#include "type_definitions.h"

//~~~~~~ PLATFORM TO APP SERVICES ~~~~~~\\


//~~~~~~ APP TO PLATFORM SERVICES ~~~~~~\\

struct BitmapBuffer
{
	void* memory;
	int width;
	int height;
	int pitch;
};

struct AudioBuffer
{
	int samples_per_second;
	int sample_count;
	int16* samples;
};

static_internal void AppUpdateAndRender(BitmapBuffer* bitmap_buffer, AudioBuffer* audio_buffer);


#define PLATFORM_H
#endif