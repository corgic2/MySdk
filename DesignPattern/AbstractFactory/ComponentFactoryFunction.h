#pragma once
#include "AbstractFactory.h"
class ComponentFactoryFunction : public CptBaseObject
{
  public:
    void RegisterComponent()override;
    void PrintText() override;
};

