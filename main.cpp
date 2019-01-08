/**
 * main.cpp
 *
 * Created on: 25.11.2017
 *     Author: Lev Vorobjev
 *      Email: lev.vorobjev@rambler.ru
 *
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 01.12.2017
 * @License: MIT
 * @Copyright: Copyright (c) 2017 Lev Vorobjev
 */

#include <windows.h>
#include <wincrypt.h>
#include "editor.h"
#include "stego.h"
#include "crypt_tools.h"
#include "resource.h"
#include "dialog.h"
#include "except.h"

#define IDC_EDITTEXT  40050

#define WND_TITLE TEXT("Стеганография")
#define MSG_TITLE TEXT("Lab6")
#define BUFFER_SIZE 512

#define HANDLE_ERROR(lpszFunctionName, dwStatus) \
    MultiByteToWideChar(CP_ACP, 0, \
        lpszFunctionName, -1, lpszBuffer, BUFFER_SIZE); \
    _stprintf(lpszBuffer, TEXT("%s error.\nStatus code: %d"), \
        lpszBuffer, dwStatus); \
    MessageBox(hWnd, lpszBuffer, MSG_TITLE, MB_OK | MB_ICONWARNING);

#ifdef _DEBUG
#define DEBUG_INFO(editor) \
    _stprintf(lpszBuffer, TEXT("TextLength: %d\nStatus: %s\nText: %s"), \
        editor->getTextLength(), \
        editor->getModify() ? TEXT("MODIFIED") : TEXT("NOT MODIFIED"), \
        editor->getText()); \
    MessageBox(hWnd, lpszBuffer, MSG_TITLE, MB_OK | MB_ICONINFORMATION);

#define DEBUG_DUMP(lpData, dwLenght) \
    for (int i = 0; i < dwLenght; i++) { \
        _stprintf(lpszBuffer + 3 * i, (i % 16 == 15) ? TEXT("%02X\n") : TEXT("%02X "), ((LPBYTE)lpData)[i]); \
    } \
    MessageBox(hWnd, lpszBuffer, MSG_TITLE, MB_OK | MB_ICONINFORMATION);
#else
#define DEBUG_INFO(editor)
#define DEBUG_DUMP(lpData, dwLenght)
#endif

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
    static PASSWORD_DLG pdlg = {0};
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
        ofn.lpstrFilter = TEXT("Все файлы\0*.*\0Текстовые файлы\0*.txt\0Точечные рисунки\0*.bmp\0");
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
        // Заполнение стуктуры диалога ввода пароля
        pdlg.lStructSize = sizeof(PASSWORD_DLG);
        pdlg.hInstance = hInst;
        pdlg.hwndOwner = hWnd;
        pdlg.lpszPassword = lpszBuffer;
        pdlg.dwMaxPassword = BUFFER_SIZE;
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
                ofn.nFilterIndex = 2;
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
                ofn.nFilterIndex = 2;
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
                HCRYPTKEY hAesKey;
                StegoContainer stego;
                LPVOID lpData = NULL;
                int nDataSize;
                int nHashSize;
                ENCFILE_HEADER efh = {0};
                ENCFILE_HEADER* lpEfh;
                LPBYTE lpbData;
                LPBYTE lpbHash;
                LPTSTR lpszPassword = NULL;
                INT nPasswordLen;
                DWORD dwEncryptLen;

                if (! CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
                    HANDLE_ERROR("CryptAquireContext", GetLastError());
                    break;
                }

                // Встроить данные в контейнер
                ofn.lpstrTitle = TEXT("Открыть файл контейнера");
                ofn.nFilterIndex = 3;
                if (GetOpenFileName(&ofn)) {
                    pdlg.lpszTitle = NULL;
                    pdlg.lpszPassword = lpszBuffer;
                    pdlg.lpszQuery = TEXT("Пожалуйста, введите пароль для шифрования данных:");
                    if (! GetUserPassword(&pdlg)) {
                        CryptReleaseContext(hProv, 0);
                        break;
                    }

                    nPasswordLen = _tcslen(lpszBuffer);
                    lpszPassword = new TCHAR[nPasswordLen + 1];
                    pdlg.lpszTitle = TEXT("Повтор пароля");
                    pdlg.lpszPassword = lpszPassword;
                    pdlg.lpszQuery = TEXT("Повторите ввод пароля:");
                    if (! GetUserPassword(&pdlg)) {
                        CryptReleaseContext(hProv, 0);
                        delete lpszPassword;
                        break;
                    }

                    if (_tcscmp(lpszBuffer, lpszPassword) != 0) {
                        MessageBox(hWnd, TEXT("Пароли не совпадают"),
                            MSG_TITLE, MB_OK | MB_ICONWARNING);
                        CryptReleaseContext(hProv, 0);
                        delete lpszPassword;
                        break;
                    }

                    try {
                        stego.open(lpszFilename);
                        nDataSize = editor->writeToBuffer(NULL, 0);
                        nHashSize = ComputeMD5Hash(hProv, NULL, 0, NULL);
                        efh.dwMagic = ENCFILE_MAGIC;
                        efh.wDataOffset = sizeof(ENCFILE_HEADER);
                        efh.wSignLen = (WORD)nHashSize;
                        efh.dwSizeHigh = 0;
                        efh.dwSizeLow = nDataSize;
                        dwEncryptLen = sizeof(ENCFILE_HEADER) + nDataSize + nHashSize;
                        lpData = new BYTE[dwEncryptLen + 16];
                        ZeroMemory(lpData, dwEncryptLen + 16);
                        lpEfh = (ENCFILE_HEADER*) lpData;
                        lpbData = (LPBYTE)lpEfh + sizeof(ENCFILE_HEADER);
                        lpbHash = lpbData + nDataSize;
                        CopyMemory(lpEfh, &efh, sizeof(ENCFILE_HEADER));
                        editor->writeToBuffer(lpbData, nDataSize);
                        ComputeMD5Hash(hProv, lpbData, nDataSize, lpbHash);

                        DEBUG_DUMP(lpData, dwEncryptLen)

                        hAesKey = PasswordToAesKey(hProv, lpszPassword, nPasswordLen);
                        if (! CryptEncrypt(hAesKey, 0, TRUE, 0, (BYTE*)lpData, &dwEncryptLen, dwEncryptLen + 16)) {
                            raise_system_error("CryptEncrypt");
                        }
                        stego.stego((LPBYTE)lpData, dwEncryptLen);
                        stego.save();
                        CryptDestroyKey(hAesKey);
                        MessageBox(hWnd, TEXT("Данные внедрены в BMP"),
                            MSG_TITLE, MB_OK | MB_ICONINFORMATION);

                    } catch (std::system_error& ex) {
                        HANDLE_ERROR(ex.what(), ex.code())
                    } catch (std::exception& ex) {
                        HANDLE_ERROR(ex.what(), 0)
                    }
                    delete lpszPassword;
                    delete (LPBYTE)lpData;
                    stego.close();
                } else {
                    dwStatus = CommDlgExtendedError();
                    if (dwStatus != 0) {
                        _stprintf(lpszBuffer, TEXT("Save file dialog error.\nStatus code: %d"), dwStatus);
                        MessageBox(hWnd, lpszBuffer, MSG_TITLE, MB_OK | MB_ICONWARNING);
                    }
                }

                CryptReleaseContext(hProv, 0);

                break;
            }

            case IDM_ITEMUNSTEGO:
            {
                HCRYPTPROV hProv;
                HCRYPTKEY hAesKey;
                StegoContainer stego;
                LPVOID lpData = NULL;
                int nDataSize;
                int nHashSize;
                ENCFILE_HEADER efh = {0};
                ENCFILE_HEADER* lpEfh;
                LPBYTE lpbData;
                LPBYTE lpbHash;
                LPBYTE lpbCheckHash;
                DWORD dwEncryptLen;

                if (! CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
                    HANDLE_ERROR("CryptAquireContext", GetLastError());
                    break;
                }

                // Изъять данные из контейнера
                ofn.lpstrTitle = TEXT("Открыть файл контейнера");
                ofn.nFilterIndex = 3;
                if (GetOpenFileName(&ofn)) {
                    pdlg.lpszTitle = NULL;
                    pdlg.lpszPassword = lpszBuffer;
                    pdlg.lpszQuery = TEXT("Пожалуйста, введите пароль для расшифровки данных:");
                    if (! GetUserPassword(&pdlg)) {
                        CryptReleaseContext(hProv, 0);
                        break;
                    }

                    try {
                        hAesKey = PasswordToAesKey(hProv, lpszBuffer, _tcslen(lpszBuffer));
                        stego.open(lpszFilename);
                        stego.unstego((LPBYTE)&efh, sizeof(ENCFILE_HEADER));

                        dwEncryptLen = sizeof(ENCFILE_HEADER);
                        // Расшифровать заголовок
                        if (! CryptDecrypt(hAesKey, 0, FALSE, 0, (LPBYTE)&efh, &dwEncryptLen)) {
                            raise_system_error("CryptDecrypt");
                        }

                        if (efh.dwMagic != ENCFILE_MAGIC) {
                            MessageBox(hWnd, TEXT("Контейнер не содержит стеганографических данных"),
                                MSG_TITLE, MB_OK | MB_ICONINFORMATION);
                            CryptDestroyKey(hAesKey);
                            CryptReleaseContext(hProv, 0);
                            break;
                        }
                        nHashSize = efh.wSignLen;
                        nDataSize = efh.dwSizeLow;
                        dwEncryptLen = efh.wDataOffset + nDataSize + nHashSize;
                        lpData = new BYTE[dwEncryptLen + 16];
                        ZeroMemory(lpData, dwEncryptLen + 16);
                        lpbCheckHash = new BYTE[nHashSize];
                        lpEfh = (ENCFILE_HEADER*) lpData;
                        lpbData = (LPBYTE)lpEfh + efh.wDataOffset;
                        lpbHash = lpbData + nDataSize;
                        dwEncryptLen = ((dwEncryptLen + 15) / 16) * 16;
                        stego.unstego((LPBYTE)lpData, dwEncryptLen);

                        // Расшифровать данные
                        if (! CryptDecrypt(hAesKey, 0, TRUE, 0, (LPBYTE)lpData, &dwEncryptLen)) {
                            raise_system_error("CryptDecrypt");
                        }

                        ComputeMD5Hash(hProv, lpbData, nDataSize, lpbCheckHash);

                        DEBUG_DUMP(lpData, dwEncryptLen)
                        DEBUG_DUMP(lpbCheckHash, nHashSize)

                        if (memcmp(lpbHash, lpbCheckHash, nHashSize) != 0) {
                            MessageBox(hWnd, TEXT("Стеганографические данные были искажены"),
                                MSG_TITLE, MB_OK | MB_ICONINFORMATION);
                        } else {
                            editor->readFromBuffer(lpbData, nDataSize);
                        }

                    } catch (std::system_error& ex) {
                        HANDLE_ERROR(ex.what(), ex.code().value())
                    } catch (std::exception& ex) {
                        HANDLE_ERROR(ex.what(), 0)
                    }
                    CryptDestroyKey(hAesKey);
                    delete lpbCheckHash;
                    delete (LPBYTE)lpData;
                    stego.close();
                } else {
                    dwStatus = CommDlgExtendedError();
                    if (dwStatus != 0) {
                        _stprintf(lpszBuffer, TEXT("Save file dialog error.\nStatus code: %d"), dwStatus);
                        MessageBox(hWnd, lpszBuffer, MSG_TITLE, MB_OK | MB_ICONWARNING);
                    }
                }

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
