#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>

#if RAND_MAX == 32767
#define Rand32() ((rand() << 16) + (rand() << 1) + (rand() & 1))
#else
#define Rand32() rand()
#endif

static BOOL quit = FALSE;

struct frame_t{
    int width;
    int height;
    UINT32 *pixels; // Pointer to pixel data
} frame = {0};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static BITMAPINFO frameBitmapInfo;
static HBITMAP frameBitmap;
static HDC frameDC;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = {
        .lpfnWndProc = WindowProc,
        .hInstance = hInstance,
        .lpszClassName = CLASS_NAME
    };

    RegisterClass(&wc);

    frameBitmapInfo.bmiHeader.biSize = sizeof(frameBitmapInfo.bmiHeader);
    frameBitmapInfo.bmiHeader.biPlanes = 1;
    frameBitmapInfo.bmiHeader.biBitCount = 32;
    frameBitmapInfo.bmiHeader.biCompression = BI_RGB;
    frameDC = CreateCompatibleDC(0);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    while (!quit) {
        // Run the message loop.
        static MSG msg = { 0 };
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            DispatchMessage(&msg); 
        }
		InvalidateRect(hwnd, NULL, FALSE);
        UpdateWindow(hwnd);

        static unsigned int p = 0;
        int perRun = 100;
        for (int i = 0; i < perRun; i++) {
            frame.pixels[(p++) % (frame.width * frame.height)] = Rand32(); // Fill with color
            frame.pixels[Rand32() % (frame.width * frame.height)] = 0;
        }
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            

            BitBlt(hdc,
				ps.rcPaint.left,
                ps.rcPaint.top,
				ps.rcPaint.right - ps.rcPaint.left,
				ps.rcPaint.bottom - ps.rcPaint.top,
				frameDC,
				ps.rcPaint.left,
				ps.rcPaint.top,
				SRCCOPY);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_SIZE: {
			frameBitmapInfo.bmiHeader.biWidth = LOWORD(lParam);
			frameBitmapInfo.bmiHeader.biHeight = HIWORD(lParam); // Negative height for top-down bitmap
            if(frameBitmap) {
                DeleteObject(frameBitmap);
			}
			frameBitmap = CreateDIBSection(NULL, &frameBitmapInfo, DIB_RGB_COLORS, &frame.pixels, 0, 0);
			SelectObject(frameDC, frameBitmap);
			frame.width = LOWORD(lParam);
			frame.height = HIWORD(lParam);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
