#include "ComponentFactoryFunction.h"

bool ComponentFactoryFunction::CreateComponent()
{
    return true;
}

bool ComponentFactoryFunction::OperateComponent(ST_OptearteBaseInfo *info)
{
    return true;
}

bool ComponentFactoryFunction::NotifyComponent(ST_NotifyBaseInfo *info)
{
    return true;
}

void ComponentFactoryFunction::InitComponentFunctionMap()
{
}