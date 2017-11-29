/**
 * main.cpp
 *
 * Created on: 25.11.2017
 *     Author: Lev Vorobjev
 *      Email: lev.vorobjev@rambler.ru
 *
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 29.11.2017
 * @License: MIT
 * @Copyright: Copyright (c) 2017 Lev Vorobjev
 */

#include <windows.h>
#include <wincrypt.h>
#include "editor.h"
#include "stego.h"
#include "encfile.h"
#include "resource.h"

#define IDC_EDITTEXT  40050

#define WND_TITLE TEXT("Стеганография")
#define MSG_TITLE TEXT("Lab6")
#define BUFFER_SIZE 512

#define HANDLE_ERROR(lpszFunctionName, dwStatus) \
    _stprintf(lpszBuffer, TEXT("%s error.\nStatus code: %d"), \
        TEXT(lpszFunctionName), dwStatus); \
    MessageBox(hWnd, lpszBuffer, MSG_TITLE, MB_OK | MB_ICONWARNING);

#define DEBUG_INFO(editor) \
    _stprintf(lpszBuffer, TEXT("TextLength: %d\nStatus: %s\nText: %s"), \
        editor->getTextLength(), \
        editor->getModify() ? TEXT("MODIFIED") : TEXT("NOT MODIFIED"), \
        editor->getText()); \
    MessageBox(hWnd, lpszBuffer, MSG_TITLE, MB_OK | MB_ICONINFORMATION);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
ATOM RegMyWindowClass(HINSTANCE, LPCTSTR);

int APIENTRY WinMain(HINSTANCE hInstance,
             HINSTANCE         hPrevInstance,
             LPSTR             lpCmdLine,
             int               nCmdShow) {
    LPCTSTR lpszClass = TEXT("SPAD_Lab6_Window");
    LPCTSTR lpszTitle = WND_TITLE;
    HWND hWnd;
    MSG msg = {0};
    BOOL status;

    if (!RegMyWindowClass(hInstance, lpszClass))
        return 1;

    hWnd = CreateWindow(lpszClass, lpszTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);
    if(!hWnd) return 2;

    ShowWindow(hWnd, nCmdShow);

    while ((status = GetMessage(&msg, NULL, 0, 0 )) != 0) {
        if (status == -1) return 3;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

ATOM RegMyWindowClass(HINSTANCE hInst, LPCTSTR lpszClassName) {
    WNDCLASS wcWindowClass = {0};
    wcWindowClass.lpfnWndProc = (WNDPROC)WndProc;
    wcWindowClass.style = CS_HREDRAW|CS_VREDRAW;
    wcWindowClass.hInstance = hInst;
    wcWindowClass.lpszClassName = lpszClassName;
    wcWindowClass.lpszMenuName = MAKEINTRESOURCE(IDR_APPMENU);
    wcWindowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcWindowClass.hbrBackground = (HBRUSH) ( COLOR_WINDOW + 1);
    wcWindowClass.cbClsExtra = 0;
    wcWindowClass.cbWndExtra = 0;
    return RegisterClass(&wcWindowClass);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                         WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    HINSTANCE hInst;
    PAINTSTRUCT ps;

    static OPENFILENAME ofn = {0};
    static LPTSTR lpszFilename;
    static LPTSTR lpszBuffer;
    static HMENU hMenu;
    static TextEditor* editor;
    DWORD dwStatus;

    switch (message) {
      case WM_CREATE:
        hInst = (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
        lpszFilename = new TCHAR[BUFFER_SIZE];
        _tcscpy(lpszFilename, TEXT("\0"));
        lpszBuffer = new TCHAR[BUFFER_SIZE];
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hInstance = hInst;
        ofn.hwndOwner = hWnd;
        ofn.lpstrFile = lpszFilename;
        ofn.nMaxFile = BUFFER_SIZE;
        ofn.lpstrFilter = TEXT("Все файлы\0*.*\0Файлы ключей\0*.pek\0");
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
        // Получение дескрипторов разделов меню
        hMenu = GetMenu(hWnd);
        editor = new TextEditor(hWnd, (HMENU)IDC_EDITTEXT, hInst);
        break;
      case WM_SIZE:
        editor->setSize(LOWORD(lParam), HIWORD(lParam));
        break;
      case WM_COMMAND:
        switch (LOWORD(wParam)) {

            case IDC_EDITTEXT:
                switch (HIWORD(wParam)) {
                    case EN_CHANGE:
                        // Пользователь изменил текст
                        EnableMenuItem(GetSubMenu(hMenu, 0), IDM_ITEM2, MF_BYCOMMAND | MF_ENABLED);
                        break;
                }
                break;

            case IDM_ITEM1:
            {
                if (editor->isFileOpened() && editor->getModify()) {
                    if (MessageBox(hWnd, TEXT("Сохранить изменения в файле?"),
                        MSG_TITLE, MB_YESNO) == IDYES) {
                        editor->saveFile();
                    }
                }

                // Открыть файл
                ofn.lpstrTitle = NULL;
                if (GetOpenFileName(&ofn)) {

                    editor->openFile(lpszFilename);

                    _stprintf(lpszBuffer, TEXT("%s: %s"),
                        WND_TITLE, lpszFilename);
                    SetWindowText(hWnd, lpszBuffer);

                } else {
                    dwStatus = CommDlgExtendedError();
                    if (dwStatus != 0) {
                        _stprintf(lpszBuffer, TEXT("Open file dialog error.\nStatus code: %d"), dwStatus);
                        MessageBox(hWnd, lpszBuffer, MSG_TITLE, MB_OK | MB_ICONWARNING);
                    }
                }
                break;
            }

            case IDM_ITEM2:
            {
                // Сохранить файл
                if (editor->isFileOpened()) {
                    editor->saveFile();
                    EnableMenuItem(GetSubMenu(hMenu, 0), IDM_ITEM2, MF_BYCOMMAND | MF_GRAYED);
                    break;
                }
            }

            case IDM_ITEM3:
            {
                // Сохранить файл как...
                ofn.lpstrTitle = NULL;
                if (GetSaveFileName(&ofn)) {
                    editor->saveFile(lpszFilename);

                    EnableMenuItem(GetSubMenu(hMenu, 0), IDM_ITEM2, MF_BYCOMMAND | MF_GRAYED);

                    _stprintf(lpszBuffer, TEXT("%s: %s"),
                        WND_TITLE, lpszFilename);
                    SetWindowText(hWnd, lpszBuffer);
                } else {
                    dwStatus = CommDlgExtendedError();
                    if (dwStatus != 0) {
                        _stprintf(lpszBuffer, TEXT("Save file dialog error.\nStatus code: %d"), dwStatus);
                        MessageBox(hWnd, lpszBuffer, MSG_TITLE, MB_OK | MB_ICONWARNING);
                    }
                }
                break;
            }

            case IDM_ITEM4:
            {
                if (editor->isFileOpened() && editor->getModify()) {
                    if (MessageBox(hWnd, TEXT("Сохранить изменения в файле?"),
                        MSG_TITLE, MB_YESNO) == IDYES) {
                        editor->saveFile();
                    }
                }
                editor->emptyFile();
                EnableMenuItem(GetSubMenu(hMenu, 0), IDM_ITEM2, MF_BYCOMMAND | MF_GRAYED);
                SetWindowText(hWnd, WND_TITLE);
                break;
            }

            case IDM_ITEMUNDO:
                editor->undo();
                break;
            case IDM_ITEMCUT:
                editor->cut();
                break;
            case IDM_ITEMCOPY:
                editor->copy();
                break;
            case IDM_ITEMPASTE:
                editor->paste();
                break;
            case IDM_ITEMALL:
                editor->selectAll();
                break;

            case IDM_ITEMSTEGO:
            {
                HCRYPTPROV hProv;
                HCRYPTHASH hUserHash;
                HCRYPTKEY hAesKey;
                HCRYPTHASH hMsgHash;
                StegoContainer stego;
                LPVOID lpData = NULL;
                int nDataSize;
                int nSignSize;
                ENCFILE_HEADER efh = {0};

                if (! CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, 0)) {
                    HANDLE_ERROR("CryptAquireContext", GetLastError());
                    break;
                }

                if (! CryptCreateHash(hProv, CALG_SHA_256, NULL, 0, &hUserHash)) {
                    CryptReleaseContext(hProv, 0);
                    HANDLE_ERROR("CryptCreateHash", GetLastError());
                    break;
                }

                if (! CryptCreateHash(hProv, CALG_MD5, NULL, 0, &hMsgHash)) {
                    CryptReleaseContext(hProv, 0);
                    HANDLE_ERROR("CryptCreateHash", GetLastError());
                    break;
                }

                // Встроить данные в контейнер
                ofn.lpstrTitle = TEXT("Открыть файл контейнера");
                if (GetOpenFileName(&ofn)) {
                    stego.open(lpszFilename);
                    nDataSize = editor->writeToBuffer(NULL, 0);
                    nSignSize = 0;
                    efh.dwMagic = ENCFILE_MAGIC;
                    efh.wDataOffset = sizeof(ENCFILE_HEADER);
                    efh.wSignLen = (WORD)nSignSize;
                    efh.dwSizeHigh = 0;
                    efh.dwSizeLow = nDataSize;
                    lpData = new BYTE[sizeof(ENCFILE_HEADER) + nDataSize + nSignSize];
                    CopyMemory(lpData, &efh, sizeof(ENCFILE_HEADER));
                    editor->writeToBuffer((LPBYTE)lpData + sizeof(ENCFILE_HEADER), nDataSize);

                    stego.stego((LPBYTE)lpData, sizeof(ENCFILE_HEADER) + nDataSize + nSignSize);
                    stego.save();

                    delete (LPBYTE)lpData;
                    stego.close();
                } else {
                    dwStatus = CommDlgExtendedError();
                    if (dwStatus != 0) {
                        _stprintf(lpszBuffer, TEXT("Save file dialog error.\nStatus code: %d"), dwStatus);
                        MessageBox(hWnd, lpszBuffer, MSG_TITLE, MB_OK | MB_ICONWARNING);
                    }
                }

                CryptDestroyHash(hMsgHash);
                CryptDestroyHash(hUserHash);
                CryptReleaseContext(hProv, 0);

                break;
            }

            case IDM_ITEMUNSTEGO:
            {
                HCRYPTPROV hProv;
                HCRYPTHASH hUserHash;
                HCRYPTKEY hAesKey;
                HCRYPTHASH hMsgHash;
                StegoContainer stego;
                LPVOID lpData = NULL;
                int nDataSize;
                int nSignSize;
                ENCFILE_HEADER efh = {0};

                if (! CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, 0)) {
                    HANDLE_ERROR("CryptAquireContext", GetLastError());
                    break;
                }

                if (! CryptCreateHash(hProv, CALG_SHA_256, NULL, 0, &hUserHash)) {
                    CryptReleaseContext(hProv, 0);
                    HANDLE_ERROR("CryptCreateHash", GetLastError());
                    break;
                }

                if (! CryptCreateHash(hProv, CALG_MD5, NULL, 0, &hMsgHash)) {
                    CryptReleaseContext(hProv, 0);
                    HANDLE_ERROR("CryptCreateHash", GetLastError());
                    break;
                }

                // Изъять данные из контейнера
                ofn.lpstrTitle = TEXT("Открыть файл контейнера");
                if (GetOpenFileName(&ofn)) {
                    stego.open(lpszFilename);
                    stego.unstego((LPBYTE)&efh, sizeof(ENCFILE_HEADER));
                    if (efh.dwMagic != ENCFILE_MAGIC) {
                        MessageBox(hWnd, TEXT("Контейнер не содержит стеганографических данных"),
                            MSG_TITLE, MB_OK | MB_ICONINFORMATION);
                    }
                    nSignSize = efh.wSignLen;
                    nDataSize = efh.dwSizeLow;
                    lpData = new BYTE[efh.wDataOffset + nDataSize + nSignSize];
                    stego.unstego((LPBYTE)lpData, efh.wDataOffset + nDataSize + nSignSize);
                    editor->readFromBuffer((LPBYTE)lpData + efh.wDataOffset, nDataSize);

                    delete (LPBYTE)lpData;
                    stego.close();
                } else {
                    dwStatus = CommDlgExtendedError();
                    if (dwStatus != 0) {
                        _stprintf(lpszBuffer, TEXT("Save file dialog error.\nStatus code: %d"), dwStatus);
                        MessageBox(hWnd, lpszBuffer, MSG_TITLE, MB_OK | MB_ICONWARNING);
                    }
                }

                CryptDestroyHash(hMsgHash);
                CryptDestroyHash(hUserHash);
                CryptReleaseContext(hProv, 0);

                break;
            }
        }
        break;
      case WM_DESTROY:
        delete lpszBuffer;
        delete lpszFilename;
        delete editor;
        PostQuitMessage(0);
        break;
      default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
