#pragma once
#include <windows.h>

typedef struct canvas {
	int width;
	int height;
	UINT32* pixels_show; // Pointer to pixel data
	UINT32* pixels_process; // Pointer to pixel data
	HANDLE* show_mutex; // Mutex for thread safety
	HANDLE* process_mutex; // Mutex for thread safety
} canvas_t;

int init(canvas_t* frameBuffer);

#if RAND_MAX == 32767
#define Rand32() ((rand() << 16) + (rand() << 1) + (rand() & 1))
#else
#define Rand32() rand()
#endif