/**
 * editor.h
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
#include <string.h>
#include <tchar.h>

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
        _lpszText = new TCHAR[1];
        _tcscpy(_lpszText, TEXT("\0"));
    }

    virtual ~TextEditor() {
        CloseHandle(_hFile);
        delete _lpszText;
    }

    /**
     * emptyFile()
     * Создать новый пустой текстовый документ
     */
    void emptyFile();

    /**
     * openFile()
     * Открыть текстовый файл
     * @param lpszFilename имя открываемого файла
     */
    void openFile(LPCTSTR lpszFilename);

    /**
     * saveFile()
     * Сохраняет изменения в открытом текстовом файле
     */
    void saveFile();

    /**
     * saveFile(fileName)
     * Сохраняет документ в текстовом файле
     * @param lpszFilename имя текстового файла, в который сохраняется документ
     */
    void saveFile(LPCTSTR lpszFilename);

    /**
     * getText()
     * Получить текст в кодировке UTF16 / ASCII
     */
    LPTSTR getText() {
        if (SendMessage(_hEdit, EM_GETMODIFY, 0, 0L)) {
            UINT cchTextNew = getTextLength();
            if (cchTextNew > _cchText) {
                delete _lpszText;
                _lpszText = new TCHAR[cchTextNew + 1];
            }
            _cchText = GetWindowText(_hEdit, _lpszText, cchTextNew + 1);
        }
        return _lpszText;
    }

    /**
     * setText()
     * Установить текст в кодировке UTF16 / ASCII
     */
    void setText(LPCTSTR lpszText) {
        UINT cchTextNew = _tcslen(lpszText);
        if (cchTextNew > _cchText) {
            delete _lpszText;
            _lpszText = new TCHAR[cchTextNew + 1];
        }
        _cchText = cchTextNew;
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
     * Получить актуальную длину текста в поле ввода
     * @return количество символов в тексте
     */
    INT getTextLength() {
        if (getModify() == FALSE) {
            return _cchText;
        }
        INT nTextLength = 0;
        INT nLineCount = SendMessage(_hEdit, EM_GETLINECOUNT, 0, 0L);
        for (INT nLine = 0; nLine < nLineCount; nLine ++) {
            INT nLineIndex = SendMessage(_hEdit, EM_LINEINDEX, (WPARAM) nLine, 0L);
            nTextLength += SendMessage(_hEdit, EM_LINELENGTH, (WPARAM) nLineIndex, 0L);
            if (nLine != nLineCount - 1) {
                nTextLength += 2; // CR LF
            }
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
     * clear()
     * Удаление выделенного текста без копирования в буфер обмена
     */
    void clear() {
        SendMessage(_hEdit, WM_CLEAR, 0, 0L);
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
