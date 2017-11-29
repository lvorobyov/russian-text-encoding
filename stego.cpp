/**
 * stego.cpp
 *
 * Created on: 28.11.2017
 *     Author: Lev Vorobjev
 *      Email: lev.vorobjev@rambler.ru
 *
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 29.11.2017
 * @License: MIT
 * @Copyright: Copyright (c) 2017 Lev Vorobjev
 */

#include "stego.h"
#include <win32/win32_error.h>
#include <stdexcept>

void StegoContainer::open(LPCTSTR lpszFilename) {
    if (_hFile != NULL) {
        close();
    }

    LPVOID lpStego;
    DWORD dwBytesRead;
    DWORD nFileSize;

    WORD wBmpType;
    DWORD dwBmpSize;

    _hFile = CreateFile(lpszFilename,
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (_hFile == INVALID_HANDLE_VALUE) {
        throw win32::win32_error("CreateFile");
    }

    nFileSize = GetFileSize(_hFile, NULL);
    _hglbStego = GlobalAlloc(GMEM_FIXED, nFileSize);
    lpStego = (LPVOID) GlobalLock(_hglbStego);
    if (lpStego == NULL) {
        throw win32::win32_error("GlobalLock");
    }

    if (! ReadFile(_hFile, lpStego, nFileSize, &dwBytesRead, NULL)) {
        throw win32::win32_error("ReadFile");
    }

    wBmpType = ((LPBITMAPFILEHEADER)lpStego)->bfType;
    dwBmpSize = ((LPBITMAPINFO)((LPBYTE)lpStego+sizeof(BITMAPFILEHEADER)))->bmiHeader.biSize;

    GlobalUnlock(_hglbStego);

    if (wBmpType != 0x4D42 || dwBmpSize != 40) {
        throw std::invalid_argument("Unrecognized bitmap file");
    }
}

void StegoContainer::save() {
    if (_hFile == NULL) {
        throw std::invalid_argument("_hFile is NULL");
    }

    DWORD dwBytesWritten;
    DWORD nFileSize;
    LPVOID lpStego;

    nFileSize = GlobalSize(_hglbStego);

    lpStego = (LPVOID) GlobalLock(_hglbStego);
    if (lpStego == NULL) {
        throw win32::win32_error("GlobalLock");
    }

    SetFilePointer(_hFile, 0, NULL, FILE_BEGIN);
    if (! WriteFile(_hFile, lpStego, nFileSize, &dwBytesWritten, NULL)) {
        throw win32::win32_error("ReadFile");
    }

    GlobalUnlock(_hglbStego);
}

void StegoContainer::save(LPCTSTR lpszFilename) {
    // Создать новый файл
    HANDLE hNewFile = CreateFile(lpszFilename,
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hNewFile == INVALID_HANDLE_VALUE) {
        throw win32::win32_error("CreateFile");
    }

    // Заменить открытый файл новым
    if (_hFile != NULL) {
        CloseHandle(_hFile);
        _hFile = NULL;
    }
    _hFile = hNewFile;

    save();
}

void StegoContainer::close() {
    if (_hFile != NULL) {
        CloseHandle(_hFile);
        _hFile = NULL;
    }
    if (_hglbStego != NULL) {
        GlobalFree(_hglbStego);
        _hglbStego = NULL;
    }
}

int StegoContainer::stego(LPBYTE lpbData, int nDataLen) {
	int nResult = 0;
    LPVOID lpStego;
    LPBITMAPFILEHEADER bmpHeader;
    LPBITMAPINFOHEADER bmpInfo;
	DWORD dwBits;
	DWORD dwBytes;
	DWORD dwSizeImage;
    lpStego = (LPVOID) GlobalLock(_hglbStego);
    if (lpStego == NULL) {
        throw win32::win32_error("GlobalLock");
    }

    bmpHeader = (LPBITMAPFILEHEADER)lpStego;
    bmpInfo = (LPBITMAPINFOHEADER)((LPBYTE)lpStego+sizeof(BITMAPFILEHEADER));

    // Количество значимых битов в одной строке битового изображения
	dwBits = bmpInfo->biBitCount * bmpInfo->biWidth;
    // Количество байтов в одной строке битового изображения
	dwBytes = (dwBits + 31)/32 * 4;
    // Количество байтов для всего битового изображения
	dwSizeImage = dwBytes * bmpInfo->biHeight;

	if (lpbData == NULL) {
		nResult = dwSizeImage * 2 / 8;
	} else {
        if (nDataLen > dwSizeImage * 2 / 8) {
            GlobalUnlock(_hglbStego);
            throw std::invalid_argument("nDataLen greater than maximal data lenght");
        }

		LPBYTE lpbPixel = (LPBYTE)lpStego + bmpHeader->bfOffBits;
		for (int i=0; i<bmpInfo->biHeight && nResult < nDataLen; i++) {
			for (int j=0; j < dwBytes / 4 && nResult < nDataLen; j++) {
				for (int k=0; k<4; k++) {
					lpbPixel[4*j + k] &= 0xFC;
					lpbPixel[4*j + k] |= ((*lpbData >> (6 - k*2)) & 0x03);
				}
				lpbData++;
				nResult++;
			}
			lpbPixel += dwBytes;
		}
	}

    GlobalUnlock(_hglbStego);

	return nResult;
}

int StegoContainer::unstego(LPBYTE lpbData, int nDataMaxLen) {
	int nResult = 0;
    LPVOID lpStego;
    LPBITMAPFILEHEADER bmpHeader;
    LPBITMAPINFOHEADER bmpInfo;
    DWORD dwBits;
    DWORD dwBytes;
    DWORD dwSizeImage;

    if (lpbData == NULL) {
        throw std::invalid_argument("lpbData is NULL");
    }

    lpStego = (LPVOID) GlobalLock(_hglbStego);
    if (lpStego == NULL) {
        throw win32::win32_error("GlobalLock");
    }

    bmpHeader = (LPBITMAPFILEHEADER)lpStego;
    bmpInfo = (LPBITMAPINFOHEADER)((LPBYTE)lpStego+sizeof(BITMAPFILEHEADER));

    // Количество значимых битов в одной строке битового изображения
	dwBits = bmpInfo->biBitCount * bmpInfo->biWidth;
    // Количество байтов в одной строке битового изображения
	dwBytes = (dwBits + 31)/32 * 4;
    // Количество байтов для всего битового изображения
	dwSizeImage = dwBytes * bmpInfo->biHeight;

    LPBYTE lpbPixel = (LPBYTE)lpStego + bmpHeader->bfOffBits;
    BYTE bData;
    for (int i=0; i<bmpInfo->biHeight && nResult < nDataMaxLen; i++) {
        for (int j=0; j < dwBytes / 4 && nResult < nDataMaxLen; j++) {
            bData = 0;
            for (int k=0; k<4; k++) {
                bData <<= 2;
                bData |= (lpbPixel[4*j + k] & 0x03);
            }
            *lpbData = bData;
            lpbData++;
            nResult++;
        }
        lpbPixel += dwBytes;
    }

    GlobalUnlock(_hglbStego);

	return nResult;
}
