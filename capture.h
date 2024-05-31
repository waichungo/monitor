#ifndef _CAPTURER_H
#define _CAPTURER_H
#include <Windows.h>
#include <vector>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct MEMIMG
    {
        int size;
        void *buffer;
    } MEMIMG;

    MEMIMG HbitmapToJPEG(HBITMAP bitmap, int width, int height);
    HBITMAP CaptureWindow(HWND hWnd);
    MEMIMG CaptureWindowAsJPEG(HWND hWnd, BOOL usePrintWindow);
    HBITMAP CaptureWindow(HWND hWnd);
    MEMIMG GetJPEGFromPixels(char *pixels, int width, int height, int totalPixelBytes);

#ifdef __cplusplus
}
#endif
std::vector<unsigned char> GetPNGFromPixels(char *pixels, int width, int height, int totalPixelBytes);
std::vector<uint8_t> HbitmapToPNG(HBITMAP bitmap, int width, int height);
std::vector<uint8_t> HbitmapToPNG2(HBITMAP bitmap, int width, int height);
#endif