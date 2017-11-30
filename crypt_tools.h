/**
 * @Author: Lev Vorobjev
 * @Date:   30.11.2017
 * @Email:  lev.vorobjev@rambler.ru
 * @Filename: crypt_tools.h
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 30.11.2017
 * @License: MIT
 * @Copyright: Copyright (c) 2017 Lev Vorobjev
 */

#include <encfile.h>

/**
 * ComputeMD5Hash()
 * Вычисляет MD5 хеш
 * @param hProv дескриптор криптопровайдера, полученный функцией {@link CryptAcquireContext()}
 * @param pbData байтовый массив хешируемых данных
 * @param dwDataLen количество байтов для хеширования
 * @param lpbHash выходной бфйтовый массив для значения хеша
 * @return количество байтов, записанных в строку хеша
 */
int ComputeMD5Hash(HCRYPTPROV hProv, CONST BYTE *pbData, DWORD dwDataLen, LPBYTE lpbHash);

/**
 * PasswordToAesKey()
 * Создает ключ для AES шифрования по пользовательскому сообщению
 * @param hProv дескриптор криптопровайдера, полученный функцией {@link CryptAcquireContext()}
 * @param lpszPassword пользовательский пароль
 * @param dwPasswordLen длина пароля
 * @param phKey дескриптор полученного в результате преобразования ключа шифрования
 */
void PasswordToAesKey(HCRYPTPROV hProv, LPTSTR lpszPassword, DWORD dwPasswordLen, HCRYPTKEY *phKey);
