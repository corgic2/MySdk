#pragma once
#include "AbstractFactory.h"
class ComponentFactoryLayout : public CptBaseObject
{
  public:
    void RegisterComponent()override;
    void PrintText() override;
};
