#ifndef OBJECTINFO_H
#define OBJECTINFO_H
#include <Plotjuggler/plotdata.h>
#include <iostream>
#include <iterator>
#include <map>
#include <variant>
#include <QObject>
using namespace std;
#define TYPE_SIZE_MAP(X) \
    X(FLOAT_, 4) \
    X(INT8_, 1) \
    X(INT16_, 2) \
    X(INT32_, 4) \
    X(DOUBLE64_, 8) \
    X(XYTHETA_, 12) \
    X(ENUMTYPE_, 1) \
    X(CUSTOM, 0)
#define DEBUG_

#define GET_SIZE(type, size) size,
#define GET_TYPE(type, size) type,
extern QVector<uint8_t> ObjSize;
typedef enum { TYPE_SIZE_MAP(GET_TYPE) } ObjectType;
typedef struct
{
    float x_pos;
    float y_pos;
    float theta;
} __attribute__((packed)) XYTheta;

//using var_t = variant<int8_t, int16_t, int32_t, float, double, XYTheta>;

struct ENUMTYPE : QObject
{
    QString name;
    ENUMTYPE() = default;
    ENUMTYPE(QString name) { this->name = name; }
    map<uint8_t, QString> enum_Map;
};
class ObjectInfo : QObject
{
    Q_OBJECT
private:
    uint8_t id;
    ObjectType type;
    QString name;
    ENUMTYPE *ptr = nullptr;
    uint8_t size;
    union {
        int8_t i8;
        int16_t i16;
        int32_t i32;
        float f;
        double d;
        XYTheta xyt;
        uint8_t enumid;
    } value;
    uint8_t typeToSize(ObjectType type);

public:
    ObjectInfo(ObjectType type = FLOAT_, QString name = QString(""), ENUMTYPE *ptr = nullptr)
        : type(type), name(name), ptr(ptr){};
    ObjectType getType();
    void setTypeMem(ObjectType type);
    void setName(QString name);
    QString getName() const;
    void setId(uint8_t);
    uint8_t getId();
    //    int get_enumvalue() { return enum_value; };
    //    void set_enumvalue(int value) { enum_value = value; }

    void setEnumType(ENUMTYPE *ptr) { this->ptr = ptr; };
    ENUMTYPE *getEnumType() { return ptr; };

    //    QString get_enumName(QString name) { return enum_name; };
    bool isobj() { return (type > DOUBLE64_); }
    bool isnumeric() { return type <= XYTHETA_; }

    //    void set_ptrMem(void *ptr) { ptr_mem = ptr; }
    //    void *get_ptrMem() { return ptr_mem; }
    uint8_t getSize() { return size; };
    void setSize(uint8_t size) { this->size = size; }
    void setSize(ObjectType type) { this->size = typeToSize(type); };
    void setBytestoValue(char *arry);
    void *getValueRef() { return &value; };
    // void addPlot(PJ::PlotDataMapRef *datamap);
    void addPlotNumeric(PJ::PlotDataMapRef *datamap);
    void addPlotEnum(PJ::PlotDataMapRef *datamap);
    void addPlotifNotExist(PJ::PlotDataMapRef *datamap);

    void updatePlot(PJ::PlotDataMapRef &datamap, double time, double param[] = {});
    void updatePlotNumeric(PJ::PlotDataMapRef &datamap, double time, double param[] = {});
    void updatePlotEnum(PJ::PlotDataMapRef &datamap, double time);
    double getValueInDouble();

    ~ObjectInfo() {}
};
using var_t = std::variant<int8_t, int16_t, int32_t, float, double, XYTheta>;

#endif // OBJECTINFO_H
