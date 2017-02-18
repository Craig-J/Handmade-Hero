#include "handmade.h"

static_internal void RenderWeirdGradientToBuffer(BitmapBuffer* buffer, Colour::RGB offset)
{
	uint8* row = (uint8*)buffer->Memory;
	for (int y = 0; y < buffer->Height; ++y)
	{
		uint32* pixel = (uint32*)row;
		for (int x = 0; x < buffer->Width; ++x)
		{
			uint8 blue = (uint8)(x + offset.Blue);
			uint8 green = (uint8)(y + offset.Green);
			uint8 red = offset.Red;

			*pixel++ = ((red << 16) | (green << 8) | blue);
		}

		row += buffer->Pitch;
	}
}

static_internal void ClearBitmapBuffer(BitmapBuffer buffer, Colour::RGB clearColour = Colour::Black)
{
	uint8* row = (uint8*)buffer.Memory;
	for (int y = 0; y < buffer.Height; ++y)
	{
		uint32* pixel = (uint32*)row;
		for (int x = 0; x < buffer.Width; ++x)
		{
			*pixel++ = ((clearColour.Red << 16) | (clearColour.Green << 8) | clearColour.Blue);
		}

		row += buffer.Pitch;
	}
}

static_internal void RenderAudioOutput(AudioBuffer* audioBuffer, int toneFrequency)
{
	static_local_persistant real32 tSine;
	int16 toneVolume = 3000;
	int wavePeriod = audioBuffer->SamplesPerSecond / toneFrequency;

	int16* sampleOut = audioBuffer->Samples;
	for (int sampleIndex = 0; sampleIndex < audioBuffer->SampleCount; ++sampleIndex)
	{
		real32 sineValue = sin(tSine);
		int16 sample_value = (int16)(sineValue * toneVolume);
		*sampleOut++ = sample_value;
		*sampleOut++ = sample_value;

		tSine += (2.0f * Pi32) / (real32)wavePeriod;
	}
}

static_internal void UpdateAndRender(Memory* memory, Input* input, BitmapBuffer* bitmapBuffer, AudioBuffer* audioBuffer)
{
	Assert((&input->Controllers[0].Terminator - &input->Controllers[0].Buttons[0]) ==
		   (ArrayCount(input->Controllers[0].Buttons)));
	Assert(sizeof(GameState) <= memory->PermanentStorageSize);

	GameState* gameState = (GameState*)memory->PermanentStorage;
	if (!memory->IsInitialized)
	{
		// Note(Craig): Everything is default zero initialized. See Header.
			
		char* filename = __FILE__;

		Debug::FileBuffer fileBuffer = Debug::ReadEntireFile(filename);
		if (fileBuffer.Memory)
		{
			Debug::WriteEntireFile("test.out", fileBuffer.Size, fileBuffer.Memory);
			Debug::FreeFileMemory(fileBuffer.Memory);
		}

		gameState->ToneFrequency = 256;

		memory->IsInitialized = true;
	}

	for (uint8 controllerIndex = 0; controllerIndex < ArrayCount(input->Controllers); ++controllerIndex)
	{
		Controller* controller = GetController(input, controllerIndex);
		if (controller->IsAnalog)
		{
			// NOTE(Craig): Analog-only controls here.
			gameState->GradientOffset.Blue += (uint8)(4.0f*controller->StickAverageX);
			gameState->ToneFrequency = 256 + (int)(128.0f*controller->StickAverageY);
		}
		else
		{
			// NOTE(Craig): Digital-only controls here.
			if (controller->StickLeft.EndedDown)
			{
				gameState->GradientOffset.Blue -= 1;
			}
			if (controller->StickRight.EndedDown)
			{
				gameState->GradientOffset.Blue += 1;
			}
			if (controller->StickUp.EndedDown)
			{
				gameState->ToneFrequency = 512;
			}
			if (controller->StickDown.EndedDown)
			{
				gameState->ToneFrequency = 128;
			}
		}

		if (controller->ActionDown.EndedDown)
		{
			gameState->GradientOffset.Green += 1;
		}
	}

	RenderAudioOutput(audioBuffer, gameState->ToneFrequency);
	RenderWeirdGradientToBuffer(bitmapBuffer, gameState->GradientOffset);
}