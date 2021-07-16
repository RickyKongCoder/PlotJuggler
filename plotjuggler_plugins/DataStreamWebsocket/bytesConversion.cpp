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
void qBytes8Convert(void *bytes, void *f)
{
    //it works for float so it should workd for other
    if (is_big_endian()) {
        memcpy(f, &bytes, 8);
    } else {
        uint64_t bytes8 = 0;
        memcpy(&bytes8, bytes, 8);
        //qDebug()<<"bytes8 real:"<<hex<<bytes8<<endl;
        uint64_t num = ((bytes8 & 0xFF00000000000000) >> 56) + ((bytes8 & 0x00FF000000000000) >> 40)
                       + ((bytes8 & 0x0000FF0000000000) >> 24)
                       + ((bytes8 & 0x000000FF00000000) >> 8) + ((bytes8 & 0x00000000FF000000) << 8)
                       + ((bytes8 & 0x0000000000FF0000) << 24)
                       + ((bytes8 & 0x000000000000FF00) << 40)
                       + ((bytes8 & 0x00000000000000FF) << 56);
        qDebug() << "bytes8 7 " << ((bytes8 & 0xFF00000000000000) >> 56);

        qDebug() << "bytes8 6 " << (bytes8 & 0x00FF000000000000) << " | "
                 << ((bytes8 & 0x00FF000000000000) >> 40);
        qDebug() << "bytes8 5 " << ((bytes8 & 0x0000FF0000000000) >> 24);
        qDebug() << "bytes8 4 " << ((bytes8 & 0x000000FF00000000) >> 8);
        qDebug() << "bytes8 3 " << ((bytes8 & 0x00000000FF000000) << 8);
        qDebug() << "bytes8 2 " << ((bytes8 & 0x0000000000FF0000) << 24);
        qDebug() << "bytes8 1 " << ((bytes8 & 0x000000000000FF00) << 40);
        qDebug() << "bytes8 0 " << ((bytes8 & 0x00000000000000FF) << 56);
        memcpy(f, &num, 8);
        qDebug() << "final double" << *(double *) f;
    }
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
        uint16_t bytes2 = 0;
        memcpy(&bytes2, bytes, 2);
        qDebug() << hex;
        //qDebug()<<"bytes4 real:"<<hex<<bytes4<<endl;
        uint16_t num = ((bytes2 & 0xFF00) >> 8) + ((bytes2 & 0x00FF) << 8);
        qDebug() << "bytes2 1 " << ((bytes2 & 0xFF00) >> 8) << endl;
        qDebug() << "bytes2 0 " << ((bytes2 & 0x00FF) << 8) << endl;
        memcpy(f, &num, 2);
    }
}
