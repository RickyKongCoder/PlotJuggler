#ifndef BYTESCONVERSION_H
#define BYTESCONVERSION_H

#include <cstdint>
#include <cstdlib>

#include <iostream>
#include <QDebug>

bool is_big_endian();
//template<typename T>
//T qBytes4Convert(void *bytes, T &f);
//template<typename T>
//T qBytes2Convert(void *bytes, T &f);
void qNbytesConvert(void *bytes, void *f, int n);

void qBytes8Convert(void *bytes, void *f);
void qBytes4Convert(void *bytes, void *f);
void qBytes2Convert(void *bytes, void *f);

#endif // BYTESCONVERSION_H
