#include <windows.h>

typedef struct canvas {
	int width;
	int height;
	UINT32* pixels_show; // Pointer to pixel data
	UINT32* pixels_process; // Pointer to pixel data
	HANDLE* mutex; // Mutex for thread safety
} canvas_t;

DWORD WINAPI generatePicture(canvas_t* frameBuffers) {
	unsigned int p = 0;
	while (1) {
		WaitForSingleObject(*frameBuffers->mutex, INFINITE); // Lock the mutex
		int width = frameBuffers->width;
		int height = frameBuffers->height;
		ReleaseMutex(*frameBuffers->mutex); // Unlock the mutex
		for (int i = 0; i < frameBuffers->height * frameBuffers->height; i++) {
			frameBuffers->pixels_process[(p++) % (frameBuffers->width * frameBuffers->height)] = Rand32(); // Fill with color
			frameBuffers->pixels_process[Rand32() % (frameBuffers->width * frameBuffers->height)] = 0; // remove a pixel
		}
	WaitForSingleObject(*frameBuffers->mutex, INFINITE); // Lock the mutex
	UINT32* temp = frameBuffers->pixels_show;
	frameBuffers->pixels_show = frameBuffers->pixels_process;
	frameBuffers->pixels_process = temp; // Swap buffers
	ReleaseMutex(*frameBuffers->mutex); // Unlock the mutex
	}
}

int init(canvas_t* frameBuffer) {
	HANDLE thread = CreateThread(
		NULL,
		0,
		generatePicture,
		frameBuffer,
		0,
		NULL
	);
	return TRUE;
}

#define FLAT_SAMPLE 0
#define AVERAGE_SAMPLE 1

int simpleDownSample(UINT32* source, int sourceWidth, int sourceHeight, UINT32* target, int targetWidth, int targetHeight, int sampleStyle) {
	int widthSample = sourceWidth / targetWidth;
	int heightSample = sourceHeight / targetHeight;
	switch (sampleStyle) {
		case FLAT_SAMPLE: {
			for (int y = 0; y < targetHeight; y++) {
				for (int x = 0; x < targetWidth; x++) {
					int sourceX = x * widthSample;
					int sourceY = y * heightSample;
					UINT32 pixel = source[sourceY * sourceWidth + sourceX];
					target[y * targetWidth + x] = pixel;
				}
			}
			return TRUE;
		}
		case AVERAGE_SAMPLE: {
			for (int y = 0; y < targetHeight; y++) {
				for (int x = 0; x < targetWidth; x++) {
					int sourceX = x * widthSample;
					int sourceY = y * heightSample;
					int R = 0, G = 0, B = 0;
					for (int dy = 0; dy < heightSample; dy++) {
						for (int dx = 0; dx < widthSample; dx++) {
							int pixelIndex = (sourceY + dy) * sourceWidth + (sourceX + dx);
							UINT32 pixel = source[pixelIndex];
							R += (pixel & 0x00FF0000) >> 16; // Extract Red
							G += (pixel & 0x0000FF00) >> 8;  // Extract Green
							B += (pixel & 0x000000FF);       // Extract Blue
						}
					}
					//average the colors
					int totalPixels = widthSample * heightSample;
					R /= totalPixels;
					G /= totalPixels;
					B /= totalPixels;

					// make Pixel
					UINT32 pixel = B | (G << 8) | (R << 16); // Combine back to UINT32
					target[y * targetWidth + x] = pixel; // Store in target buffer
				}
			}
			return TRUE;
		}
		default:
			return FALSE; // Unsupported sample operation
	}
}

int simpleUpSample(UINT32* source, int sourceWidth, int sourceHeight, UINT32* target, int targetWidth, int targetHeight, int sampleStyle) {
	int widthSample = targetWidth / sourceWidth;
	int heightSample = targetHeight / sourceHeight;
	switch (sampleStyle) {
	case FLAT_SAMPLE: {
		for (int y = 0; y < sourceHeight; y++) {
			for (int x = 0; x < sourceWidth; x++) {
				int sourceX = x * sourceWidth / targetWidth;
				int sourceY = y * sourceHeight / targetHeight;
				UINT32 pixel = source[sourceY * sourceWidth + sourceX];
				for (int dx = 0; dx < widthSample; dx++) {
					for (int dy = 0; dy < heightSample; dy++) {
						target[((y * heightSample) + dy) * targetWidth + ((x * widthSample) + dx)] = pixel;
					}
				}
			}
		}
		return TRUE;
	}
	default:
		return FALSE;
	}
}

int sample(canvas_t* frameBuffer, int targetWidth, int targetHeight, UINT32* targetBuffer, int sampleStyle) {
	if (!frameBuffer || !frameBuffer->pixels_show) {
		return FALSE; // Invalid frame buffer
	}
	if(frameBuffer->width % targetWidth == 0 || frameBuffer->height % targetHeight == 0) {
		return simpleDownSample(frameBuffer->pixels_show, 
			frameBuffer->width, frameBuffer->height, 
			targetBuffer,
			targetWidth, targetHeight,
			sampleStyle
		);
	}
	if(targetWidth % frameBuffer->width == 0 || targetHeight % frameBuffer->height == 0) {
		return simpleUpSample(frameBuffer->pixels_show, 
			frameBuffer->width, frameBuffer->height, 
			targetBuffer,
			targetWidth, targetHeight,
			sampleStyle
		);
	}

	return FALSE; // Unsupported sample operation
}