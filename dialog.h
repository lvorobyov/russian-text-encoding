/**
 * @Author: Lev Vorobjev
 * @Date:   30.11.2017
 * @Email:  lev.vorobjev@rambler.ru
 * @Filename: dialog.h
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 30.11.2017
 * @License: MIT
 * @Copyright: Copyright (c) 2017 Lev Vorobjev
 */

#include <windef.h>

typedef struct _PASSWORD_DLG {
    DWORD lStructSize;
    HWND hwndOwner;
    HINSTANCE hInstance;
    LPTSTR lpszTitle;
    LPTSTR lpszQuery;
    LPTSTR lpszPassword;
    DWORD dwMaxPassword;
} PASSWORD_DLG, *LPPASSWORD_DLG;

BOOL GetUserPassword(LPPASSWORD_DLG lppd);
