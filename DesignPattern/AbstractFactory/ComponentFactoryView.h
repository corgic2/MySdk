#pragma once
#include "AbstractFactory.h"
class ComponentFactoryView : public CptBaseObject
{
  public:
    void RegisterComponent()override;
    void PrintText() override;
};
