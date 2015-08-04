#if !defined(PLATFORM_H)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// File:		|	platform.h																			//
// Author:		|	Craig Jeffrey (craigjeffrey3@gmail.com)												//
// Date:		|	25/07/2015																			//
// Description:	|	Header for platform-independent layer.												//
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

struct ButtonState
{
	// NOTE(Craig): Using transition count to indicate any change instead of half_transition_count like Handmade_Hero.
	unsigned int transition_count;
	bool32 ended_down;
};
typedef ButtonState KeyState; // Keys and buttons are functionally identical.

struct Controller
{
	bool32 is_analog;

	real32 start_x;
	real32 start_y;

	real32 min_x;
	real32 min_y;

	real32 max_x;
	real32 max_y;

	real32 end_x;
	real32 end_y;

	union
	{
		ButtonState buttons[6];
		struct
		{
			ButtonState up;
			ButtonState down;
			ButtonState left;
			ButtonState right;
			ButtonState left_shoulder;
			ButtonState right_shoulder;
		};
	};
};

struct Input
{
	Controller controllers[4];
};

void AppUpdateAndRender(Input* input, BitmapBuffer* bitmap_buffer, AudioBuffer* audio_buffer);


#define PLATFORM_H
#endif