#include "ObjectInfo.h"
#include "QVector"
#include "bytesConversion.h"
#include "stdint.h"
#include "variant"
#include <PlotJuggler/plotdata.h>
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
    if (size == 8) {
        qBytes8Convert(array, &value);
    } else if (size == 4) {
        qBytes4Convert(array, &value);
    } else if (size == 2) {
        qBytes2Convert(array, &value);
    } else if (size == 1) {
        memcpy(&value, array, 1);
    } else if (type == XYTHETA_) {
        memcpy(&value, array, 12);
        qBytes4Convert(&value.xyt.x_pos, &value.xyt.x_pos);
        qBytes4Convert(&value.xyt.y_pos, &value.xyt.y_pos);
        qBytes4Convert(&value.xyt.theta, &value.xyt.theta);

        //        memcpy(&value, array, size);
    }
    qDebug() << "the float value " << value.f;
    qDebug() << "the int8 value " << value.i8;
    qDebug() << "the int16 value " << value.i16;
    qDebug() << "the int32 value " << value.i8;
    qDebug() << "the doube value " << value.d;
    qDebug() << "the XYTheta x " << value.xyt.x_pos;
    qDebug() << "the XYTheta y " << value.xyt.y_pos;
    qDebug() << "the XYTheta theta " << value.xyt.theta;
    //    qDebug() << "the float value " << value.f << endl;
    //    qDebug() << "the float value " << value.f << endl;
}

double ObjectInfo::getValueInDouble()
{
    switch (type) {
    case FLOAT_:
        qDebug() << "it is float" << endl;
        return (double) (this->value.f);
    case INT8_:
        return (double) this->value.i8;
    case INT16_:
        return (double) this->value.i16;
    case INT32_:
        return (double) this->value.i32;
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
void ObjectInfo::addPlotNumeric(PJ::PlotDataMapRef *datamap)
{
    switch (type) {
    case XYTHETA_:
        datamap->getOrCreateNumeric("/" + name.toStdString() + "/" + "x");
        datamap->getOrCreateNumeric("/" + name.toStdString() + "/" + "y");
        datamap->getOrCreateNumeric("/" + name.toStdString() + "/" + "theta");

        break;
    default:
        break;
    }
}
void ObjectInfo::updatePlotNumeric(PJ::PlotDataMapRef *datamap, double param[], double time)
{
    switch (type) {
    case XYTHETA_:
        datamap->getOrCreateNumeric("/" + name.toStdString() + "/" + "x").pushBack({time, param[0]});
        datamap->getOrCreateNumeric("/" + name.toStdString() + "/" + "y").pushBack({time, param[1]});
        datamap->getOrCreateNumeric("/" + name.toStdString() + "/" + "theta")
            .pushBack({time, param[2]});
        break;
    }
}

uint8_t ObjectInfo::getId()
{
    return id;
}
