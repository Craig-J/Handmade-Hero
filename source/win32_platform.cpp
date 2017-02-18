/*
TODO(Craig): Platform TODO List

- Saved game locations
- Handle to own executable
- Asset loading path
- Threading
- Raw input
- Sleep/timebeginperiod
- Clipcursor (multimonitor support)
- Fullscreen support
- WM_SETCURSOR
- QueryCancelAutoplay
- WM_ACTIVATEAPP
- Blit speed improvement
- Hardware acceleration (OpenGL/D3D)
- GetKeyboardLayout
*/

#include "handmade.h"
#include "handmade.cpp"

#include <Windows.h>
#include <stdio.h>
#include <malloc.h>
#include <dsound.h>
#include <xinput.h>

#include "win32_platform.h"

// TODO(Craig): Change from global.
static_global Win32::Application _application;
static_global int64 _qpcFrequency;

// NOTE(Craig): XInputGetState macro/typedef/stub.
//		Description
//		First define a macro to create functions with correct signature.
//		Second create a type with this function signature using the macro.
//		Third make a default stub function using the macro.
//		Fourth create a function pointer that points to the stub function.
//		Fifth define the functions normal name to reference the function pointer instead.
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(XInputGetState_Type);
X_INPUT_GET_STATE(XInputGetState_Stub)
{
	return ERROR_DEVICE_NOT_CONNECTED;
}
static_global XInputGetState_Type* XInputGetState_FuncPtr = XInputGetState_Stub;
#define XInputGetState XInputGetState_FuncPtr

// NOTE(Craig): XInputSetState macro/typedef/stub.
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(XInputSetState_Type);
X_INPUT_SET_STATE(XInputSetState_Stub)
{
	return ERROR_DEVICE_NOT_CONNECTED;
}
static_global XInputSetState_Type* XInputSetState_FuncPtr = XInputSetState_Stub;
#define XInputSetState XInputSetState_FuncPtr

// NOTE(Craig): DirectSoundCreate macro/typedef.
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(DirectSoundCreate_Type);

namespace Win32
{
	// AUDIO
	static_internal void InitializeDirectSound(HWND window, int32 samplesPerSecond, int32 bufferSize)
	{
		HMODULE DSoundLibrary = LoadLibrary("dsound.dll");

		if (DSoundLibrary)
		{
			DirectSoundCreate_Type* DirectSoundCreate = (DirectSoundCreate_Type*)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

			LPDIRECTSOUND directSound;
			if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &directSound, 0)))
			{
				WAVEFORMATEX waveFormat = {};
				waveFormat.wFormatTag = WAVE_FORMAT_PCM;
				waveFormat.nChannels = 2;
				waveFormat.nSamplesPerSec = samplesPerSecond;
				waveFormat.wBitsPerSample = 16;
				waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
				waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
				waveFormat.cbSize = 0;

				if (SUCCEEDED(directSound->SetCooperativeLevel(window, DSSCL_PRIORITY)))
				{
					DSBUFFERDESC bufferDescription = {};
					bufferDescription.dwSize = sizeof(bufferDescription);
					bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

					// NOTE(Craig): Create a primary buffer.
					LPDIRECTSOUNDBUFFER primaryBuffer;
					if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0)))
					{
						if (SUCCEEDED(primaryBuffer->SetFormat(&waveFormat)))
						{

						}
						else
						{
							// TODO(Craig): Log format set failure.
						}
					}
					else
					{
						// TODO(Craig): Log primary buffer creation failure.
					}
				}
				else
				{
					// TODO(Craig): Log SetCooperativeLevel function failure.
				}

				DSBUFFERDESC bufferDescription = {};
				bufferDescription.dwSize = sizeof(bufferDescription);
				bufferDescription.dwBufferBytes = bufferSize;
				bufferDescription.lpwfxFormat = &waveFormat;

				if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &_application.MainAudioBuffer, 0)))
				{
					// TODO(Craig): Log buffer creation success.
				}
			}
			else
			{
				// TODO(Craig): Log that the function failed to load.
			}
		}
		else
		{
			// TODO(Craig): Log that directsound failed to load.
		}
	}

	static_internal void ClearAudioBuffer(AudioInfo* buffer)
	{
		VOID* region1;
		DWORD region1Size;
		VOID* region2;
		DWORD region2Size;
		if (SUCCEEDED(_application.MainAudioBuffer->Lock(0, buffer->BufferSize,
			&region1, &region1Size,
			&region2, &region2Size,
			0)))
		{
			uint8* sampleOut = (uint8*)region1;
			for (DWORD byteIndex = 0; byteIndex < region1Size; ++byteIndex)
			{
				*sampleOut++ = 0;
			}

			sampleOut = (uint8*)region2;
			for (DWORD byteIndex = 0; byteIndex < region2Size; ++byteIndex)
			{
				*sampleOut++ = 0;
			}

			_application.MainAudioBuffer->Unlock(region1, region1Size, region2, region2Size);
		}
	}

	static_internal void FillAudioBuffer(AudioInfo* audioInfo, DWORD byteToLock, DWORD bytesToWrite, AudioBuffer* buffer)
	{
		VOID* region1;
		DWORD region1Size;
		VOID* region2;
		DWORD region2Size;
		if (SUCCEEDED(_application.MainAudioBuffer->Lock(byteToLock, bytesToWrite,
			&region1, &region1Size,
			&region2, &region2Size,
			0)))
		{
			int16* sampleIn = buffer->Samples;

			DWORD region1SampleCount = region1Size / audioInfo->BytesPerSample;
			int16* sampleOut = (int16*)region1;
			for (DWORD sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex)
			{
				*sampleOut++ = *sampleIn++;
				*sampleOut++ = *sampleIn++;
				++audioInfo->RunningSampleIndex;
			}

			DWORD region2SampleCount = region2Size / audioInfo->BytesPerSample;
			sampleOut = (int16*)region2;
			for (DWORD sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex)
			{
				*sampleOut++ = *sampleIn++;
				*sampleOut++ = *sampleIn++;
				++audioInfo->RunningSampleIndex;
			}

			_application.MainAudioBuffer->Unlock(region1, region1Size, region2, region2Size);
		}
	}

	// INPUT
	static_internal void LoadXInput()
	{
		HMODULE XInputLibrary = LoadLibrary("xinput1_4.dll");
		if (!XInputLibrary)
		{
			XInputLibrary = LoadLibrary("xinput9_1_0.dll");
		}
		if (!XInputLibrary)
		{
			XInputLibrary = LoadLibrary("xinput1_3.dll");
		}
		if (XInputLibrary)
		{
			// TODO(Craig): Log xinput version.
			XInputGetState = (XInputGetState_Type*)GetProcAddress(XInputLibrary, "XInputGetState");
			if (!XInputGetState) { XInputGetState = XInputGetState_Stub; }
			XInputSetState = (XInputSetState_Type*)GetProcAddress(XInputLibrary, "XInputSetState");
			if (!XInputSetState) { XInputSetState = XInputSetState_Stub; }

			// TODO(Craig): Log if a function failed to load.
		}
		else
		{
			// TODO(Craig): Log that XInput failed to load.
		}
	}

	static_internal void ProcessXInputDigitalButton(WORD XInputButtonStates, DWORD buttonBit, ButtonState* oldState, ButtonState* newState)
	{
		newState->EndedDown = ((XInputButtonStates & buttonBit) == buttonBit);
		newState->TransitionCount = (oldState->EndedDown != newState->EndedDown) ? 1 : 0;
	}

	static_internal real32 ProcessXInputAnalogStick(SHORT value, SHORT deadZoneThreshold)
	{
		real32 result = 0;

		if (value < -deadZoneThreshold)
		{
			result = (real32)((value + deadZoneThreshold) / (32768.0f - deadZoneThreshold));
		}
		else if (value > deadZoneThreshold)
		{
			result = (real32)((value - deadZoneThreshold) / (32767.0f - deadZoneThreshold));
		}

		return result;
	}

	static_internal void ProcessKeyboardMessage(ButtonState* newState, bool32 isDown)
	{
		Assert(newState->EndedDown != isDown);

		newState->EndedDown = isDown;
		++newState->TransitionCount;
	}

	// GRAPHICS
	static_internal void ClearBitmapBuffer(BitmapBuffer buffer, Colour::RGB colour = Colour::Black)
	{
		uint8* row = (uint8*)buffer.Memory;
		for (int y = 0; y < buffer.Height; ++y)
		{
			uint32* pixel = (uint32*)row;
			for (int x = 0; x < buffer.Width; ++x)
			{
				*pixel++ = ((colour.Red << 16) | (colour.Green << 8) | colour.Blue);
			}

			row += buffer.Pitch;
		}
	}

	static_internal void ResizeBitmapBuffer(BitmapBuffer* buffer, int width, int height)
	{
		// TODO(Craig): Bulletproof this.
		// Don't pre-free, free after (free first if it fails).

		if (buffer->Memory)
		{
			VirtualFree(buffer->Memory, 0, MEM_RELEASE);
		}

		buffer->Width = width;
		buffer->Height = height;
		int bytesPerPixel = 4;

		// NOTE(Craig): When biHeight is negative, this is the clue to windows
		// to treat the bitmap as top-down, not bottom-up, meaning that the buffer
		// starts at the top left pixel not the bottom right.
		buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
		buffer->Info.bmiHeader.biWidth = width;
		buffer->Info.bmiHeader.biHeight = -height;
		buffer->Info.bmiHeader.biPlanes = 1;
		buffer->Info.bmiHeader.biBitCount = (WORD)(8 * bytesPerPixel);
		buffer->Info.bmiHeader.biCompression = BI_RGB;

		int bitmapMemorySize = (width * height) * bytesPerPixel;
		buffer->Memory = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		buffer->Pitch = buffer->Width * bytesPerPixel;

		// Clears buffer to black
		ClearBitmapBuffer(*buffer);
	}

	static_internal void DisplayBitmapToDevice(HDC deviceContext, BitmapBuffer* buffer, int clientWidth, int clientHeight)
	{
		StretchDIBits(deviceContext,
					  0, 0, clientWidth, clientHeight,
					  0, 0, clientWidth, clientHeight,		// No scaling
//					  0, 0, buffer->width, buffer->height,	// Scale buffer to client size
					  buffer->Memory,
					  &buffer->Info,
					  DIB_RGB_COLORS, SRCCOPY);
	}

	// WINDOW
	static_internal ClientDimensions GetClientDimensions(HWND window)
	{
		RECT clientRect;
		GetClientRect(window, &clientRect);
		ClientDimensions result;
		result.Width = clientRect.right - clientRect.left;
		result.Height = clientRect.bottom - clientRect.top;
		return result;
	}

	static_internal LRESULT CALLBACK MainWindowCallback(HWND window, UINT message, WPARAM wParameter, LPARAM lParameter)
	{
		LRESULT result = 0;

		switch (message)
		{
			case WM_SIZE:
			{
			} break;

			case WM_DESTROY:
			{
				// TODO(Craig): Handle as error - recreate window?
				_application.IsRunning = false;
			} break;

			case WM_CLOSE:
			{
				// TODO(Craig): Handle with message to user?
				_application.IsRunning = false;
			} break;

			case WM_ACTIVATEAPP:
			{
				OutputDebugString("WM_ACTIVATEAPP\n");
			} break;

			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			{
				Assert(!"Keyboard input came through a non-dispatch method.");
			} break;

			case WM_PAINT:
			{
				PAINTSTRUCT paint;
				HDC deviceContext = BeginPaint(window, &paint);
				ClientDimensions client = GetClientDimensions(window);
				DisplayBitmapToDevice(deviceContext, &_application.MainBitmapBuffer, client.Width, client.Height);
				EndPaint(window, &paint);
			} break;

			default:
			{
				result = DefWindowProc(window, message, wParameter, lParameter);
			} break;
		}

		return result;
	}

	static_internal void ProcessMessages(Controller* controller)
	{
		MSG message;

		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT)
			{
				_application.IsRunning = false;
			}

			switch (message.message)
			{
				case WM_KEYDOWN:
				case WM_KEYUP:
				case WM_SYSKEYDOWN:
				case WM_SYSKEYUP:
				{
					uint32 VKCode = (uint32)message.wParam;
					bool32 keyReleased = ((message.lParam & (1 << 30)) != 0);
					bool32 keyPressed = ((message.lParam & (1 << 31)) == 0);
					bool32 altDown = (message.lParam & (1 << 29));

					if (keyPressed == keyReleased)
					{
						// NOTE(Craig): Key is repeating in this block
					}
					else
					{
						if (VKCode == 'W')
						{
							Win32::ProcessKeyboardMessage(&controller->StickUp, keyPressed);
						}
						else if (VKCode == 'A')
						{
							Win32::ProcessKeyboardMessage(&controller->StickLeft, keyPressed);
						}
						else if (VKCode == 'S')
						{
							Win32::ProcessKeyboardMessage(&controller->StickDown, keyPressed);
						}
						else if (VKCode == 'D')
						{
							Win32::ProcessKeyboardMessage(&controller->StickRight, keyPressed);
						}
						else if (VKCode == 'Q')
						{
							Win32::ProcessKeyboardMessage(&controller->LeftShoulder, keyPressed);
						}
						else if (VKCode == 'E')
						{
							Win32::ProcessKeyboardMessage(&controller->RightShoulder, keyPressed);
						}
						else if (VKCode == 'R')
						{

						}
						else if (VKCode == 'T')
						{

						}
						else if (VKCode == 'F')
						{

						}
						else if (VKCode == 'G')
						{

						}
						else if (VKCode == VK_ESCAPE)
						{
							Win32::ProcessKeyboardMessage(&controller->Back, keyPressed);
						}
						else if (VKCode == VK_SPACE)
						{
							Win32::ProcessKeyboardMessage(&controller->Start, keyPressed);
						}
						else if (VKCode == VK_UP)
						{
							Win32::ProcessKeyboardMessage(&controller->ActionUp, keyPressed);
						}
						else if (VKCode == VK_DOWN)
						{
							Win32::ProcessKeyboardMessage(&controller->ActionDown, keyPressed);
						}
						else if (VKCode == VK_LEFT)
						{
							Win32::ProcessKeyboardMessage(&controller->ActionLeft, keyPressed);
						}
						else if (VKCode == VK_RIGHT)
						{
							Win32::ProcessKeyboardMessage(&controller->ActionRight, keyPressed);
						}
					}

					if ((altDown) && (VKCode == VK_F4))
					{
						_application.IsRunning = false;
					}
				} break;

				default:
				{
					TranslateMessage(&message);
					DispatchMessage(&message);
				} break;
			}
		}
	}

	// SYSTEM

	static_internal inline LARGE_INTEGER GetWallClock()
	{
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return counter;
	}

	static_internal inline real32 GetTimeDifference(LARGE_INTEGER start, LARGE_INTEGER end)
	{
		return((real32)(end.QuadPart - start.QuadPart) / (real32)_qpcFrequency);
	}
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int commandShow)
{
	LARGE_INTEGER temp_performance_counter_frequency;
	QueryPerformanceFrequency(&temp_performance_counter_frequency);
	_qpcFrequency = temp_performance_counter_frequency.QuadPart;

	//UINT desiredSchedulerMS = 1;
	//bool32 sleepIsGranular = (timeBeginPeriod(desiredSchedulerMS) == TIMERR_NOERROR);

	_application = {};
	Win32::ResizeBitmapBuffer(&_application.MainBitmapBuffer, 1920, 1080);
	Win32::LoadXInput();

	WNDCLASS windowClass = {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = Win32::MainWindowCallback;
	windowClass.hInstance = instance;
	// windowClass.hIcon;
	windowClass.lpszClassName = "HandmadeHeroWindowClass";

	int monitorRefreshRate = 60;
	int gameUpdateRate = monitorRefreshRate / 2;
	real32 targetFrameTime = 1.0f / gameUpdateRate;

	if (RegisterClass(&windowClass))
	{
		HWND window = CreateWindowEx(0,
									 windowClass.lpszClassName,
									 "Handmade Hero",
									 WS_OVERLAPPEDWINDOW | WS_VISIBLE,
									 CW_USEDEFAULT,
									 CW_USEDEFAULT,
									 CW_USEDEFAULT,
									 CW_USEDEFAULT,
									 0,
									 0,
									 instance,
									 0);

		if (window)
		{
			HDC deviceContext = GetDC(window);

			Win32::AudioInfo audioInfo = {};
			audioInfo.SamplesPerSecond = 48000;
			audioInfo.RunningSampleIndex = 0;
			audioInfo.BytesPerSample = sizeof(int16) * 2;
			audioInfo.BufferSize = audioInfo.SamplesPerSecond * audioInfo.BytesPerSample;
			audioInfo.LatencySampleCount = audioInfo.SamplesPerSecond / 15;

			Win32::InitializeDirectSound(window, audioInfo.SamplesPerSecond, audioInfo.BufferSize);
			Win32::ClearAudioBuffer(&audioInfo);
			_application.MainAudioBuffer->Play(0, 0, DSBPLAY_LOOPING);

			int16* samples = (int16*)VirtualAlloc(0, audioInfo.BufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

#if DEV_BUILD
			LPVOID baseAddress = (LPVOID)Terabytes(2);
#else
			LPVOID baseAddress = 0;
#endif

			Memory memory = {};
			memory.PermanentStorageSize = Megabytes(64);
			memory.TransientStorageSize = Gigabytes(2);
			//memory.transient_storage_size = Megabytes(963); 

			uint64 totalMemorySize = memory.PermanentStorageSize + memory.TransientStorageSize;
			memory.PermanentStorage = VirtualAlloc(baseAddress, (size_t)totalMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			memory.TransientStorage = ((uint8*)memory.PermanentStorage + memory.PermanentStorageSize);

			if (samples && memory.PermanentStorage && memory.TransientStorage)
			{
				Input input[2] = {};
				Input* newInput = &input[0];
				Input* oldInput = &input[1];

				LARGE_INTEGER previousCounter = Win32::GetWallClock();

				uint64 previousCycleCount = __rdtsc();

				_application.IsRunning = true;
				while (_application.IsRunning)
				{
					Controller* oldKeyboardController = GetController(oldInput, 0);
					Controller* newKeyboardController = GetController(newInput, 0);
					*newKeyboardController = {};
					newKeyboardController->IsConnected = true;
					for (uint8 buttonIndex = 0; buttonIndex < ArrayCount(newKeyboardController->Buttons); ++buttonIndex)
					{
						newKeyboardController->Buttons[buttonIndex].EndedDown = oldKeyboardController->Buttons[buttonIndex].EndedDown;
					}

					Win32::ProcessMessages(newKeyboardController);

					// NOTE(Craig): Keyboard + Number of controllers allowed.
					uint8 maxControllerCount = 1 + XUSER_MAX_COUNT;
					if (maxControllerCount > ArrayCount(newInput->Controllers))
					{
						maxControllerCount = ArrayCount(newInput->Controllers);
					}

					// NOTE(Craig): Index starting at 1, 0 is keyboard.
					for (uint8 controllerIndex = 1; controllerIndex < maxControllerCount; ++controllerIndex)
					{
						Controller* oldController = GetController(oldInput, controllerIndex);
						Controller* newController = GetController(newInput, controllerIndex);

						XINPUT_STATE controller_state;
						if (XInputGetState(controllerIndex, &controller_state) == ERROR_SUCCESS)
						{
							// NOTE(Craig): Controller available.
							newController->IsConnected = true;

							XINPUT_GAMEPAD* pad = &controller_state.Gamepad;

							newController->StickAverageX = Win32::ProcessXInputAnalogStick(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
							newController->StickAverageY = Win32::ProcessXInputAnalogStick(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
							if ((newController->StickAverageX != 0.0f) || (newController->StickAverageY != 0.0f))
							{
								newController->IsAnalog = true;
							}

							if (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
							{
								newController->StickAverageY = 1.0f;
								newController->IsAnalog = false;
							}
							if (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
							{
								newController->StickAverageY = -1.0f;
								newController->IsAnalog = false;
							}
							if (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
							{
								newController->StickAverageX = -1.0f;
								newController->IsAnalog = false;
							}
							if (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
							{
								newController->StickAverageX = 1.0f;
								newController->IsAnalog = false;
							}

							real32 threshold = 0.5f;
							Win32::ProcessXInputDigitalButton((newController->StickAverageX < -threshold) ? 1 : 0, 1, &oldController->StickLeft, &newController->StickLeft);
							Win32::ProcessXInputDigitalButton((newController->StickAverageX > threshold) ? 1 : 0, 1, &oldController->StickRight, &newController->StickRight);
							Win32::ProcessXInputDigitalButton((newController->StickAverageY < -threshold) ? 1 : 0, 1, &oldController->StickDown, &newController->StickDown);
							Win32::ProcessXInputDigitalButton((newController->StickAverageY > threshold) ? 1 : 0, 1, &oldController->StickUp, &newController->StickUp);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER, &oldController->LeftShoulder, &newController->LeftShoulder);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER, &oldController->RightShoulder, &newController->RightShoulder);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_A, &oldController->ActionDown, &newController->ActionDown);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_B, &oldController->ActionRight, &newController->ActionRight);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_X, &oldController->ActionLeft, &newController->ActionLeft);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_Y, &oldController->ActionUp, &newController->ActionUp);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_START, &oldController->Start, &newController->Start);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_BACK, &oldController->Back, &newController->Back);
						}
						else
						{
							// NOTE(Craig): Controller not available.
							newController->IsConnected = false;
						}
					}

					DWORD byteToLock;
					DWORD targetCursor;
					DWORD bytesToWrite;
					DWORD playCursor;
					DWORD writeCursor;
					bool32 soundIsValid = false;
					if (SUCCEEDED(_application.MainAudioBuffer->GetCurrentPosition(&playCursor, &writeCursor)))
					{
						byteToLock = (audioInfo.RunningSampleIndex * audioInfo.BytesPerSample) % audioInfo.BufferSize;
						targetCursor = (playCursor + audioInfo.LatencySampleCount * audioInfo.BytesPerSample) % audioInfo.BufferSize;

						if (byteToLock > targetCursor)
						{
							bytesToWrite = audioInfo.BufferSize - byteToLock;
							bytesToWrite += targetCursor;
						}
						else
						{
							bytesToWrite = targetCursor - byteToLock;
						}

						soundIsValid = true;
					}

					AudioBuffer audioBuffer = {};
					audioBuffer.SamplesPerSecond = audioInfo.SamplesPerSecond;
					audioBuffer.SampleCount = bytesToWrite / audioInfo.BytesPerSample;
					audioBuffer.Samples = samples;

					BitmapBuffer bitmapBuffer = {};
					bitmapBuffer.Memory = _application.MainBitmapBuffer.Memory;
					bitmapBuffer.Width = _application.MainBitmapBuffer.Width;
					bitmapBuffer.Height = _application.MainBitmapBuffer.Height;
					bitmapBuffer.Pitch = _application.MainBitmapBuffer.Pitch;

					UpdateAndRender(&memory, newInput, &bitmapBuffer, &audioBuffer);

					if (soundIsValid)
					{
						FillAudioBuffer(&audioInfo, byteToLock, bytesToWrite, &audioBuffer);
					}

					LARGE_INTEGER workCounter = Win32::GetWallClock();
					real32 workSecondsElapsed = Win32::GetTimeDifference(previousCounter, workCounter);

					real32 secondsElapsedForFrame = workSecondsElapsed;
					if (secondsElapsedForFrame < targetFrameTime)
					{
						/*if (sleepIsGranular)
						{
							DWORD sleepMS = (DWORD)(1000.0f * (targetFrameTime -
													secondsElapsedForFrame));
							if (sleepMS > 0)
							{
								Sleep(sleepMS);
							}
						}

						real32 testSecondsElapsedForFrame = Win32::GetTimeDifference(previousCounter, Win32::GetWallClock());

						Assert(testSecondsElapsedForFrame < targetFrameTime);

						while (secondsElapsedForFrame < targetFrameTime)
						{
							secondsElapsedForFrame = Win32::GetTimeDifference(previousCounter, Win32::GetWallClock());
						}*/
					}
					else
					{
						// TODO(Craig): Missed frame rate, logging
					}

					Win32::ClientDimensions client = Win32::GetClientDimensions(window);
					Win32::DisplayBitmapToDevice(deviceContext, &_application.MainBitmapBuffer, client.Width, client.Height);

					Input* temp = newInput;
					newInput = oldInput;
					oldInput = temp;

					LARGE_INTEGER counter = Win32::GetWallClock();
					previousCounter = counter;

					uint64 cycleCount = __rdtsc();
					uint64 cyclesElapsed = cycleCount - previousCycleCount;
					previousCycleCount = cycleCount;

					real32 msPerFrame = 1000.0f * Win32::GetTimeDifference(previousCounter, counter);
					real64 framesPerSecond = 0.0f;
					real64 megaCyclesPerFrame = ((real64)cyclesElapsed / (1000.0f * 1000.0f));

					char buffer[256];
					sprintf_s(buffer, "Frame Time: %.02fms  FPS: %.02f  Megacycles: %.02f\n", msPerFrame, framesPerSecond, megaCyclesPerFrame);
					OutputDebugString(buffer);
				}
			}
			else
			{
				// TODO(Craig): Log that memory allocation failed.
			}
		}
		else
		{
			// TODO(Craig): Log that we failed creation of the window.
		}
	}
	else
	{
		// TODO(Craig): Log the we failed to register the window class.
	}

	return EXIT_SUCCESS;
}

// DEBUG IMPLEMENTATIONS

Debug::FileBuffer Debug::ReadEntireFile(char* filename)
{
	Debug::FileBuffer result = {};

	HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(fileHandle, &fileSize))
		{
			// NOTE(Craig): ReadFile takes DWORD file size, max read is 4GB.
			uint32 fileSize32 = TruncateUInt64(fileSize.QuadPart);
			result.Memory = VirtualAlloc(0, fileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (result.Memory)
			{
				DWORD bytesRead;
				if (ReadFile(fileHandle, result.Memory, fileSize32, &bytesRead, 0) && (fileSize32 == bytesRead))
				{
					// NOTE(Craig): Read success
					result.Size = fileSize32;
				}
				else
				{
					// NOTE(Craig): Read file failure, so free memory
					// TODO(Craig): Logging
					FreeFileMemory(result.Memory);
					result.Memory = 0;
				}
			}
			else
			{
				// TODO(Craig): Logging
			}
		}
		else
		{
			// TODO(Craig): Logging
		}
		CloseHandle(fileHandle);
	}
	else
	{
		// TODO(Craig): Logging
	}
	return result;
}

void Debug::FreeFileMemory(void* memory)
{
	if (memory)
	{
		VirtualFree(memory, 0, MEM_RELEASE);
	}
}

bool32 Debug::WriteEntireFile(char* filename, uint32 memorySize, void* memory)
{
	bool32 result = false;
	HANDLE fileHandle = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten;
		if (WriteFile(fileHandle, memory, memorySize, &bytesWritten, 0))
		{
			// NOTE(Craig): Write success
			result = (bytesWritten == memorySize);
		}
		else
		{
			// TODO(Craig): Logging
		}

		CloseHandle(fileHandle);
	}
	else
	{
		// TODO(Craig): Logging
	}
	return result;
}