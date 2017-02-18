#if !defined(WIN32_H)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// File:		|	win32_platform																		//
// Author:		|	Craig Jeffrey (craigjeffrey3@gmail.com)												//
// Date:		|	February 2017																		//
// Description:	|	Windows platform layer.																//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

namespace Win32
{
	struct AudioInfo
	{
		int SamplesPerSecond;
		uint32 RunningSampleIndex;
		int BytesPerSample;
		int BufferSize;
		int LatencySampleCount;
	};

	struct BitmapBuffer : ::BitmapBuffer
	{
		// NOTE(Craig): Win32 pixel layout.
		// 32-bit
		// Memory:		BB GG RR xx
		// Register:	xx RR GG BB
		BITMAPINFO Info;
	};

	struct ClientDimensions
	{
		int Width;
		int Height;
	};

	struct Application
	{
		bool32 IsRunning;
		BitmapBuffer MainBitmapBuffer;
		LPDIRECTSOUNDBUFFER MainAudioBuffer;
	};
}

#define WIN32_H
#endif