#if !defined(WIN32_H)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// File:		|	win32_platform																		//
// Author:		|	Craig Jeffrey (craigjeffrey3@gmail.com)												//
// Date:		|	August 2015																			//
// Description:	|	Windows platform layer.																//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#include "common.h"
#include "abstract.h"
#include <windows.h>

namespace Win32
{

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

	struct AudioInfo
	{
		int samples_per_second;
		uint32 running_sample_index;
		int bytes_per_sample;
		int secondary_buffer_size;
		int latency_sample_count;
	};

}

#define WIN32_H
#endif