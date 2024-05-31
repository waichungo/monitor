#include "capture.h"
#include <Windows.h>

#include "jpeglib.h"
#include "lodepng.h"
#include "vector"
#include "iostream"
#include "wincodec.h"

#include "setjmp.h"

BITMAPINFOHEADER createBitmapHeader(int width, int height)
{
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    return bi;
}
HBITMAP CaptureWindow(HWND hWnd)
{
    // get handles to a device context (DC)
    HDC hwindowDC = GetDC(hWnd);
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);
    RECT shape;
    memset((void *)&shape, 0, sizeof(shape));
    GetClientRect(hWnd, &shape);

    // define scale, height and width
    int scale = 1;
    int screenx = shape.left;
    int screeny = shape.top;
    int width = shape.right - shape.left;
    int height = shape.bottom - shape.top;

    // create a bitmap
    HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    BITMAPINFOHEADER bi = createBitmapHeader(width, height);

    // use the previously created device context with the bitmap
    SelectObject(hwindowCompatibleDC, hbwindow);

    // copy from the window device context to the bitmap device context
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screenx, screeny, width, height, SRCCOPY); // change SRCCOPY to NOTSRCCOPY for wacky colors !

    // avoid memory leak
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hWnd, hwindowDC);

    return hbwindow;
}
MEMIMG HbitmapToJPEG(HBITMAP bitmap, int width, int height)
{
    MEMIMG res;
    memset(&res, 0, sizeof(MEMIMG));
    HDC hdc = GetDC(0);
    BITMAP bm;

    if (width == 0 || height == 0)
    {
        if (GetObjectA(bitmap, sizeof(BITMAP), &bm) != 0)
        {
            width = bm.bmWidth;
            height = bm.bmHeight;

            // DeleteObject(&bm);
        }
        else
        {
            return res;
        }
    }
    BITMAPINFOHEADER bi = createBitmapHeader(width, height);
    DWORD dwBmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;
    char *lpbitmap = (char *)malloc(dwBmpSize);
    BOOL captured = GetDIBits(hdc, bitmap, 0, height, lpbitmap, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    if (captured)
    {
        res = GetJPEGFromPixels(lpbitmap, width, height, bi.biBitCount / 8);
    }
    return res;
}
std::vector<uint8_t> HbitmapToPNG2(HBITMAP bitmap, int width, int height)
{
    std::vector<uint8_t> pngBytes;

    // Initialize COM
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
        return pngBytes;

    IWICImagingFactory *pFactory = nullptr;
    IWICBitmap *pBitmap = nullptr;
    IWICStream *pStream = nullptr;
    IWICBitmapEncoder *pEncoder = nullptr;
    IWICBitmapFrameEncode *pFrame = nullptr;
    IPropertyBag2 *pPropertyBag = nullptr;
    HGLOBAL hGlobal = NULL;

    do
    {
        // Create WIC imaging factory
        hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
        if (FAILED(hr))
            break;

        // Create WIC bitmap from HBITMAP
        hr = pFactory->CreateBitmapFromHBITMAP(bitmap, NULL, WICBitmapUseAlpha, &pBitmap);
        if (FAILED(hr))
            break;

        // Create a memory stream
        hr = pFactory->CreateStream(&pStream);
        if (FAILED(hr))
            break;

        // Initialize the stream from a global memory handle
        hGlobal = GlobalAlloc(GMEM_MOVEABLE, 0);
        if (hGlobal == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = CreateStreamOnHGlobal(hGlobal, TRUE, (LPSTREAM *)&pStream);
        if (FAILED(hr))
            break;

        // Create a PNG encoder
        hr = pFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder);
        if (FAILED(hr))
            break;

        // Initialize the encoder with the memory stream
        hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
        if (FAILED(hr))
            break;

        // Create a new frame
        hr = pEncoder->CreateNewFrame(&pFrame, &pPropertyBag);
        if (FAILED(hr))
            break;

        // Initialize the frame
        hr = pFrame->Initialize(pPropertyBag);
        if (FAILED(hr))
            break;
        if (height == 0 || width == 0)
        {
            UINT lwidth, lheight;
            pBitmap->GetSize(&lwidth, &lheight);
            height = lheight;
            width = lwidth;
        }
        // Set frame size 
        hr = pFrame->SetSize(width, height);
        if (FAILED(hr))
            break;

        // Set pixel format
        WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
        hr = pFrame->SetPixelFormat(&format);
        if (FAILED(hr))
            break;

        // Write the bitmap to the frame
        hr = pFrame->WriteSource(pBitmap, NULL);
        if (FAILED(hr))
            break;

        // Commit the frame
        hr = pFrame->Commit();
        if (FAILED(hr))
            break;

        // Commit the encoder
        hr = pEncoder->Commit();
        if (FAILED(hr))
            break;

        // Get the size of the stream
        STATSTG statstg;
        hr = pStream->Stat(&statstg, STATFLAG_DEFAULT);
        if (FAILED(hr))
            break;

        // Allocate memory and read the stream
        ULONG size = static_cast<ULONG>(statstg.cbSize.QuadPart);
        pngBytes.resize(size);
        LARGE_INTEGER zero = {};
        pStream->Seek(zero, STREAM_SEEK_SET, NULL);
        ULONG bytesRead = 0;
        hr = pStream->Read(pngBytes.data(), size, &bytesRead);
        if (FAILED(hr))
            break;

    } while (false);

    // Cleanup
    if (pFrame)
        pFrame->Release();
    if (pPropertyBag)
        pPropertyBag->Release();
    if (pEncoder)
        pEncoder->Release();
    if (pStream)
        pStream->Release();
    if (pBitmap)
        pBitmap->Release();
    if (pFactory)
        pFactory->Release();
    if (hGlobal)
        GlobalFree(hGlobal);
    CoUninitialize();
    return pngBytes;
}
std::vector<uint8_t> HbitmapToPNG(HBITMAP bitmap, int width, int height)
{
    std::vector<uint8_t> res;
    HDC hdc = GetDC(0);
    BITMAP bm;

    if (width == 0 || height == 0)
    {
        if (GetObjectA(bitmap, sizeof(BITMAP), &bm) != 0)
        {
            width = bm.bmWidth;
            height = bm.bmHeight;
            DeleteObject(&bm);
        }
        else
        {
            return res;
        }
    }
    BITMAPINFOHEADER bi = createBitmapHeader(width, height);
    DWORD dwBmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;
    char *lpbitmap = (char *)malloc(dwBmpSize);
    BOOL captured = GetDIBits(hdc, bitmap, 0, height, lpbitmap, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    if (captured)
    {
        res = GetPNGFromPixels(lpbitmap, width, height, bi.biBitCount / 8);
    }
    return res;
}
MEMIMG CaptureWindowAsJPEG(HWND hWnd, BOOL usePrintWindow)
{
    MEMIMG img;
    img.size = 0;
    // get handles to a device context (DC)
    HDC hwindowDC = GetDC(hWnd);
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);
    RECT shape;
    memset((void *)&shape, 0, sizeof(shape));

    GetClientRect(hWnd, &shape);
    HBITMAP captureBitmap;

    int screenx = shape.left;
    int screeny = shape.top;
    int width = shape.right - shape.left;
    int height = shape.bottom - shape.top;
    captureBitmap = CreateCompatibleBitmap(hwindowDC, width, height);

    // create a bitmap
    BOOL captured = FALSE;

    BITMAPINFOHEADER bi = createBitmapHeader(width, height);
    DWORD dwBmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;

    SelectObject(hwindowCompatibleDC, captureBitmap);
    if (usePrintWindow)
    {
        captured = PrintWindow(hWnd, hwindowCompatibleDC, 0);
    }

    if (!usePrintWindow || (usePrintWindow && !captured))
    {
        captured = StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screenx, screeny, width, height, SRCCOPY);
    }

    if (captured)
    {
        char *lpbitmap = (char *)malloc(dwBmpSize);
        if (lpbitmap != NULL)
        {
            captured = GetDIBits(hwindowCompatibleDC, captureBitmap, 0, height, lpbitmap, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

            if (captured)
            {
                img = GetJPEGFromPixels(lpbitmap, width, height, bi.biBitCount / 8);
            }
        }
        free(lpbitmap);
    }
    // avoid memory leak
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hWnd, hwindowDC);
    DeleteDC(hwindowDC);
    DeleteObject(captureBitmap);

    return img;
}

MEMIMG GetJPEGFromPixels(char *pixels, int width, int height, int totalPixelBytes)
{
    // Step 1: Capture the bit
    MEMIMG img;
    img.size = 0;
    // BITMAP bitmap;
    // GetObject(hBitmap, sizeof(BITMAP), &bitmap);

    // Step 3: Initialize JPEG structures
    if (pixels != NULL)
    {
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;

        cinfo.err = jpeg_std_error(&jerr);
        unsigned long size = 0;

        jpeg_create_compress(&cinfo);
        unsigned char *buffer = NULL;

        jpeg_mem_dest(&cinfo, &buffer, &size);

        cinfo.image_width = width;
        cinfo.image_height = height;
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;

        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, 90, TRUE); // Set quality (adjust as needed)

        jpeg_start_compress(&cinfo, TRUE);

        // Step 4: Encode and save the JPEG data
        unsigned char *row_buffer = (unsigned char *)malloc(3 * width);
        JSAMPROW row_pointer = row_buffer;

        int row = 0;
        int start = 0;
        int line = 0;
        while (cinfo.next_scanline < cinfo.image_height)
        {
            line = (width * totalPixelBytes * row);
            for (int x = 0; x < width; x++)
            {
                start = (x * totalPixelBytes) + line;

                row_buffer[3 * x] = pixels[start + 2];
                row_buffer[3 * x + 1] = pixels[start + 1];
                row_buffer[3 * x + 2] = pixels[start];
            }
            (void)jpeg_write_scanlines(&cinfo, &row_pointer, 1);
            row++;
        }

        free(row_buffer);
        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);
        img.buffer = buffer;
        img.size = size;
    }
    return img; // Successful conversion
}
std::vector<unsigned char> GetPNGFromPixels(char *pixels, int width, int height, int totalPixelBytes)
{

    auto size = width * height * 4;
    std::vector<unsigned char> row_buffer;
    row_buffer.resize(size);

    int start = 0;
    int line = 0;
    int bufferLine = 0;
    for (int y = 0; y < height; y++)
    {
        line = (width * totalPixelBytes * y);
        bufferLine = 4 * width * y;
        for (int x = 0; x < width; x++)
        {
            start = (x * totalPixelBytes) + line;

            row_buffer[(4 * x) + bufferLine] = pixels[start + 2];
            row_buffer[(4 * x + 1) + bufferLine] = pixels[start + 1];
            row_buffer[(4 * x + 2) + bufferLine] = pixels[start];
            row_buffer[(4 * x + 3) + bufferLine] = 255;
        }
    }
    std::vector<unsigned char> png;
    auto error = lodepng::encode(png, row_buffer, width, height);
    return png; // Successful conversion
}