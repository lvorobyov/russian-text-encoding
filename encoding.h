//
// Created by Lev on 08.01.2019.
//

#ifndef LAB6_ENCODING_H
#define LAB6_ENCODING_H

#include <windows.h>

/**
 * Эвристичесоке определение кодировки текста
 * @param lpData бинарное представление текста
 * @param sz кол-во байтов в адресуемом блоке
 * @return номер кодовой страницы
 */
UINT RecogniseEncoding(LPCBYTE lpData, size_t sz);

#endif //LAB6_ENCODING_H
