//
// Created by Lev on 09.01.2019.
//

#ifndef LAB6_DEBUG_H
#define LAB6_DEBUG_H

#include <cstdio>
#include <cassert>
#include <csignal>
#include "stego.h"

class StegoDebug : public StegoContainer {
private:
    FILE *f;
public:
    StegoDebug() {
        f = fopen("stego.bin", "wb");
        signal(SIGABRT, [](int) { throw runtime_error("Assertion failed"); });
    }
    ~StegoDebug() {
        signal(SIGABRT, SIG_DFL);
        fclose(f);
    }

    int stego(LPBYTE lpbData, int nDataLen) override {
        fwrite(lpbData, sizeof(BYTE), nDataLen, f);
        int nResult = StegoContainer::stego(lpbData, nDataLen);
        assert(nResult == nDataLen);
        return nResult;
    }

    int unstego(LPBYTE lpbData, int nDataMaxLen) override {
        int nDataLen = StegoContainer::unstego(lpbData, nDataMaxLen);
        if (nDataLen > sizeof(ENCFILE_HEADER))
            fwrite(lpbData, sizeof(BYTE), nDataLen, f);
        return nDataLen;
    }
};

#endif //LAB6_DEBUG_H
