#include "win32_platform.h"
#include <xinput.h>
#include <dsound.h>
#include <cstdio>

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

namespace
{

	// TODO(Craig): Change from global.
	static_global bool32 global_running;
	static_global Win32::BitmapBuffer global_back_buffer;
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

}

namespace Win32
{
	// AUDIO
	static_internal void InitializeDirectSound(HWND _window, int32 _samples_per_second, int32 _buffer_size)
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

	static_internal void ClearAudioBuffer(AudioInfo* _buffer)
	{
		VOID* region_1;
		DWORD region_1_size;
		VOID* region_2;
		DWORD region_2_size;
		if (SUCCEEDED(global_secondary_buffer->Lock(0, _buffer->secondary_buffer_size,
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

	static_internal void FillAudioBuffer(AudioInfo* _audio_info, DWORD _byte_to_lock, DWORD _bytes_to_write, Abstract::AudioBuffer* _buffer)
	{
		VOID* region_1;
		DWORD region_1_size;
		VOID* region_2;
		DWORD region_2_size;
		if (SUCCEEDED(global_secondary_buffer->Lock(_byte_to_lock, _bytes_to_write,
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

	static_internal void ProcessXInputDigitalButton(WORD _XInput_button_states, DWORD _button_bit, Abstract::ButtonState* _old, Abstract::ButtonState* _new)
	{
		_new->ended_down = ((_XInput_button_states & _button_bit) == _button_bit);
		_new->transition_count = (_old->ended_down != _new->ended_down) ? 1 : 0;
	}

	static_internal real32 ProcessXInputAnalogStick(SHORT _value, SHORT _dead_zone_threshold)
	{
		real32 result = 0;

		if (_value < -_dead_zone_threshold)
		{
			result = (real32)((_value + _dead_zone_threshold) / (32768.0f - _dead_zone_threshold));
		}
		else if (_value > _dead_zone_threshold)
		{
			result = (real32)((_value - _dead_zone_threshold) / (32767.0f - _dead_zone_threshold));
		}

		return result;
	}

	static_internal void ProcessKeyboardMessage(Abstract::ButtonState* _new, bool32 _is_down)
	{
		Assert(_new->ended_down != _is_down);
		_new->ended_down = _is_down;
		++_new->transition_count;
	}

	// GRAPHICS
	static_internal void ClearBitmapBuffer(BitmapBuffer _buffer, Colour::RGB _colour = Colour::Black)
	{
		uint8* row = (uint8*)_buffer.memory;
		for (int y = 0; y < _buffer.height; ++y)
		{
			uint32* pixel = (uint32*)row;
			for (int x = 0; x < _buffer.width; ++x)
			{
				*pixel++ = ((_colour.red << 16) | (_colour.green << 8) | _colour.blue);
			}

			row += _buffer.pitch;
		}
	}

	static_internal void ResizeBitmapBuffer(BitmapBuffer* _buffer, int _width, int _height)
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
		_buffer->info.bmiHeader.biBitCount = (WORD)(8 * bytes_per_pixel);
		_buffer->info.bmiHeader.biCompression = BI_RGB;

		int bitmap_memory_size = (_width * _height) * bytes_per_pixel;
		_buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		_buffer->pitch = _buffer->width * bytes_per_pixel;

		// Clears buffer to black
		ClearBitmapBuffer(*_buffer);
	}

	static_internal void DisplayBitmapToDevice(HDC _device_context, BitmapBuffer* _buffer, int _client_width, int _client_height)
	{
		StretchDIBits(_device_context,
					  0, 0, _client_width, _client_height,
					  0, 0, _client_width, _client_height,		// No scaling
//					  0, 0, _buffer->width, _buffer->height,	// Scale buffer to client size
					  _buffer->memory,
					  &_buffer->info,
					  DIB_RGB_COLORS, SRCCOPY);
	}

	// WINDOW
	static_internal ClientDimensions GetClientDimensions(HWND _window)
	{
		RECT client_rect;
		GetClientRect(_window, &client_rect);
		ClientDimensions result;
		result.width = client_rect.right - client_rect.left;
		result.height = client_rect.bottom - client_rect.top;
		return result;
	}

	static_internal LRESULT CALLBACK MainWindowCallback(HWND _window, UINT _message, WPARAM _wparameter, LPARAM _lparameter)
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
			Assert(!"Keyboard input came through a non-dispatch method.");
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC device_context = BeginPaint(_window, &paint);
			ClientDimensions client = GetClientDimensions(_window);
			DisplayBitmapToDevice(device_context, &global_back_buffer, client.width, client.height);
			EndPaint(_window, &paint);
		} break;

		default:
		{
			result = DefWindowProc(_window, _message, _wparameter, _lparameter);
		} break;
		}

		return result;
	}

	static_internal void ProcessMessages(Abstract::Controller* _controller)
	{
		MSG message;

		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT)
			{
				global_running = false;
			}

			switch (message.message)
			{
			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			{
				uint32 VK_code = (uint32)message.wParam;
				bool32 key_released = ((message.lParam & (1 << 30)) != 0);
				bool32 key_pressed = ((message.lParam & (1 << 31)) == 0);
				bool32 alt_down = (message.lParam & (1 << 29));

				if (key_pressed == key_released)
				{
					// NOTE(Craig): Key is repeating in this block
				}
				else
				{
					if (VK_code == 'W')
					{
						Win32::ProcessKeyboardMessage(&_controller->stick_up, key_pressed);
					}
					else if (VK_code == 'A')
					{
						Win32::ProcessKeyboardMessage(&_controller->stick_left, key_pressed);
					}
					else if (VK_code == 'S')
					{
						Win32::ProcessKeyboardMessage(&_controller->stick_down, key_pressed);
					}
					else if (VK_code == 'D')
					{
						Win32::ProcessKeyboardMessage(&_controller->stick_right, key_pressed);
					}
					else if (VK_code == 'Q')
					{
						Win32::ProcessKeyboardMessage(&_controller->left_shoulder, key_pressed);
					}
					else if (VK_code == 'E')
					{
						Win32::ProcessKeyboardMessage(&_controller->right_shoulder, key_pressed);
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
						Win32::ProcessKeyboardMessage(&_controller->back, key_pressed);
					}
					else if (VK_code == VK_SPACE)
					{
						Win32::ProcessKeyboardMessage(&_controller->start, key_pressed);
					}
					else if (VK_code == VK_UP)
					{
						Win32::ProcessKeyboardMessage(&_controller->action_up, key_pressed);
					}
					else if (VK_code == VK_DOWN)
					{
						Win32::ProcessKeyboardMessage(&_controller->action_down, key_pressed);
					}
					else if (VK_code == VK_LEFT)
					{
						Win32::ProcessKeyboardMessage(&_controller->action_left, key_pressed);
					}
					else if (VK_code == VK_RIGHT)
					{
						Win32::ProcessKeyboardMessage(&_controller->action_right, key_pressed);
					}
				}

				if ((alt_down) && (VK_code == VK_F4))
				{
					global_running = false;
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
}

int CALLBACK WinMain(HINSTANCE _instance, HINSTANCE _previnstance, LPSTR _command_line, int _command_show)
{
	Win32::ResizeBitmapBuffer(&global_back_buffer, 1920, 1080);
	Win32::LoadXInput();

	LARGE_INTEGER temp_performance_counter_frequency;
	QueryPerformanceFrequency(&temp_performance_counter_frequency);
	int64 performance_counter_frequency = temp_performance_counter_frequency.QuadPart;

	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	window_class.lpfnWndProc = Win32::MainWindowCallback;
	window_class.hInstance = _instance;
	// window_class.hIcon;
	window_class.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&window_class))
	{
		HWND window = CreateWindowEx(0,
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

			Win32::AudioInfo audio_info = {};
			audio_info.samples_per_second = 48000;
			audio_info.running_sample_index = 0;
			audio_info.bytes_per_sample = sizeof(int16) * 2;
			audio_info.secondary_buffer_size = audio_info.samples_per_second * audio_info.bytes_per_sample;
			audio_info.latency_sample_count = audio_info.samples_per_second / 15;

			Win32::InitializeDirectSound(window, audio_info.samples_per_second, audio_info.secondary_buffer_size);
			Win32::ClearAudioBuffer(&audio_info);
			global_secondary_buffer->Play(0, 0, DSBPLAY_LOOPING);

			int16* samples = (int16*)VirtualAlloc(0, audio_info.secondary_buffer_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

#if (INTERNAL_BUILD == true)
			LPVOID base_address = (LPVOID)Terabytes(2);
#else
			LPVOID base_address = 0;
#endif

			Abstract::Memory memory = {};
			memory.permanent_storage_size = Megabytes(64);
			memory.transient_storage_size = Gigabytes(2);
			//memory.transient_storage_size = Megabytes(963); 

			uint64 total_memory_size = memory.permanent_storage_size + memory.transient_storage_size;
			memory.permanent_storage = VirtualAlloc(base_address, (size_t)total_memory_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			memory.transient_storage = ((uint8*)memory.permanent_storage + memory.permanent_storage_size);

			if (samples && memory.permanent_storage && memory.transient_storage)
			{
				Abstract::Input input[2] = {};
				Abstract::Input* new_input = &input[0];
				Abstract::Input* old_input = &input[1];

				LARGE_INTEGER previous_counter;
				QueryPerformanceCounter(&previous_counter);

				uint64 previous_cycle_count = __rdtsc();

				global_running = true;
				while (global_running)
				{
					Abstract::Controller* old_keyboard_controller = GetController(old_input, 0);
					Abstract::Controller* new_keyboard_controller = GetController(new_input, 0);
					*new_keyboard_controller = {};
					new_keyboard_controller->is_connected = true;
					for (uint8 button_index = 0; button_index < ArrayCount(new_keyboard_controller->buttons); ++button_index)
					{
						new_keyboard_controller->buttons[button_index].ended_down = old_keyboard_controller->buttons[button_index].ended_down;
					}

					Win32::ProcessMessages(new_keyboard_controller);

					// NOTE(Craig): Keyboard + Number of controllers allowed.
					uint8 max_controller_count = 1 + XUSER_MAX_COUNT;
					if (max_controller_count > ArrayCount(new_input->controllers))
					{
						max_controller_count = ArrayCount(new_input->controllers);
					}

					// NOTE(Craig): Index starting at 1, 0 is keyboard.
					for (uint8 controller_index = 1; controller_index < max_controller_count; ++controller_index)
					{
						Abstract::Controller* old_controller = GetController(old_input, controller_index);
						Abstract::Controller* new_controller = GetController(new_input, controller_index);

						XINPUT_STATE controller_state;
						if (XInputGetState(controller_index, &controller_state) == ERROR_SUCCESS)
						{
							// NOTE(Craig): Controller available.
							new_controller->is_connected = true;
							
							XINPUT_GAMEPAD* pad = &controller_state.Gamepad;

							new_controller->stick_average_x = Win32::ProcessXInputAnalogStick(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
							new_controller->stick_average_y = Win32::ProcessXInputAnalogStick(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
							if ((new_controller->stick_average_x != 0.0f) || (new_controller->stick_average_y != 0.0f))
							{
								new_controller->is_analog = true;
							}

							if (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
							{
								new_controller->stick_average_y = 1.0f;
								new_controller->is_analog = false;
							}
							if (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
							{
								new_controller->stick_average_y = -1.0f;
								new_controller->is_analog = false;
							}
							if (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
							{
								new_controller->stick_average_x = -1.0f;
								new_controller->is_analog = false;
							}
							if (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
							{
								new_controller->stick_average_x = 1.0f;
								new_controller->is_analog = false;
							}

							real32 threshold = 0.5f;
							Win32::ProcessXInputDigitalButton((new_controller->stick_average_x < -threshold) ? 1 : 0, 1, &old_controller->stick_left, &new_controller->stick_left);
							Win32::ProcessXInputDigitalButton((new_controller->stick_average_x > threshold) ? 1 : 0, 1, &old_controller->stick_right, &new_controller->stick_right);
							Win32::ProcessXInputDigitalButton((new_controller->stick_average_y < -threshold) ? 1 : 0, 1, &old_controller->stick_down, &new_controller->stick_down);
							Win32::ProcessXInputDigitalButton((new_controller->stick_average_y > threshold) ? 1 : 0, 1, &old_controller->stick_up, &new_controller->stick_up);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER, &old_controller->left_shoulder, &new_controller->left_shoulder);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER, &old_controller->right_shoulder, &new_controller->right_shoulder);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_A, &old_controller->action_down, &new_controller->action_down);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_B, &old_controller->action_right, &new_controller->action_right);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_X, &old_controller->action_left, &new_controller->action_left);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_Y, &old_controller->action_up, &new_controller->action_up);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_START, &old_controller->start, &new_controller->start);
							Win32::ProcessXInputDigitalButton(pad->wButtons, XINPUT_GAMEPAD_BACK, &old_controller->back, &new_controller->back);
						}
						else
						{
							// NOTE(Craig): Controller not available.
							new_controller->is_connected = false;
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

					Abstract::AudioBuffer audio_buffer = {};
					audio_buffer.samples_per_second = audio_info.samples_per_second;
					audio_buffer.sample_count = bytes_to_write / audio_info.bytes_per_sample;
					audio_buffer.samples = samples;

					Abstract::BitmapBuffer bitmap_buffer = {};
					bitmap_buffer.memory = global_back_buffer.memory;
					bitmap_buffer.width = global_back_buffer.width;
					bitmap_buffer.height = global_back_buffer.height;
					bitmap_buffer.pitch = global_back_buffer.pitch;

					UpdateAndRender(&memory, new_input, &bitmap_buffer, &audio_buffer);

					if (sound_is_valid)
					{
						FillAudioBuffer(&audio_info, byte_to_lock, bytes_to_write, &audio_buffer);
					}

					Win32::ClientDimensions client = Win32::GetClientDimensions(window);
					DisplayBitmapToDevice(device_context, &global_back_buffer, client.width, client.height);

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

					Abstract::Input* temp = new_input;
					new_input = old_input;
					old_input = temp;
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

FileBuffer ReadEntireFile(char* _filename)
{
	FileBuffer result = {};

	HANDLE file_handle = CreateFile(_filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (file_handle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER file_size;
		if (GetFileSizeEx(file_handle, &file_size))
		{
			// NOTE(Craig): ReadFile takes DWORD file size, max read is 4GB.
			uint32 file_size_32 = TruncateUInt64(file_size.QuadPart);
			result.memory = VirtualAlloc(0, file_size_32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (result.memory)
			{
				DWORD bytes_read;
				if (ReadFile(file_handle, result.memory, file_size_32, &bytes_read, 0) && (file_size_32 == bytes_read))
				{
					// NOTE(Craig): Read success
					result.size = file_size_32;
				}
				else
				{
					// NOTE(Craig): Read file failure, so free memory
					// TODO(Craig): Logging
					FreeFileMemory(result.memory);
					result.memory = 0;
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
		CloseHandle(file_handle);
	}
	else
	{
		// TODO(Craig): Logging
	}
	return result;
}

void FreeFileMemory(void* _memory)
{
	if (_memory)
	{
		VirtualFree(_memory, 0, MEM_RELEASE);
	}
}

bool32 WriteEntireFile(char* _filename, uint32 _memory_size, void* _memory)
{
	bool32 result = false;
	HANDLE file_handle = CreateFile(_filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (file_handle != INVALID_HANDLE_VALUE)
	{
		DWORD bytes_written;
		if (WriteFile(file_handle, _memory, _memory_size, &bytes_written, 0))
		{
			// NOTE(Craig): Write success
			result = (bytes_written == _memory_size);
		}
		else
		{
			// TODO(Craig): Logging
		}

		CloseHandle(file_handle);
	}
	else
	{
		// TODO(Craig): Logging
	}
	return result;
}