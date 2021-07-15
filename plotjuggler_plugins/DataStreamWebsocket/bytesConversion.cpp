#include "bytesConversion.h"
#include <QDebug>
//Assume from jetson, it is big endian so, need to process it in socket or ... to make sure it is big endian
bool is_big_endian(void)
{
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}

void qBytes4Convert(void *bytes, void *f)
{
    //it works for float so it should workd for other
    if (is_big_endian()) {
        memcpy(f, &bytes, 4);
    } else {
        uint32_t bytes4 = 0;
        memcpy(&bytes4, bytes, 4);
        //qDebug()<<"bytes4 real:"<<hex<<bytes4<<endl;
        uint32_t num = ((bytes4 & 0xFF000000) >> 24) + ((bytes4 & 0x00FF0000) >> 8)
                       + ((bytes4 & 0x0000FF00) << 8) + ((bytes4 & 0x000000FF) << 24);
        qDebug() << "bytes4 3 " << ((bytes4 & 0xFF000000) >> 24);

        qDebug() << "bytes4 2 " << (bytes4 & 0x00FF0000) << " | " << ((bytes4 & 0x00FF0000) >> 8);
        qDebug() << "bytes4 1 " << ((bytes4 & 0x0000FF00) << 8);
        qDebug() << "bytes4 0 " << ((bytes4 & 0x000000FF) << 24);
        memcpy(f, &num, 4);
        qDebug() << "final float" << *(float *) f;
    }
}
void qBytes2Convert(void *bytes, void *f)
{
    //it works for float so it should workd for other
    if (is_big_endian()) {
        memcpy(f, &bytes, 2);
    } else {
        uint32_t bytes4 = 0;
        memcpy(&bytes4, bytes, 2);
        qDebug() << hex;
        //qDebug()<<"bytes4 real:"<<hex<<bytes4<<endl;
        uint32_t num = ((bytes4 & 0xFF000000) >> 24) + ((bytes4 & 0x00FF0000) >> 8)
                       + ((bytes4 & 0x0000FF00) << 8) + ((bytes4 & 0x000000FF) << 24);
        qDebug() << "bytes2 1 " << ((bytes4 & 0x00FF) >> 8) << endl;
        qDebug() << "bytes2 0 " << ((bytes4 & 0xFF00) << 8) << endl;
        memcpy(f, &num, 2);
    }
}
