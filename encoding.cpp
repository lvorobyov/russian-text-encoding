//
// Created by Lev on 08.01.2019.
//

#include "encoding.h"
//#include <algorithm>
#include <array>
//#include <numeric>

using namespace std;

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
    int allCount = 0;
    array<int,8> counts{};
    int iYiCount = 0;
    for (int i=0; i<sz; i++) {
        if (lpData[i] <= 0x7F)
            continue;
        c = lpData[i];
        if (c >= 0xC2) {
            d = ((c-0xC0) >> 4);
            d = d? d+1 : 2;
            if (i < sz - d && lpData[i+d] == c)
                repeatCount ++;
            allCount ++;
        }
        counts[(c-0x80)>>4]++;
        if ((c&0xEE) == 0xA6)
            iYiCount ++;
    }
    // Распознание многобайтовой кодировки по частоте повторения
    // одинаковых символов через один байт
    if (repeatCount) {
        repeatRate = repeatCount / static_cast<float>(allCount);
        if (repeatRate > 0.1)
            return CP_UTF8;
    }
    // Распознаем однобайтовоую кодировку по частоте появления
    // в тексте символов кириллицы
    //allCount = accumulate(counts.begin(),counts.end(),0);
    counts[0] += counts[1];
    counts[2] += counts[6];
    counts[3] += counts[4];
    counts[4] += counts[5];
    counts[5] += counts[6];
    counts[6] += counts[7];
    if (counts[4] > counts[6])
        return (iYiCount)? CP_KOI8U : CP_KOI8R;
    if (counts[0] > counts[4] + counts[7])
        return 866;
    if (counts[3] > counts[4]
        && counts[5] > counts[6])
        return CP_ISO_5;
    return CP_ACP;
}
