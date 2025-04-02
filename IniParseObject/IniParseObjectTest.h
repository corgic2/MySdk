#pragma once
#include "INIParseObject.h"

void InitParseObjectTest()
{
    my_sdk::INIParseObject obj;
    obj.Load("../TestFiles/testIni.ini");
    obj.PrintAll();
}
