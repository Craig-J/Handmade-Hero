#if !defined(WIN32_H)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// File:		|	win32_platform																		//
// Author:		|	Craig Jeffrey (craigjeffrey3@gmail.com)												//
// Date:		|	August 2015																			//
// Description:	|	Windows platform layer.																//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#include "abstract.h"
#include <windows.h>

namespace Win32
{
	struct AudioInfo
	{
		int samples_per_second;
		uint32 running_sample_index;
		int bytes_per_sample;
		int buffer_size;
		int latency_sample_count;
	};

	struct BitmapBuffer : public Abstract::BitmapBuffer
	{
		// NOTE(Craig): Win32 pixel layout.
		// 32-bit
		// Memory:		BB GG RR xx
		// Register:	xx RR GG BB
		BITMAPINFO info;
	};

	struct ClientDimensions
	{
		int width;
		int height;
	};

	struct Application
	{
		bool32 running;
		BitmapBuffer main_bitmap_buffer;
		LPDIRECTSOUNDBUFFER main_audio_buffer;
		int64 performance_counter_frequency;
	};
}

#define WIN32_H
#endif