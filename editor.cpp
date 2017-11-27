/**
 * editor.cpp
 *
 * Created on: 27.11.2017
 *     Author: Lev Vorobjev
 *      Email: lev.vorobjev@rambler.ru
 *
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 28.11.2017
 * @License: MIT
 * @Copyright: Copyright (c) 2017 Lev Vorobjev
 */

#include <windows.h>
#include <win32/win32_error.h>
#include <stdexcept>

#include "editor.h"

void TextEditor::emptyFile() {
    SetWindowText(_hEdit, (LPTSTR) NULL);
    delete _lpszText;
    _lpszText = NULL;
    _cchText = 0;
    if (_hFile != NULL) {
        CloseHandle(_hFile);
        _hFile = NULL;
    }
}

void TextEditor::openFile(LPCTSTR lpszFilename) {
    if (_hFile != NULL) {
        emptyFile();
    }

    HANDLE hFileMapping;
    LPVOID lpData = NULL;
    UINT nFileSize;
    UINT nTextSize;

    _hFile = CreateFile(lpszFilename,
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (_hFile == INVALID_HANDLE_VALUE) {
        throw win32::win32_error("CreateFile");
    }

    hFileMapping = CreateFileMapping(_hFile, NULL,
        PAGE_READONLY | SEC_COMMIT, 0, 0, NULL);
    if (hFileMapping == NULL) {
        throw win32::win32_error("CreateFileMapping");
    }

    nFileSize = GetFileSize(_hFile, NULL);
    lpData = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if (lpData == NULL) {
        throw win32::win32_error("MapViewOfFile");
    }

#ifdef _UNICODE
    nTextSize = MultiByteToWideChar(CP_UTF8, 0,
        (LPCSTR)lpData, -1, NULL, 0);
    _lpszText = new TCHAR[nTextSize];
    MultiByteToWideChar(CP_UTF8, 0,
        (LPCSTR)lpData, -1, _lpszText, nTextSize);
#else
    nTextSize = nFileSize;
    _lpszText = new TCHAR[nTextSize];
    _tcscpy(_lpszText, (LPCSTR)lpData);
#endif
    _cchText = nTextSize;
    SetWindowText(_hEdit, _lpszText);
    UnmapViewOfFile(lpData);
    CloseHandle(hFileMapping);
}

void TextEditor::saveFile() {
    if (_hFile == NULL) {
        throw std::invalid_argument("_hFile is NULL");
    }

    LPVOID lpData = NULL;
    DWORD dwBytesWritten;
    UINT nFileSize;
    UINT nTextSize;

    if (SendMessage(_hEdit, EM_GETMODIFY, 0, 0L)) {
        nTextSize = getTextLength();
        if (nTextSize > _cchText) {
            _cchText = nTextSize;
            delete _lpszText;
            _lpszText = new TCHAR[_cchText + 1];
        }
        _cchText = GetWindowText(_hEdit, _lpszText, _cchText + 1);
    }

    nTextSize = _cchText;
#ifdef _UNICODE
    nFileSize = WideCharToMultiByte(CP_UTF8, 0,
        _lpszText, nTextSize, NULL, 0, NULL, NULL);
    lpData = new BYTE[nFileSize];
    WideCharToMultiByte(CP_UTF8, 0,
        _lpszText, nTextSize, (LPSTR)lpData, nFileSize, NULL, NULL);
#else
    nFileSize = nTextSize;
    lpData = new BYTE[nFileSize];
    _tcscpy((LPTSTR)lpData, _lpszText);
#endif

    WriteFile(_hFile, lpData, nFileSize, &dwBytesWritten, NULL);
    delete (LPBYTE)lpData;

    SendMessage(_hEdit, EM_SETMODIFY, (WPARAM)(UINT)FALSE, 0L);
}

void TextEditor::saveFile(LPCTSTR lpszFilename) {
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

    if (_lpszText == NULL) {
        // Определить размер текста в поле ввода
        _cchText = getTextLength();
        _lpszText = new TCHAR[_cchText + 1];
    }

    saveFile();
}
