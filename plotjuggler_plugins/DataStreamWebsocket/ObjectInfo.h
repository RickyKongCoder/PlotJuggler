#ifndef OBJECTINFO_H
#define OBJECTINFO_H
#include <iterator>
#include <map>
#include <QObject>
#include <QString>
typedef enum { FLOAT_, INT8_, INT16_, INT32_, DOUBLE64_, XYTHETA_, ENUMTYPE_ } ObjectType;

typedef struct
{
    uint8_t start;
    float x_pos;
    float y_pos;
    float theta;
} XYTheta;

struct ENUMTYPE : QObject
{
    QString name;
    std::map<int, QString> enum_Map;
};
class ObjectInfo : QObject
{
    Q_OBJECT
private:
    uint8_t id;
    ObjectType type;
    QString name;
    QString enum_name;
    int enum_value;
    void *ptr_mem;

public:
    ObjectInfo(ObjectType type = FLOAT_, QString name = QString("")) : type(type), name(name){};
    ObjectType getType();
    void setType(ObjectType type);
    void setName(QString name);
    QString getName();
    void setId(uint8_t);
    uint8_t getId();
    int get_enumvalue() { return enum_value; };
    void set_enumvalue(int value) { enum_value = value; }
    void set_enumName(QString name) { enum_name = name; }
    QString get_enumName(QString name) { return enum_name; };
    void set_ptrMem(void *ptr) { ptr_mem = ptr; }
    void *get_ptrMem() { return ptr_mem; }
    ~ObjectInfo()
    {
        if (ptr_mem != nullptr)
            delete ptr_mem;
    }
};

#endif // OBJECTINFO_H
