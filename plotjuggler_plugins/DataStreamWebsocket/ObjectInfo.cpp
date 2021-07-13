#include "ObjectInfo.h"

ObjectType ObjectInfo::getType()
{
    return type;
}
void ObjectInfo::setType(ObjectType type)
{
    this->type = type;
}

void ObjectInfo::setName(QString name)
{
    this->name = name;
}

QString ObjectInfo::getName()
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
