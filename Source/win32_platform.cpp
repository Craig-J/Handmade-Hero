#include "win32_platform.h"

#include <malloc.h>
#include <xinput.h>
#include <dsound.h>
#include <cstdio>

/*
	TODO(Craig)

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

// TODO(Craig): Change from global.
static_global bool32 global_running;
static_global Win32BitmapBuffer global_back_buffer;
static_global LPDIRECTSOUNDBUFFER global_secondary_buffer;

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

static_internal void Win32InitializeDirectSound(HWND _window, int32 _samples_per_second, int32 _buffer_size)
{
	HMODULE DSoundLibrary = LoadLibrary("dsound.dll");

	if (DSoundLibrary)
	{
		DirectSoundCreate_Type* DirectSoundCreate = (DirectSoundCreate_Type*)GetProcAddress(DSoundLibrary, "DirectSoundCreate");
		
		LPDIRECTSOUND direct_sound;
		if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &direct_sound, 0)))
		{
			WAVEFORMATEX wave_format = {};
			wave_format.wFormatTag = WAVE_FORMAT_PCM;
			wave_format.nChannels = 2;
			wave_format.nSamplesPerSec = _samples_per_second;
			wave_format.wBitsPerSample = 16;
			wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
			wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
			wave_format.cbSize = 0;

			if (SUCCEEDED(direct_sound->SetCooperativeLevel(_window, DSSCL_PRIORITY)))
			{
				DSBUFFERDESC buffer_description = {};
				buffer_description.dwSize = sizeof(buffer_description);
				buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;

				// NOTE(Craig): Create a primary buffer.
				LPDIRECTSOUNDBUFFER primary_buffer;
				if (SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_description, &primary_buffer, 0)))
				{
					if (SUCCEEDED(primary_buffer->SetFormat(&wave_format)))
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

			DSBUFFERDESC buffer_description = {};
			buffer_description.dwSize = sizeof(buffer_description);
			buffer_description.dwBufferBytes = _buffer_size;
			buffer_description.lpwfxFormat = &wave_format;

			if (SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_description, &global_secondary_buffer, 0)))
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

static_internal void Win32ClearBuffer(Win32AudioInfo* _buffer)
{
	VOID* region_1;
	DWORD region_1_size;
	VOID* region_2;
	DWORD region_2_size;
	if (SUCCEEDED(global_secondary_buffer->Lock(
		0, _buffer->secondary_buffer_size,
		&region_1, &region_1_size,
		&region_2, &region_2_size,
		0)))
	{
		uint8* sample_out = (uint8*)region_1;
		for (DWORD byte_index = 0; byte_index < region_1_size; ++byte_index)
		{
			*sample_out++ = 0;
		}

		sample_out = (uint8*)region_2;
		for (DWORD byte_index = 0; byte_index < region_2_size; ++byte_index)
		{
			*sample_out++ = 0;
		}

		global_secondary_buffer->Unlock(region_1, region_1_size, region_2, region_2_size);
	}
}

static_internal void Win32FillAudioBuffer(Win32AudioInfo* _audio_info, DWORD _byte_to_lock, DWORD _bytes_to_write, AudioBuffer* _buffer)
{
	VOID* region_1;
	DWORD region_1_size;
	VOID* region_2;
	DWORD region_2_size;
	if (SUCCEEDED(global_secondary_buffer->Lock(
		_byte_to_lock, _bytes_to_write,
		&region_1, &region_1_size,
		&region_2, &region_2_size,
		0)))
	{
		int16* sample_in = _buffer->samples;

		DWORD region_1_sample_count = region_1_size / _audio_info->bytes_per_sample;
		int16* sample_out = (int16*)region_1;
		for (DWORD sample_index = 0; sample_index < region_1_sample_count; ++sample_index)
		{
			*sample_out++ = *sample_in++;
			*sample_out++ = *sample_in++;
			++_audio_info->running_sample_index;
		}
		
		DWORD region_2_sample_count = region_2_size / _audio_info->bytes_per_sample;
		sample_out = (int16*)region_2;
		for (DWORD sample_index = 0; sample_index < region_2_sample_count; ++sample_index)
		{
			*sample_out++ = *sample_in++;
			*sample_out++ = *sample_in++;
			++_audio_info->running_sample_index;
		}

		global_secondary_buffer->Unlock(region_1, region_1_size, region_2, region_2_size);
	}
}

static_internal void Win32LoadXInput()
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

static_internal void Win32ClearBuffer(Win32BitmapBuffer _buffer)
{
	uint8* row = (uint8*)_buffer.memory;
	for (int y = 0; y < _buffer.height; ++y)
	{
		uint32* pixel = (uint32*)row;
		for (int x = 0; x < _buffer.width; ++x)
		{
			*pixel++ = 0;
		}

		row += _buffer.pitch;
	}
}

static_internal void Win32ClearBuffer(Win32BitmapBuffer _buffer, uint8 _red, uint8 _green, uint8 _blue)
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

static_internal void Win32ResizeBitmapBuffer(Win32BitmapBuffer* _buffer, int _width, int _height)
{
	// TODO(Craig): Bulletproof this.
	// Don't pre-free, free after (free first if it fails).

	if (_buffer->memory)
	{
		VirtualFree(_buffer->memory, 0, MEM_RELEASE);
	}

	_buffer->width = _width;
	_buffer->height = _height;
	int bytes_per_pixel = 4;

	// NOTE(Craig): When biHeight is negative, this is the clue to windows
	// to treat the bitmap as top-down, not bottom-up, meaning that the buffer
	// starts at the top left pixel not the bottom right.
	_buffer->info.bmiHeader.biSize = sizeof(_buffer->info.bmiHeader);
	_buffer->info.bmiHeader.biWidth = _width;
	_buffer->info.bmiHeader.biHeight = -_height;
	_buffer->info.bmiHeader.biPlanes = 1;
	_buffer->info.bmiHeader.biBitCount = 8 * bytes_per_pixel;
	_buffer->info.bmiHeader.biCompression = BI_RGB;

	int bitmap_memory_size = (_width * _height) * bytes_per_pixel;
	_buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	_buffer->pitch = _buffer->width * bytes_per_pixel;

	// Clears buffer to black
	Win32ClearBuffer(*_buffer, 0, 0, 0);
}

static_internal void Win32DisplayBitmapToDevice(HDC _device_context, Win32BitmapBuffer* _buffer, int _client_width, int _client_height)
{
	StretchDIBits(_device_context,
				  0, 0, _client_width, _client_height,
				  0, 0, _client_width, _client_height,		// No scaling
//				  0, 0, _buffer->width, _buffer->height,	// Scale buffer to client size
				  _buffer->memory,
				  &_buffer->info,
				  DIB_RGB_COLORS, SRCCOPY);
}

static_internal Win32ClientDimensions Win32GetClientDimensions(HWND _window)
{
	RECT client_rect;
	GetClientRect(_window, &client_rect);
	Win32ClientDimensions result;
	result.width = client_rect.right - client_rect.left;
	result.height = client_rect.bottom - client_rect.top;
	return result;
}

static_internal LRESULT CALLBACK Win32MainWindowCallback(HWND _window, UINT _message, WPARAM _wparameter, LPARAM _lparameter)
{
	LRESULT result = 0;

	switch (_message)
	{
	case WM_SIZE:
	{
	} break;

	case WM_DESTROY:
	{
		// TODO(Craig): Handle as error - recreate window?
		global_running = false;
	} break;

	case WM_CLOSE:
	{
		// TODO(Craig): Handle with message to user?
		global_running = false;
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
		uint32 VK_code = (uint32)_wparameter;
		bool32 key_released = ((_lparameter & (1 << 30)) != 0);
		bool32 key_pressed = ((_lparameter & (1 << 31)) == 0);
		bool32 alt_down = (_lparameter & (1 << 29));

		
		if (key_pressed == key_released)
		{
			// NOTE(Craig): Key is repeating in this block
		}
		else
		{
			if (VK_code == 'W')
			{

			}
			else if (VK_code == 'A')
			{

			}
			else if (VK_code == 'S')
			{

			}
			else if (VK_code == 'D')
			{

			}
			else if (VK_code == 'Q')
			{

			}
			else if (VK_code == 'E')
			{

			}
			else if (VK_code == 'R')
			{

			}
			else if (VK_code == 'T')
			{

			}
			else if (VK_code == 'F')
			{

			}
			else if (VK_code == 'G')
			{

			}
			else if (VK_code == VK_ESCAPE)
			{

			}
			else if (VK_code == VK_SPACE)
			{

			}
		}

		if ((alt_down) && (VK_code == VK_F4))
		{
			global_running = false;
		}
	} break;

	case WM_PAINT:
	{
		PAINTSTRUCT paint;
		HDC device_context = BeginPaint(_window, &paint);
		Win32ClientDimensions client = Win32GetClientDimensions(_window);
		Win32DisplayBitmapToDevice(device_context, &global_back_buffer, client.width, client.height);
		EndPaint(_window, &paint);
	} break;

	default:
	{
		result = DefWindowProc(_window, _message, _wparameter, _lparameter);
	} break;
	}

	return result;
}

int CALLBACK WinMain(HINSTANCE _instance, HINSTANCE _previnstance, LPSTR _command_line, int _command_show)
{
	Win32ResizeBitmapBuffer(&global_back_buffer, 1920, 1080);
	Win32LoadXInput();

	LARGE_INTEGER temp_performance_counter_frequency;
	QueryPerformanceFrequency(&temp_performance_counter_frequency);
	int64 performance_counter_frequency = temp_performance_counter_frequency.QuadPart;

	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	window_class.lpfnWndProc = Win32MainWindowCallback;
	window_class.hInstance = _instance;
	// window_class.hIcon;
	window_class.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&window_class))
	{
		HWND window = CreateWindowEx(
			0,
			window_class.lpszClassName,
			"Handmade Hero",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			_instance,
			0);

		if (window)
		{
			HDC device_context = GetDC(window);

			Win32AudioInfo audio_info = {};
			audio_info.samples_per_second = 48000;
			audio_info.running_sample_index = 0;
			audio_info.bytes_per_sample = sizeof(int16) * 2;
			audio_info.secondary_buffer_size = audio_info.samples_per_second * audio_info.bytes_per_sample;
			audio_info.latency_sample_count = audio_info.samples_per_second / 15;
			Win32InitializeDirectSound(window, audio_info.samples_per_second, audio_info.secondary_buffer_size);
			Win32ClearBuffer(&audio_info);
			global_secondary_buffer->Play(0, 0, DSBPLAY_LOOPING);
			int16* samples = (int16*)VirtualAlloc(0, audio_info.secondary_buffer_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			LARGE_INTEGER previous_counter;
			QueryPerformanceCounter(&previous_counter);

			uint64 previous_cycle_count = __rdtsc();

			global_running = true;
			while (global_running)
			{

				MSG message;
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
				{
					if (message.message == WM_QUIT)
					{
						global_running = false;
					}
					
					TranslateMessage(&message);
					DispatchMessage(&message);
				}

				for (DWORD controller = 0; controller < XUSER_MAX_COUNT; ++controller)
				{
					XINPUT_STATE controller_state;
					if (XInputGetState(controller, &controller_state) == ERROR_SUCCESS)
					{
						// NOTE(Craig): Controller available.
						XINPUT_GAMEPAD* pad = &controller_state.Gamepad;

						bool32 up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool32 down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool32 left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool32 right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool32 start = (pad->wButtons & XINPUT_GAMEPAD_START);
						bool32 back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
						bool32 left_shoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool32 right_shoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool32 A = (pad->wButtons & XINPUT_GAMEPAD_A);
						bool32 B = (pad->wButtons & XINPUT_GAMEPAD_B);
						bool32 X = (pad->wButtons & XINPUT_GAMEPAD_X);
						bool32 Y = (pad->wButtons & XINPUT_GAMEPAD_Y);

						int16 stick_x = pad->sThumbLX;
						int16 stick_y = pad->sThumbLY;

					}
					else
					{
						// NOTE(Craig): Controller not available.
					}
				}

				DWORD byte_to_lock;
				DWORD target_cursor;
				DWORD bytes_to_write;
				DWORD play_cursor;
				DWORD write_cursor;
				bool32 sound_is_valid = false;
				if (SUCCEEDED(global_secondary_buffer->GetCurrentPosition(&play_cursor, &write_cursor)))
				{
					byte_to_lock = (audio_info.running_sample_index * audio_info.bytes_per_sample) % audio_info.secondary_buffer_size;
					target_cursor = (play_cursor + audio_info.latency_sample_count * audio_info.bytes_per_sample) % audio_info.secondary_buffer_size;

					if (byte_to_lock > target_cursor)
					{
						bytes_to_write = audio_info.secondary_buffer_size - byte_to_lock;
						bytes_to_write += target_cursor;
					}
					else
					{
						bytes_to_write = target_cursor - byte_to_lock;
					}

					sound_is_valid = true;
				}

				AudioBuffer audio_buffer = {};
				audio_buffer.samples_per_second = audio_info.samples_per_second;
				audio_buffer.sample_count = bytes_to_write / audio_info.bytes_per_sample;
				audio_buffer.samples = samples;

				BitmapBuffer bitmap_buffer = {};
				bitmap_buffer.memory = global_back_buffer.memory;
				bitmap_buffer.width = global_back_buffer.width;
				bitmap_buffer.height = global_back_buffer.height;
				bitmap_buffer.pitch = global_back_buffer.pitch;

				AppUpdateAndRender(&bitmap_buffer, &audio_buffer);

				if (sound_is_valid)
				{
					Win32FillAudioBuffer(&audio_info, byte_to_lock, bytes_to_write, &audio_buffer);
				}

				Win32ClientDimensions client = Win32GetClientDimensions(window);
				Win32DisplayBitmapToDevice(device_context, &global_back_buffer, client.width, client.height);

				uint64 cycle_count_end = __rdtsc();

				LARGE_INTEGER counter_end;
				QueryPerformanceCounter(&counter_end);

				uint64 cycles_elapsed = cycle_count_end - previous_cycle_count;
				int64 counter_elapsed = counter_end.QuadPart - previous_counter.QuadPart;
				real32 ms_per_frame = (1000.0f * (real32)counter_elapsed) / (real32)performance_counter_frequency;
				real32 frames_per_second = (real32)performance_counter_frequency / (real32)counter_elapsed;
				real32 mega_cycles_per_frame = (real32)cycles_elapsed / (1000.0f * 1000.0f);

				char buffer[256];
				sprintf_s(buffer, "Frame Time: %.02fms  FPS: %.02f  Megacycles: %.02f\n", ms_per_frame, frames_per_second, mega_cycles_per_frame);
				OutputDebugString(buffer);

				previous_cycle_count = cycle_count_end;
				previous_counter = counter_end;
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