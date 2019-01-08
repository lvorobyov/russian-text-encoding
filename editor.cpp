/**
 * editor.cpp
 *
 * Created on: 27.11.2017
 *     Author: Lev Vorobjev
 *      Email: lev.vorobjev@rambler.ru
 *
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 30.11.2017
 * @License: MIT
 * @Copyright: Copyright (c) 2017 Lev Vorobjev
 */

#include <windows.h>
#include "except.h"
#include "editor.h"

void TextEditor::emptyFile() {
    SetWindowText(_hEdit, (LPTSTR) NULL);
    if (_lpszText == NULL) {
        _lpszText = new TCHAR[1];
    }
    _tcscpy(_lpszText, TEXT("\0"));
    _cchText = 0;
    if (_hFile != NULL) {
        CloseHandle(_hFile);
        _hFile = NULL;
    }
}

void TextEditor::openFile(LPCTSTR lpszFilename) {
    if (_hFile != NULL) {
        if (_hFile != NULL) {
            CloseHandle(_hFile);
            _hFile = NULL;
        }
        delete _lpszText;
        _lpszText = NULL;
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
        raise_system_error("CreateFile");
    }

    hFileMapping = CreateFileMapping(_hFile, NULL,
        PAGE_READONLY | SEC_COMMIT, 0, 0, NULL);
    if (hFileMapping == NULL) {
        raise_system_error("CreateFileMapping");
    }

    nFileSize = GetFileSize(_hFile, NULL);
    lpData = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if (lpData == NULL) {
        raise_system_error("MapViewOfFile");
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
    _cchText = _tcslen(_lpszText);
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
        UINT cchTextNew = getTextLength();
        if (cchTextNew > _cchText) {
            delete _lpszText;
            _lpszText = new TCHAR[cchTextNew + 1];
        }
        _cchText = GetWindowText(_hEdit, _lpszText, cchTextNew + 1);
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

    SetFilePointer(_hFile, 0, NULL, FILE_BEGIN);
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
        raise_system_error("CreateFile");
    }

    // Заменить открытый файл новым
    if (_hFile != NULL) {
        CloseHandle(_hFile);
        _hFile = NULL;
    }
    _hFile = hNewFile;

    saveFile();
}

int TextEditor::writeToBuffer(LPVOID lpBuffer, int nBufferSize) {
    int nFileSize;
    int nTextSize;

    if (SendMessage(_hEdit, EM_GETMODIFY, 0, 0L)) {
        UINT cchTextNew = getTextLength();
        if (cchTextNew > _cchText) {
            delete _lpszText;
            _lpszText = new TCHAR[cchTextNew + 1];
        }
        _cchText = GetWindowText(_hEdit, _lpszText, cchTextNew + 1);
    }

    nTextSize = _cchText;
#ifdef _UNICODE
    nFileSize = WideCharToMultiByte(CP_UTF8, 0,
        _lpszText, nTextSize, NULL, 0, NULL, NULL);
    if (lpBuffer == NULL) {
        return nFileSize;
    }
    if (nFileSize > nBufferSize)
        nFileSize = nBufferSize;
    WideCharToMultiByte(CP_UTF8, 0,
        _lpszText, nTextSize, (LPSTR)lpBuffer, nFileSize, NULL, NULL);
#else
    nFileSize = nTextSize;
    if (lpBuffer == NULL) {
        return nFileSize;
    }
    if (nFileSize > nBufferSize)
        nFileSize = nBufferSize;
    _tcsncpy((LPTSTR)lpBuffer, _lpszText, nFileSize);
#endif

    return nFileSize;
}

int TextEditor::readFromBuffer(LPVOID lpBuffer, int nBytesToRead) {
    int nFileSize = nBytesToRead;
    int nTextSize;

#ifdef _UNICODE
    nTextSize = MultiByteToWideChar(CP_UTF8, 0,
        (LPCSTR)lpBuffer, nFileSize, NULL, 0);
    delete _lpszText;
    _lpszText = new TCHAR[nTextSize + 1];
    MultiByteToWideChar(CP_UTF8, 0,
        (LPCSTR)lpBuffer, nFileSize, _lpszText, nTextSize);
    _lpszText[nTextSize] = 0;
#else
    nTextSize = nFileSize;
    delete _lpszText;
    _lpszText = new TCHAR[nTextSize];
    _tcsncpy(_lpszText, (LPCSTR)lpBuffer, nTextSize);
#endif
    _cchText = _tcslen(_lpszText);
    SetWindowText(_hEdit, _lpszText);

    return nFileSize;
}
