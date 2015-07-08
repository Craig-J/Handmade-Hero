#include <Windows.h>

LRESULT CALLBACK MainWindowCallback(
	HWND   _window,
	UINT   _message,
	WPARAM _wparameter,
	LPARAM _lparameter)
{
	LRESULT result = 0;

	switch (_message)
	{
	case WM_SIZE:
	{
		OutputDebugString("WM_SIZE\n");
	} break;

	case WM_DESTROY:
	{
		OutputDebugString("WM_DESTROY\n");
	} break;

	case WM_CLOSE:
	{
		OutputDebugString("WM_CLOSE\n");
	} break;

	case WM_ACTIVATEAPP:
	{
		OutputDebugString("WM_ACTIVATEAPP\n");
	} break;

	case WM_PAINT:
	{
		PAINTSTRUCT paint;
		HDC device_context = BeginPaint(_window, &paint);
		int x = paint.rcPaint.left;
		int y = paint.rcPaint.top;
		int height = paint.rcPaint.bottom - paint.rcPaint.top;
		int width = paint.rcPaint.right - paint.rcPaint.left;
		static DWORD operation = WHITENESS;
		if (operation == WHITENESS)
		{
			operation = BLACKNESS;
		}
		else
		{
			operation = WHITENESS;
		}
		PatBlt(device_context, x, y, width, height, operation);
		EndPaint(_window, &paint);
	} break;

	default:
	{
		result = DefWindowProc(_window, _message, _wparameter, _lparameter);
	} break;
	}

	return result;
}

int CALLBACK WinMain(
	HINSTANCE _instance,
	HINSTANCE _previnstance,
	LPSTR _command_line,
	int _command_show)
{
	WNDCLASS window_class = {};
	window_class.style = CS_OWNDC;
	window_class.lpfnWndProc = MainWindowCallback;
	window_class.hInstance = _instance;
	// window_class.hIcon;
	window_class.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&window_class))
	{
		HWND window_handle = CreateWindowEx(
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

		if (window_handle)
		{
			MSG message;
			for (;;)
			{
				BOOL message_result = GetMessage(&message, 0, 0, 0);
				if (message_result > 0)
				{
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				else
				{
					break;
				}
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

	return EXIT_SUCCESS;
}