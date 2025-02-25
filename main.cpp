#include <iostream>
#include <string>
#include "FileSystem/FileSystemTest.h"
#include "JsonObject/JsonObjectTest.h"
#include "DesignPattern/SinglePattern.h"
int main()
{
  //FileSystemTest();
  //JsonObjectTest();

	SinglePatternClass::getinstance().print();
  return 0;
}
