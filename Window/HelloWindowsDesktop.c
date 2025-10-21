#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include "picSample.h"

static BOOL quit = FALSE;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static BITMAPINFO frameBitmapInfo;
static HBITMAP frameBitmap;
static HBITMAP frameBitmapSecond;
static HDC frameDC;

int state = 0; // 0 or 1

canvas_t canvas = {0};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    init(&canvas);
    
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
        int ret = 0;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg); 
        }
        DWORD wait = WaitForSingleObject(canvas.show_mutex,10);
        if (wait != 0) {
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
            ReleaseMutex(canvas.show_mutex);
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
            
            if (state == 0) {
                SelectObject(frameDC, frameBitmapSecond);
				state = 1;
            }else if(state == 1) {
                SelectObject(frameDC, frameBitmap);
				state = 0;
            }

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
            WaitForSingleObject(canvas.process_mutex, INFINITE);
            WaitForSingleObject(canvas.show_mutex, INFINITE);
			frameBitmapInfo.bmiHeader.biWidth = LOWORD(lParam);
			frameBitmapInfo.bmiHeader.biHeight = -HIWORD(lParam); // Negative height for top-down bitmap
            if(frameBitmap) {
                DeleteObject(frameBitmap);
			}
            frameBitmapSecond = CreateDIBSection(NULL, &frameBitmapInfo, DIB_RGB_COLORS, (&canvas)->pixels_process, 0, 0);
            frameBitmap = CreateDIBSection(NULL, &frameBitmapInfo, DIB_RGB_COLORS, (&canvas)->pixels_show, 0, 0);
			SelectObject(frameDC, frameBitmap);
            state = 0;
            canvas.width = LOWORD(lParam);
            canvas.height = HIWORD(lParam);
            ReleaseMutex(canvas.process_mutex);
            ReleaseMutex(canvas.show_mutex);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
