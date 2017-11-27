/**
 * editor.h
 *
 * Created on: 27.11.2017
 *     Author: Lev Vorobjev
 *      Email: lev.vorobjev@rambler.ru
 *
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 27.11.2017
 * @License: MIT
 * @Copyright: Copyright (c) 2017 Lev Vorobjev
 */

#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <win32/win32_error.h>
#include <stdexcept>

/**
 * TextEditor
 * Класс предоставляет пользователю возможность ввода текста,
 * чтения и записи текстовых файлов.
 */
class TextEditor {
private:
    /**
     * _hWndParent
     * Дескриптор родительского окна
     */
    HWND _hWndParent;

    /**
     * _hEdit
     * Дескриптор текстового поля ввода
     */
    HWND _hEdit;

    /**
     * _hFile
     * Дескриптор открытого текстового файла
     */
    HANDLE _hFile = NULL;

    /**
     * _lpszText
     * Текст поля ввода
     */
    LPTSTR _lpszText = NULL;

    /**
     * _cchText
     * Количество символов в тексте
     */
    UINT _cchText = 0;

public:
    /**
     * Конструктор с параметрами
     * Устанавливает поле ввода в родительском окне
     * @param hWndParent дескриптор родительского окна
     * @param hMenu идентификатор текстового поля
     * @param hInst дескриптор экземпляра приложения
     */
    TextEditor(HWND hWndParent, HMENU hMenu, HINSTANCE hInst) {
        RECT rcClient = {0};
        _hWndParent = hWndParent;
        // Создать текстовое поле для вывода открытого файла
        GetClientRect(_hWndParent, &rcClient);
        _hEdit = CreateWindow(TEXT("EDIT"), NULL,
            WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL |
            ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            rcClient.left, rcClient.top, rcClient.right, rcClient.bottom,
            _hWndParent, hMenu, hInst, NULL);
    }

    virtual ~TextEditor() {
        CloseHandle(_hFile);
        delete _lpszText;
    }

    /**
     * emptyFile()
     * Создать новый пустой текстовый документ
     */
    void emptyFile() {
        SendMessage(_hEdit, WM_CLEAR, 0, 0L);
        if (_lpszText != NULL) {
            _tcscpy(_lpszText, TEXT("\0"));
        }
        _cchText = 0;
        if (_hFile != NULL) {
            CloseHandle(_hFile);
            _hFile = NULL;
        }
    }

    /**
     * openFile()
     * Открыть текстовый файл
     * @param lpszFilename имя открываемого файла
     */
    void openFile(LPCTSTR lpszFilename) {
        if (_hFile != NULL) {
            emptyFile();
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

    /**
     * saveFile()
     * Сохраняет изменения в открытом текстовом файле
     */
    void saveFile() {
        if (_hFile == NULL) {
            throw std::invalid_argument("_hFile is NULL");
        }

        LPVOID lpData = NULL;
        DWORD dwBytesWritten;
        UINT nFileSize;
        UINT nTextSize;

        if (SendMessage(_hEdit, EM_GETMODIFY, 0, 0L)) {
            _cchText = GetWindowText(_hEdit, _lpszText, _cchText);
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

    /**
     * saveFile(fileName)
     * Сохраняет документ в текстовом файле
     * @param lpszFilename имя текстового файла, в который сохраняется документ
     */
    void saveFile(LPCTSTR lpszFilename) {
        if (_hFile != NULL) {
            CloseHandle(_hFile);
            _hFile = NULL;
        }

        _hFile = CreateFile(lpszFilename,
            GENERIC_READ | GENERIC_WRITE,
            0, NULL, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, NULL);
        if (_hFile == INVALID_HANDLE_VALUE) {
            throw win32::win32_error("CreateFile");
        }

        // Определить размер текста в поле ввода
        INT nTextLength = 0;
        INT nLineCount = SendMessage(_hEdit, EM_GETLINECOUNT, 0, 0L);
        for (INT nLine = 0; nLine < nLineCount; nLine ++) {
            INT nLineIndex = SendMessage(_hEdit, EM_LINEINDEX, (WPARAM) nLine, 0L);
            nTextLength += SendMessage(_hEdit, EM_LINELENGTH, (WPARAM) nLineIndex, 0L);
        }
        _cchText = nTextLength;
        _lpszText = new TCHAR[nTextLength];

        saveFile();
    }

    /**
     * getText()
     * Получить текст в кодировке UTF16 / ASCII
     */
    LPTSTR getText() {
        if (SendMessage(_hEdit, EM_GETMODIFY, 0, 0L)) {
            _cchText = GetWindowText(_hEdit, _lpszText, _cchText);
        }
        return _lpszText;
    }

    /**
     * setText()
     * Установить текст в кодировке UTF16 / ASCII
     */
    void setText(LPTSTR lpszText) {
        delete _lpszText;
        _cchText = _tcslen(lpszText);
        _lpszText = new TCHAR[_cchText];
        _tcscpy(_lpszText, lpszText);
        SetWindowText(_hEdit, _lpszText);
    }

    /**
     * getModify()
     * Проверить флаг обновления
     * @return TRUE, если пользователь изменил текст, иначе FALSE
     */
    BOOL getModify() {
        return SendMessage(_hEdit, EM_GETMODIFY, 0, 0L);
    }

    /**
     * getTextLength()
     * Получить длину текста в символах
     * @return количество символов в тексте
     */
    INT getTextLength() {
        INT nTextLength = 0;
        INT nLineCount = SendMessage(_hEdit, EM_GETLINECOUNT, 0, 0L);
        for (INT nLine = 0; nLine < nLineCount; nLine ++) {
            INT nLineIndex = SendMessage(_hEdit, EM_LINEINDEX, (WPARAM) nLine, 0L);
            nTextLength += SendMessage(_hEdit, EM_LINELENGTH, (WPARAM) nLineIndex, 0L);
        }
        return nTextLength;
    }

    /**
     * undo()
     * Отменить последнее действие пользователя
     */
    void undo() {
        SendMessage(_hEdit, EM_UNDO, 0, 0L);
    }

    /**
     * copy()
     * Копировать выделенный текст в буфер обмена
     */
    void copy() {
        SendMessage(_hEdit, WM_COPY, 0, 0L);
    }

    /**
     * paste()
     * Вставить текст из буфера обмена в текущую позицию курсора
     */
    void paste() {
        SendMessage(_hEdit, WM_PASTE, 0, 0L);
    }

    /**
     * cut()
     * Удаление выделенного текста с записью его в буфер обмена
     */
    void cut() {
        SendMessage(_hEdit, WM_CUT, 0, 0L);
    }

    /**
     * selectAll()
     * Выделить весь текст
     */
    void selectAll() {
        SendMessage(_hEdit, EM_SETSEL, 0, MAKELPARAM(0, -1));
    }

    /**
     * setSize(width, height)
     * Изменить размеры текстового поля
     */
    void setSize(int width, int height) {
        SetWindowPos(_hEdit, HWND_TOP, 0, 0,
            width, height, SWP_NOMOVE);
    }

    /**
     * isFileOpened()
     * Проверяет, открыт ли файл
     */
    BOOL isFileOpened() {
        return _hFile != NULL;
    }
};
