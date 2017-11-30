/**
 * @Author: Lev Vorobjev
 * @Date:   30.11.2017
 * @Email:  lev.vorobjev@rambler.ru
 * @Filename: dialog.cpp
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 30.11.2017
 * @License: MIT
 * @Copyright: Copyright (c) 2017 Lev Vorobjev
 */

#include <windows.h>
#include <windowsx.h>
#include "dialog.h"
#include "resource.h"

#define MSG_TITLE TEXT("Lab6")

BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

BOOL GetUserPassword(LPPASSWORD_DLG lppd) {
    return DialogBoxParam(lppd->hInstance,
        MAKEINTRESOURCE(IDD_PASSWORDDLG),
        lppd->hwndOwner,
        (DLGPROC) DlgProc,
        (LPARAM) lppd
    ) == IDOK;
}

BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static LPPASSWORD_DLG lppd;
    static LPTSTR lpszPassword;
    static UINT cchPassword;
    switch (message) {
        case WM_INITDIALOG:
            lppd = (LPPASSWORD_DLG) lParam;
            lpszPassword = lppd->lpszPassword;
            SetDlgItemText(hDlg, IDC_EDITPASS, lpszPassword);
            if (lppd->lpszTitle != NULL) {
                SetWindowText(hDlg, lppd->lpszTitle);
            }
            SetDlgItemText(hDlg, IDC_LBLQUERY, lppd->lpszQuery);
            SetFocus(GetDlgItem(hDlg, IDC_EDITPASS));
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    cchPassword = GetDlgItemText(hDlg, IDC_EDITPASS,
                        lpszPassword, lppd->dwMaxPassword);
                    if (cchPassword == 0) {
                        MessageBox(hDlg, TEXT("Текстовое поле не заполнено"),
                            MSG_TITLE, MB_OK | MB_ICONWARNING);
                        return TRUE;
                    }

                case IDCANCEL:
                    EndDialog(hDlg, LOWORD(wParam));
                    return TRUE;
            }
    }
    return FALSE;
}
