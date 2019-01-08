//
// Created by Lev on 08.01.2019.
//

#ifndef LAB6_EXCEPT_H
#define LAB6_EXCEPT_H

#include <system_error>
using namespace std;

inline void raise_system_error(const char* msg) {
    throw system_error((int)GetLastError(),
                       system_category(), msg);
}

#endif //LAB6_EXCEPT_H
