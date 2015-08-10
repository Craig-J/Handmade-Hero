#if !defined(ABSTRACT_H)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// File:		|	abstract																			//
// Author:		|	Craig Jeffrey (craigjeffrey3@gmail.com)												//
// Date:		|	August 2015																			//
// Description:	|	Platform-independent abstraction layer.												//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#include "common.h"

namespace Abstract
{

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

	struct AppMemory
	{
		bool32 is_initialized;
		uint64 permanent_storage_size;
		void* permanent_storage; // NOTE(Craig): Required to be zeroed at initialization (VirtualAlloc does this automatically).
		uint64 transient_storage_size;
		void* transient_storage; // NOTE(Craig): Required to be zeroed at initialization (VirtualAlloc does this automatically).
	};

	void AppUpdateAndRender(AppMemory* memory, Input* input, BitmapBuffer* bitmap_buffer, AudioBuffer* audio_buffer);

	struct GameState
	{
		int tone_frequency;
		Colour::RGB gradient_offset;
	};

}
#define PLATFORM_H
#endif