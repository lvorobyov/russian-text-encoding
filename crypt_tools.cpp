/**
 * @Author: Lev Vorobjev
 * @Date:   30.11.2017
 * @Email:  lev.vorobjev@rambler.ru
 * @Filename: crypt_tools.cpp
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 30.11.2017
 * @License: MIT
 * @Copyright: Copyright (c) 2017 Lev Vorobjev
 */

#include "crypt_tools.h"
#include <win32/win32_error.h>

int ComputeMD5Hash(HCRYPTPROV hProv, CONST BYTE *pbData, DWORD dwDataLen, LPBYTE lpbHash) {
    HCRYPTHASH hHash;
    DWORD cbHashSize = 0;
    DWORD dwCount = sizeof(DWORD);;

    if (! CryptCreateHash(hProv, CALG_MD5, NULL, 0, &hHash)) {
        throw win32::win32_error("CryptCreateHash");
    }

    if (lpbHash == NULL) {
        if (! CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*) &cbHashSize, &dwCount, 0)) {
            CryptDestroyHash(hHash);
            throw win32::win32_error("CryptGetHashParam");
        }
        return cbHashSize;
    }

    if (! CryptHashData(hHash, pbData, dwDataLen, 0)) {
        CryptDestroyHash(hHash);
        throw win32::win32_error("CryptHashData");
    }

    if (! CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*) &cbHashSize, &dwCount, 0)) {
        CryptDestroyHash(hHash);
        throw win32::win32_error("CryptGetHashParam");
    }

    if (! CryptGetHashParam(hHash, HP_HASHVAL, lpbHash, &cbHashSize, 0)) {
        CryptDestroyHash(hHash);
        throw win32::win32_error("CryptGetHashParam");
    }

    CryptDestroyHash(hHash);

    return cbHashSize;
}

void PasswordToAesKey(HCRYPTPROV hProv, LPTSTR lpszPassword, DWORD dwPasswordLen, HCRYPTKEY *phKey) {
    HCRYPTHASH hHash;

    if (! CryptCreateHash(hProv, CALG_SHA_256, NULL, 0, &hHash)) {
        throw win32::win32_error("CryptCreateHash");
    }

    if (! CryptHashData(hHash, (BYTE*) lpszPassword, dwPasswordLen * sizeof(TCHAR), 0)) {
        CryptDestroyHash(hHash);
        throw win32::win32_error("CryptHashData");
    }

    if (! CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, phKey)) {
        throw win32::win32_error("CryptDeriveKey");
    }

    // Установить требуемый режим шифрования: CBC
    const DWORD dwMode = CRYPT_MODE_CBC;
    CryptSetKeyParam(*phKey, KP_MODE, (LPBYTE)&dwMode, 0);

    CryptDestroyHash(hHash);
}
