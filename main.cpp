/**
 * main.cpp
 *
 * Created on: 25.11.2017
 *     Author: Lev Vorobjev
 *      Email: lev.vorobjev@rambler.ru
 *
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 25.11.2017
 * @License: MIT
 * @Copyright: Copyright (c) 2017 Lev Vorobjev
 */

#include <windows.h>
#include "resource.h"

#define WND_TITLE TEXT("Стеганография")
#define MSG_TITLE TEXT("Lab6")
#define BUFFER_SIZE 512

#define HANDLE_ERROR(lpszFunctionName, dwStatus) \
    _stprintf(lpszBuffer, TEXT("%s error.\nStatus code: %d"), \
        lpszFunctionName, dwStatus); \
    MessageBox(hWnd, lpszBuffer, MSG_TITLE, MB_OK | MB_ICONWARNING);

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

    switch (message) {
      case WM_CREATE:
        hInst = (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
        break;
      case WM_DESTROY:
        PostQuitMessage(0);
        break;
      default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
