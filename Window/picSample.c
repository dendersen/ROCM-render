#include "picSample.h"

DWORD WINAPI generatePicture(canvas_t* frameBuffers) {
	unsigned int p = 0;
	while (1) {
		int width = frameBuffers->width;
		int height = frameBuffers->height;

		for (int i = 0; i < frameBuffers->height * frameBuffers->height; i++) {
			WaitForSingleObject(frameBuffers->process_mutex, INFINITE); // Lock the mutex
			frameBuffers->pixels_process[(p++) % (frameBuffers->width * frameBuffers->height)] = Rand32(); // Fill with color
			ReleaseMutex(frameBuffers->process_mutex); // Unlock the mutex
		}
	WaitForSingleObject(frameBuffers->show_mutex, INFINITE); // Lock the mutex
	UINT32* temp = frameBuffers->pixels_show;
	frameBuffers->pixels_show = frameBuffers->pixels_process;
	frameBuffers->pixels_process = temp; // Swap buffers
	ReleaseMutex(frameBuffers->show_mutex); // Unlock the mutex
	}
}

int updateFrame(canvas_t* frameBuffer) {
	int ret = WaitForSingleObject(frameBuffer->show_mutex, INFINITE); // Lock the mutex
	if (ret != WAIT_OBJECT_0) {
		return ret;
	}
	ret = WaitForSingleObject(frameBuffer->process_mutex, INFINITE); // Lock the mutex
	if (ret != WAIT_OBJECT_0) {
		ReleaseMutex(*frameBuffer->show_mutex);
		return ret;
	}
	UINT32* temp = frameBuffer->pixels_show;
	frameBuffer->pixels_show = frameBuffer->pixels_show; // Copy the pointer
	frameBuffer->pixels_process = temp;
	ReleaseMutex(*frameBuffer->process_mutex);
	ReleaseMutex(*frameBuffer->show_mutex);
	return 0;
}

DWORD WINAPI mainFunc(canvas_t* frameBuffer) {
	generatePicture(frameBuffer);
	return 0;
}

int init(canvas_t* frameBuffer) {
	frameBuffer->pixels_show    = (UINT32*)malloc(sizeof(UINT32) * frameBuffer->width * frameBuffer->height);
	frameBuffer->pixels_process = (UINT32*)malloc(sizeof(UINT32) * frameBuffer->width * frameBuffer->height);
	if(frameBuffer->pixels_show == NULL || frameBuffer->pixels_process == NULL) {
		return FALSE;
	}
	frameBuffer->process_mutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL               // unnamed mutex
	);
	frameBuffer->show_mutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL               // unnamed mutex
	);
	
	HANDLE thread = CreateThread(
		NULL,
		0,
		mainFunc,
		frameBuffer,
		0,
		NULL
	);
	return TRUE;
}
