#include "platform.h"

static_internal void RenderWeirdGradientToBuffer(BitmapBuffer* _buffer, int _blue_offset, int _green_offset, int _red_offset)
{
	uint8* row = (uint8*)_buffer->memory;
	for (int y = 0; y < _buffer->height; ++y)
	{
		uint32* pixel = (uint32*)row;
		for (int x = 0; x < _buffer->width; ++x)
		{
			uint8 blue = (x + _blue_offset);
			uint8 green = (y + _green_offset);
			uint8 red = _red_offset;

			*pixel++ = ((red << 16) | (green << 8) | blue);
		}

		row += _buffer->pitch;
	}
}

static_internal void ClearBuffer(BitmapBuffer _buffer, uint8 _red, uint8 _green, uint8 _blue)
{
	uint8* row = (uint8*)_buffer.memory;
	for (int y = 0; y < _buffer.height; ++y)
	{
		uint32* pixel = (uint32*)row;
		for (int x = 0; x < _buffer.width; ++x)
		{
			*pixel++ = ((_red << 16) | (_green << 8) | _blue);
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

static_internal void AppUpdateAndRender(BitmapBuffer* _bitmap_buffer, AudioBuffer* _audio_buffer)
{
	RenderAudioOutput(_audio_buffer);
	RenderWeirdGradientToBuffer(_bitmap_buffer, 0, 0, 0);
}