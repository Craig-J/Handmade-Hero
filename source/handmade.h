#if !defined(HANDMADE_H)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// File:		|	handmade.h																			//
// Author:		|	Craig Jeffrey (craigjeffrey3@gmail.com)												//
// Date:		|	February 2017																		//
// Description:	|	Platform-agnostic layer.															//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#include "common.h"

#if DEV_BUILD
namespace Debug
{
	struct FileBuffer
	{
		uint32 Size;
		void* Memory;
	};

	// NOTE(Craig): Implemented in platform layer.
	FileBuffer ReadEntireFile(char* filename);
	void FreeFileMemory(void* memory);
	bool32 WriteEntireFile(char* filename, uint32 memorySize, void* memory);
}
#endif

struct AudioBuffer
{
	int SamplesPerSecond;
	int SampleCount;
	int16* Samples;
};

struct BitmapBuffer
{
	void* Memory;
	int Width;
	int Height;
	int Pitch;
};

struct ButtonState
{
	// NOTE(Craig): Using transition count to indicate any change instead of half_transition_count like Handmade_Hero.
	unsigned TransitionCount;
	bool32 EndedDown;
};
typedef ButtonState KeyState; // Keys and buttons are functionally identical.

struct Controller
{
	bool32 IsConnected;
	bool32 IsAnalog;

	real32 StickAverageX;
	real32 StickAverageY;

	union
	{
		ButtonState Buttons[12];
		struct
		{
			ButtonState StickUp;
			ButtonState StickDown;
			ButtonState StickLeft;
			ButtonState StickRight;

			/*ButtonState PadUp;
			ButtonState PadDown;
			ButtonState PadLeft;
			ButtonState PadRight;*/

			ButtonState ActionUp;
			ButtonState ActionDown;
			ButtonState ActionLeft;
			ButtonState ActionRight;

			ButtonState LeftShoulder;
			ButtonState RightShoulder;

			ButtonState Start;
			ButtonState Back;

			// NOTE(Craig): Add buttons above here
			ButtonState Terminator;
		};
	};
};

struct Input
{
	Controller Controllers[5];
};

inline Controller* GetController(Input* input, uint8 controllerIndex)
{
	Assert(controllerIndex < ArrayCount(input->Controllers));

	return &input->Controllers[controllerIndex];
}

struct Memory
{
	bool32 IsInitialized;

	uint64 PermanentStorageSize;
	void* PermanentStorage; // NOTE(Craig): Required to be zeroed at initialization (VirtualAlloc does this automatically).

	uint64 TransientStorageSize;
	void* TransientStorage; // NOTE(Craig): Required to be zeroed at initialization (VirtualAlloc does this automatically).
};

struct GameState
{
	int ToneFrequency;
	Colour::RGB GradientOffset;
};

static_internal void UpdateAndRender(Memory* memory, Input* input, BitmapBuffer* bitmapBuffer, AudioBuffer* audioBuffer);

#define HANDMADE_H
#endif