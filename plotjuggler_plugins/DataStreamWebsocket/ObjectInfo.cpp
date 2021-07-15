#include "ObjectInfo.h"
#include "QVector"
#include "bytesConversion.h"
#include "stdint.h"
#include "variant"
QVector<uint8_t> ObjSize = {TYPE_SIZE_MAP(GET_SIZE)};
ObjectType ObjectInfo::getType()
{
    return type;
}

uint8_t ObjectInfo::typeToSize(ObjectType type)
{
    return (size = ObjSize.at((uint8_t) type));
}
void ObjectInfo::setTypeMem(ObjectType type)
{
    this->type = type;
}
void ObjectInfo::setBytestoValue(char *array)
{
    if (size == 4) {
        qBytes4Convert(array, &value);
    } else if (size == 2) {
        qBytes2Convert(array, &value);
    }
    qDebug() << "the float value " << value.f << endl;
}

double ObjectInfo::getValueInDouble()
{
    switch (type) {
    case FLOAT_:
        qDebug() << "it is float" << endl;
        return (double) (this->value.f);
    case INT8_:
        return this->value.i8;
    case INT16_:
        return this->value.i16;
    case INT32_:
        return this->value.i32;
    case DOUBLE64_:
        return this->value.d;
        break;
    case XYTHETA_:
        return 0;
        //  return this->value.xyt;
    case ENUMTYPE_:
        return 0;
    default:
        return 0;
        break;
    }
    return 0;
}

void ObjectInfo::setName(QString name)
{
    this->name = name;
}

QString ObjectInfo::getName() const
{
    return name;
}

void ObjectInfo::setId(uint8_t id)
{
    this->id = id;
}
uint8_t ObjectInfo::getId()
{
    return id;
}
