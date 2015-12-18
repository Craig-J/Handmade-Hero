#if !defined(ABSTRACT_H)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// File:		|	abstract																			//
// Author:		|	Craig Jeffrey (craigjeffrey3@gmail.com)												//
// Date:		|	August 2015																			//
// Description:	|	Platform-independent abstraction layer.												//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#include "common.h"
#include "build.h"

namespace Abstract
{
	struct AudioBuffer
	{
		int samples_per_second;
		int sample_count;
		int16* samples;
	};

	struct BitmapBuffer
	{
		void* memory;
		int width;
		int height;
		int pitch;
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
		bool32 is_connected;
		bool32 is_analog;

		real32 stick_average_x;
		real32 stick_average_y;

		union
		{
			ButtonState buttons[12];
			struct
			{
				ButtonState stick_up;
				ButtonState stick_down;
				ButtonState stick_left;
				ButtonState stick_right;

				/*ButtonState pad_up;
				ButtonState pad_down;
				ButtonState pad_left;
				ButtonState pad_right;*/

				ButtonState action_up;
				ButtonState action_down;
				ButtonState action_left;
				ButtonState action_right;

				ButtonState left_shoulder;
				ButtonState right_shoulder;

				ButtonState start;
				ButtonState back;
			};
		};
	};

	struct Input
	{
		Controller controllers[5];
	};
	inline Controller* GetController(Input* _input, uint8 _controller_index)
	{
		Assert(_controller_index < ArrayCount(_input->controllers));
		Controller* result = &_input->controllers[_controller_index];
		return result;
	}

	struct Memory
	{
		bool32 is_initialized;
		uint64 permanent_storage_size;
		void* permanent_storage; // NOTE(Craig): Required to be zeroed at initialization (VirtualAlloc does this automatically).
		uint64 transient_storage_size;
		void* transient_storage; // NOTE(Craig): Required to be zeroed at initialization (VirtualAlloc does this automatically).
	};

	struct GameState
	{
		int tone_frequency;
		Colour::RGB gradient_offset;
	};

	void UpdateAndRender(Memory* memory, Input* input, BitmapBuffer* bitmap_buffer, AudioBuffer* audio_buffer);
}
#define ABSTRACT_H
#endif