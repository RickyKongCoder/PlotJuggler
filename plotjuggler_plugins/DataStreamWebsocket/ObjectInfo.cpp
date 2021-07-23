#include "ObjectInfo.h"
#include "QVector"
#include "bytesConversion.h"
#include "stdint.h"
#include "variant"
#include <PlotJuggler/plotdata.h>
#include <cstring>

#include <QDebug>
using namespace std;
QVector<uint8_t> ObjSize = {TYPE_SIZE_MAP(GET_SIZE)};
//#ifdef qDebug()
//#define debug_(var) qDebug() << var;
//#else
//#define debug_(var)
//#endif
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
    if (type == LOGING_THREAD) {
        log_message = new char[255];
    }
}
void ObjectInfo::setBytestoValue(char *array)
{
    if (type == LOGING_THREAD)
        std::strcpy(log_message, array);

    else
        memcpy(&value, array, size);

    //    if (size == 8) {
    //        memcpy(&value, array, 8);

    //        //    qBytes8Convert(array, &value);
    //    } else if (size == 4) {
    //        memcpy(&value, array, 4);

    //        //   qBytes4Convert(array, &value);
    //    } else if (size == 2) {
    //        memcpy(&value, array, 2);

    //        //  qBytes2Convert(array, &value);
    //    } else if (size == 1) {
    //        memcpy(&value, array, 1);
    //    } else if (type == XYTHETA_) {
    //        memcpy(&value, array, 12);

    //        //        qBytes4Convert(&value.xyt.x_pos, &value.xyt.x_pos);
    //        //        qBytes4Convert(&value.xyt.y_pos, &value.xyt.y_pos);
    //        //        qBytes4Convert(&value.xyt.theta, &value.xyt.theta);

    //        //        memcpy(&value, array, size);
    //    }
    //debug("the float value " << value.f)
    qDebug() << "the float value " << value.f;
    qDebug() << "the int8 value " << value.i8;
    qDebug() << "the int16 value " << value.i16;
    qDebug() << "the int32 value " << value.i8;
    qDebug() << "the doube value " << value.d;
    qDebug() << "the XYTheta x " << value.xyt.x_pos;
    qDebug() << "the XYTheta y " << value.xyt.y_pos;
    qDebug() << "the XYTheta theta " << value.xyt.theta;
    qDebug() << "the float value " << value.f << endl;
    qDebug() << "the float value " << value.f << endl;
    if (type == LOGING_THREAD)
        qDebug() << "const string message*" << log_message << endl;
}

double ObjectInfo::getValueInDouble()
{
    switch (type) {
    case FLOAT_:
        //   qDebug() << "it is float" << endl;
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
void ObjectInfo::addPlotifNotExist(PJ::PlotDataMapRef *datamap)
{
    if (this->isnumeric()) {
        this->addPlotNumeric(datamap);
    } else if (this->type == ENUMTYPE_) {
        this->addPlotEnum(datamap);
    } else if (this->type == LOGING_THREAD) {
        this->addPlotLoging(datamap);
    }
}

void ObjectInfo::addPlotEnum(PJ::PlotDataMapRef *datamap)
{
    auto &serial_strings_plots = datamap->strings;
    auto target_plotIt = serial_strings_plots.find(name.toStdString());
    qDebug() << "add Plot enum name" << name;

    if (target_plotIt == serial_strings_plots.end()) {
        datamap->addStringSeries(name.toStdString());
    }
}
void ObjectInfo::addPlotLoging(PJ::PlotDataMapRef *datamap)
{
    auto &serial_strings_plots = datamap->loging_message;
    auto target_plotIt = serial_strings_plots.find(name.toStdString());
    qDebug() << "add Plot loging name" << name;
    if (target_plotIt == serial_strings_plots.end()) {
        datamap->addLogingSeries(name.toStdString());
    }
}
void ObjectInfo::addPlotNumeric(PJ::PlotDataMapRef *datamap)
{
    switch (type) {
    case FLOAT_:
    case INT8_:
    case INT16_:
    case INT32_:
    case DOUBLE64_: {
        auto &serial_numeric_plots = datamap->numeric;
        auto target_plotIt = serial_numeric_plots.find(name.toStdString());
        if (target_plotIt == serial_numeric_plots.end()) {
            qDebug() << "name" << name;
        }
        datamap->addNumeric(name.toStdString());

        break;
    }
    case XYTHETA_:
        datamap->getOrCreateNumeric("/" + name.toStdString() + "/" + "x");
        datamap->getOrCreateNumeric("/" + name.toStdString() + "/" + "y");
        datamap->getOrCreateNumeric("/" + name.toStdString() + "/" + "theta");

        break;

    default:
        break;
    }
}
void ObjectInfo::setString(const char *str)
{
    ;
}
void ObjectInfo::updatePlotNumeric(PJ::PlotDataMapRef &datamap, double time, double param[])
{
    switch (type) {
    case FLOAT_:
    case INT8_:
    case INT16_:
    case INT32_:
    case DOUBLE64_:
        datamap.numeric.find(name.toStdString())
            ->second.pushBack({(double) time, this->getValueInDouble()});
        break;
    case XYTHETA_:

        if (sizeof(param) / sizeof(param[0]) == 0) {
            datamap.getOrCreateNumeric("/" + name.toStdString() + "/" + "x")
                .pushBack({time, this->value.xyt.x_pos});
            datamap.getOrCreateNumeric("/" + name.toStdString() + "/" + "y")
                .pushBack({time, this->value.xyt.y_pos});
            datamap.getOrCreateNumeric("/" + name.toStdString() + "/" + "theta")
                .pushBack({time, this->value.xyt.theta});
        }
        //        qDebug() << "x" << ((XYTheta *) this->getValueRef())->x_pos << endl;
        //        qDebug() << "y" << ((XYTheta *) this->getValueRef())->y_pos << endl;
        //        qDebug() << "theta" << ((XYTheta *) this->getValueRef())->theta << endl;
        //        double xyt[] = {double(((XYTheta *) this->getValueRef())->x_pos),
        //                        double(((XYTheta *) this->getValueRef())->y_pos),
        //                        double(((XYTheta *) this->getValueRef())->theta)};
        break;

    default:
        break;
    }
}

//string yeeeh = "yeeh";
void ObjectInfo::updatePlotEnum(PJ::PlotDataMapRef &datamap, double time)
{
    //    yeeeh += "FUCK";
    //    qDebug() << "GET STRING"
    //             << QString::fromStdString(ptr->enum_Map.find(this->value.enumid)->second.toStdString());
    //datamap.strings.find(name.toStdString())->second.pushBack({time, });
    datamap.strings.find(name.toStdString())
        ->second.pushBack({time, ptr->enum_Map.find(this->value.enumid)->second.toStdString()});
}
void ObjectInfo::showallinloging(PJ::PlotDataMapRef &datamap)
{
    for (auto &ptr : datamap.loging_message) {
        for (auto &str : ptr.second) {
            qDebug() << str.y.data() << ",";
        }
    }
}
void ObjectInfo::updatePlotLoging(PJ::PlotDataMapRef &datamap, double time)
{
    datamap.loging_message.find(name.toStdString())->second.pushBack({time, log_message});

    return;
}
void ObjectInfo::updatePlot(PJ::PlotDataMapRef &datamap, double time, double param[])
{
    if (this->isnumeric()) {
        updatePlotNumeric(datamap, time, param);
    } else if (this->type == ENUMTYPE_) {
        updatePlotEnum(datamap, time);
    } else if (this->type = LOGING_THREAD) {
        updatePlotLoging(datamap, time);
    }
}
uint8_t ObjectInfo::getId()
{
    return id;
}
