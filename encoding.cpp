//
// Created by Lev on 08.01.2019.
//

#include "encoding.h"

UINT RecogniseEncoding(LPCBYTE lpData, size_t sz) {
    if (sz >= sizeof(DWORD)) {
        DWORD bom = *reinterpret_cast<const DWORD *>(lpData);
        if ((bom & (~0U>>8)) == 0xBFBBEF)
            return CP_UTF8;
        if ((bom & 0xE8FFFFFF) == 0x28762F2B)
            return CP_UTF7;
    }
    static const UINT CP_KOI8R = 20866;
    static const UINT CP_KOI8U = 21866;
    static const UINT CP_ISO_5 = 28595;
    BYTE c; int d;
    float repeatRate = 1.0;
    int repeatCount = 0;
    for (int i=0; i<sz; i++) {
        if (lpData[i] <= 0x7F)
            continue;
        c = lpData[i];
        if (c >= 0xC2) {
            d = ((c-0xC0) >> 4);
            d = d? d+1 : 2;
            if (i < sz - d && lpData[i+d] == c) {
                repeatCount ++;
            }
        }
    }
    // Распознание многобайтовой кодировки по частоте повторения
    // одинаковых символов через один байт
    if (repeatCount) {
        repeatRate = repeatCount / static_cast<float>(sz-2);
        if (repeatRate > 0.1)
            return CP_UTF8;
    }


    return CP_ACP;
}
