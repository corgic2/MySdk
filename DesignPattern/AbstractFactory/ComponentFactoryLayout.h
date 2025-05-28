#pragma once
#include "AbstractFactory.h"
class ComponentFactoryLayout : public CptBaseObject
{
  public:
    ST_ComponentBaseInfo* CreateComponentObject(unsigned long uid, const std::string& cptName) override;


};
