#pragma once
#include "AbstractFactory.h"
class ComponentFactoryFunction : public CptBaseObject
{
  public:
    ST_ComponentBaseInfo* CreateComponentObject(unsigned long uid, const std::string& cptName) override;

};

