#include "abstract.h"
namespace Abstract
{

	static_internal void RenderWeirdGradientToBuffer(BitmapBuffer* _buffer, Colour::RGB _offset)
	{
		uint8* row = (uint8*)_buffer->memory;
		for (int y = 0; y < _buffer->height; ++y)
		{
			uint32* pixel = (uint32*)row;
			for (int x = 0; x < _buffer->width; ++x)
			{
				uint8 blue = (x + _offset.blue);
				uint8 green = (y + _offset.green);
				uint8 red = _offset.red;

				*pixel++ = ((red << 16) | (green << 8) | blue);
			}

			row += _buffer->pitch;
		}
	}

	static_internal void ClearBuffer(BitmapBuffer _buffer, Colour::RGB _clear_colour = Colour::Black)
	{
		uint8* row = (uint8*)_buffer.memory;
		for (int y = 0; y < _buffer.height; ++y)
		{
			uint32* pixel = (uint32*)row;
			for (int x = 0; x < _buffer.width; ++x)
			{
				*pixel++ = ((_clear_colour.red << 16) | (_clear_colour.green << 8) | _clear_colour.blue);
			}

			row += _buffer.pitch;
		}
	}

	static_internal void RenderAudioOutput(AudioBuffer* _audio_buffer)
	{
		static_local_persistant real32 t_sine;
		int16 tone_volume = 3000;
		int tone_frequency = 256;
		int wave_period = _audio_buffer->samples_per_second / tone_frequency;

		int16* sample_out = _audio_buffer->samples;
		for (int sample_index = 0; sample_index < _audio_buffer->sample_count; ++sample_index)
		{
			real32 sine_value = sin(t_sine);
			int16 sample_value = (int16)(sine_value * tone_volume);
			*sample_out++ = sample_value;
			*sample_out++ = sample_value;

			t_sine += (2.0f * Pi32) / (real32)wave_period;
		}
	}

	void UpdateAndRender(Memory* _memory, Input* _input, BitmapBuffer* _bitmap_buffer, AudioBuffer* _audio_buffer)
	{
		Assert(sizeof(GameState) <= _memory->permanent_storage_size);

		GameState* game_state = (GameState*)_memory->permanent_storage;
		if (!_memory->is_initialized)
		{
			// Note(Craig): Everything is default zero initialized. See Header.
			
			char* filename = __FILE__;

			FileBuffer file_buffer = ReadEntireFile(filename);
			if (file_buffer.memory)
			{
				WriteEntireFile("test.out", file_buffer.size, file_buffer.memory);
				FreeFileMemory(file_buffer.memory);
			}

			game_state->tone_frequency = 256;

			_memory->is_initialized = true;
		}

		Controller* controller_0 = &_input->controllers[0];
		if (controller_0->is_analog)
		{
			// NOTE(Craig): Analog-only controls here.
		}
		else
		{
			// NOTE(Craig): Digital-only controls here.
		}

		if (controller_0->down.ended_down) 
		{
			game_state->gradient_offset.green += 1;
		}

		RenderAudioOutput(_audio_buffer);
		RenderWeirdGradientToBuffer(_bitmap_buffer, game_state->gradient_offset);
	}

}