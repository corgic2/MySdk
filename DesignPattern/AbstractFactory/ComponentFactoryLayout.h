#pragma once
#include "AbstractFactory.h"
class ComponentFactoryLayout : public AbstractFactory
{
  public:
    bool CreateComponent() override;

    bool OperateComponent(ST_OptearteBaseInfo *info) override;

    bool NotifyComponent(ST_NotifyBaseInfo *info) override;

  private:
    void InitComponentLayoutMap();
};
