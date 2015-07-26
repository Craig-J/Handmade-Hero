#include "platform.h"

void RenderWeirdGradientToBuffer(BitmapBuffer* _buffer, int _blue_offset, int _green_offset, int _red_offset)
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

void ClearBuffer(BitmapBuffer _buffer, uint8 _red, uint8 _green, uint8 _blue)
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

void AppRender(BitmapBuffer* _buffer)
{
	RenderWeirdGradientToBuffer(_buffer, 0, 0, 0);
}