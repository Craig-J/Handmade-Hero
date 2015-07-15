#include <Windows.h>
#include <Xinput.h>
#include <dsound.h>
#include "type_definitions.h"

struct Win32BitmapBuffer
{
	// NOTE(Craig): Pixels are 32-bit; memory order BB GG RR XX.
	BITMAPINFO info;
	void* memory;
	int width;
	int height;
	int pitch;
};

struct Win32ClientDimensions
{
	int width;
	int height;
};

// TODO(Craig): Change from global.
static_global bool global_running;
static_global Win32BitmapBuffer global_back_buffer;

// NOTE(Craig): First define a macro to create functions with correct signature.
// Second create a type with this function signature using the macro.
// Third make a default stub function using the macro.
// Fourth create a function pointer that points to the stub function.
// Fifth define the functions normal name to reference the function pointer instead.
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(XInputGetState_Type);
X_INPUT_GET_STATE(XInputGetState_Stub)
{
	return ERROR_DEVICE_NOT_CONNECTED;
}
static_global XInputGetState_Type* XInputGetState_FuncPtr = XInputGetState_Stub;
#define XInputGetState XInputGetState_FuncPtr

// NOTE(Craig): XInputSetState default function.
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(XInputSetState_Type);
X_INPUT_SET_STATE(XInputSetState_Stub)
{
	return ERROR_DEVICE_NOT_CONNECTED;
}
static_global XInputSetState_Type* XInputSetState_FuncPtr = XInputSetState_Stub;
#define XInputSetState XInputSetState_FuncPtr

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(DirectSoundCreate_Type);

static_internal void Win32LoadXInput()
{
	HMODULE XInputLibrary = LoadLibrary("xinput1_4.dll");
	if (!XInputLibrary)
	{
		// TODO(Craig): Log xinput version.
		HMODULE XInputLibrary = LoadLibrary("xinput1_3.dll");
	}
	if (XInputLibrary)
	{
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

static_internal void Win32InitializeDirectSound(HWND _window, i32 _samples_per_second, i32 _buffer_size)
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
			LPDIRECTSOUNDBUFFER secondary_buffer;
			if (SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_description, &secondary_buffer, 0)))
			{

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

static_internal Win32ClientDimensions Win32GetClientDimensions(HWND _window)
{
	RECT client_rect;
	GetClientRect(_window, &client_rect);
	Win32ClientDimensions result;
	result.width = client_rect.right - client_rect.left;
	result.height = client_rect.bottom - client_rect.top;
	return result;
}

static_internal void Win32RenderWeirdGradient(Win32BitmapBuffer* _buffer, int _blue_offset, int _green_offset, int _red_offset)
{
	ui8* row = (ui8*)_buffer->memory;
	for (int y = 0; y < _buffer->height; ++y)
	{
		ui32* pixel = (ui32*)row;
		for (int x = 0; x < _buffer->width; ++x)
		{
			/*

				Pixel (32-bit)

				Memory:		BB GG RR xx
				Register:	xx RR GG BB

			*/

			ui8 blue = (x + _blue_offset);
			ui8 green = (y + _green_offset);
			ui8 red = _red_offset;

			*pixel++ = ((red << 16) | (green << 8) | blue);
		}

		row += _buffer->pitch;
	}
}

void Win32ClearBuffer(Win32BitmapBuffer _buffer, ui8 _red, ui8 _green, ui8 _blue)
{
	ui8* row = (ui8*)_buffer.memory;
	for (int y = 0; y < _buffer.height; ++y)
	{
		ui32* pixel = (ui32*)row;
		for (int x = 0; x < _buffer.width; ++x)
		{
			/*

			Pixel (32-bit)

			Memory:		BB GG RR xx
			Register:	xx RR GG BB

			*/

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

static_internal void Win32DisplayBitmapToDevice(HDC _device_context, Win32BitmapBuffer _buffer, int _client_width, int _client_height)
{
	StretchDIBits(_device_context,
				  0, 0, _client_width, _client_height,
				  0, 0, _buffer.width, _buffer.height,
				  _buffer.memory,
				  &_buffer.info,
				  DIB_RGB_COLORS, SRCCOPY);
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
		ui32 VK_code = _wparameter;
		bool key_released = ((_lparameter & (1 << 30)) != 0);
		bool key_pressed = ((_lparameter & (1 << 31)) == 0);
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
				OutputDebugString("esc pressed\n");
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
		Win32DisplayBitmapToDevice(device_context, global_back_buffer, client.width, client.height);
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
	Win32ResizeBitmapBuffer(&global_back_buffer, 1600, 900);
	Win32LoadXInput();

	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW;
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
			int x_offset = 0;
			int y_offset = 0;

			Win32InitializeDirectSound(window, 48000, 48000*sizeof(i16)*2);

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

						bool up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
						bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
						bool left_shoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool right_shoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool A = (pad->wButtons & XINPUT_GAMEPAD_A);
						bool B = (pad->wButtons & XINPUT_GAMEPAD_B);
						bool X = (pad->wButtons & XINPUT_GAMEPAD_X);
						bool Y = (pad->wButtons & XINPUT_GAMEPAD_Y);

						i16 stick_x = pad->sThumbLX;
						i16 stick_y = pad->sThumbLY;
					}
					else
					{
						// NOTE(Craig): Controller not available.
					}
				}

				Win32RenderWeirdGradient(&global_back_buffer, x_offset, y_offset, 0);

				HDC device_context = GetDC(window);
				Win32ClientDimensions client = Win32GetClientDimensions(window);
				Win32DisplayBitmapToDevice(device_context, global_back_buffer, client.width, client.height);
				ReleaseDC(window, device_context);

				++x_offset;
				y_offset += 2;
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