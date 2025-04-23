#pragma once
#include "AbstractFactory.h"
class ComponentFactoryFunction : public AbstractFactory
{
  public:
    bool CreateComponent() override;

    bool OperateComponent(ST_OptearteBaseInfo *info) override;

    bool NotifyComponent(ST_NotifyBaseInfo *info) override;

  private:
    void InitComponentFunctionMap();
};
