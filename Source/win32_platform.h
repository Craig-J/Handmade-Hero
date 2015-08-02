#if !defined(WIN32_H)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// File:			win32_platform.h																	//
// Author:			Craig Jeffrey (craigjeffrey3@gmail.com)												//
// Date:			27/07/2015																			//
// Description:		Header for the windows platform layer.												//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#include "type_definitions.h"
#include "platform.h"

#include <windows.h>

struct Win32BitmapBuffer
{
	// NOTE(Craig): Win32 pixel layout.
	// 32-bit
	// Memory:		BB GG RR xx
	// Register:	xx RR GG BB
	BITMAPINFO info;
	void* memory;
	int width;
	int height;
	int pitch;
};

struct Win32ClientDimensions
{
	int width;
	int height;
};

struct Win32AudioInfo
{
	int samples_per_second;
	uint32 running_sample_index;
	int bytes_per_sample;
	int secondary_buffer_size;
	real32 t_sine;
	int latency_sample_count;
};

#define WIN32_H
#endif