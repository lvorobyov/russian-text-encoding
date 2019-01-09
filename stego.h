/**
 * stego.h
 *
 * Created on: 28.11.2017
 *     Author: Lev Vorobjev
 *      Email: lev.vorobjev@rambler.ru
 *
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 28.11.2017
 * @License: MIT
 * @Copyright: Copyright (c) 2017 Lev Vorobjev
 */

#pragma once
#include <windows.h>

/**
 * StegoContainer
 * Класс представляет абстрактный контейнер,
 * способный хранить стеганографическую информацию
 */
class StegoContainer {
private:
    /**
     * _hFile
     * Дескриптор файла контейнера
     */
    HANDLE _hFile = NULL;

    /**
     * _hglbStego
     * Дексриптор глобального блока данных, содержащего контейнер
     */
    HGLOBAL _hglbStego = NULL;

    /**
     * Количество битов шифротекста на один байт контейнера
     */
    static const int N = 1;

public:
    /**
     * open(fileName)
     * Открыть файл стеганографического контейнера
     * @param lpszFilename имя файла стеганографического контейнера
     */
    void open(LPCTSTR lpszFilename);

    /**
     * save(fileName)
     * Сохранить стеганографический контейнер в файле
     * @param lpszFilename имя файла стеганографического контейнера
     */
    void save(LPCTSTR lpszFilename);

    /**
     * save()
     * Сохранить стеганографический контейнер в открытом файле
     */
    void save();

    /**
     * close()
     * Закрыть файл стеганографического контейнера
     */
    void close();

    /**
     * stego()
     * Поместить данные в стеганографический контейнер
     * @param lpbData байтовый массив стеганографических данных
     * @param nDataLen количество байтов для встраивания в контейнер
     * @return количество помещенных в контейнер байтов
     */
    virtual int stego(LPBYTE lpbData, int nDataLen);

    /**
     * unstego()
     * Получить данные из стеганографического контейнера
     * @param lpbData байтовый массив стеганографических данных
     * @param nDataLen количество байтов для встраивания в контейнер
     * @return количество полученных из контейнера байтов
     */
    virtual int unstego(LPBYTE lpbData, int nDataMaxLen);

};
