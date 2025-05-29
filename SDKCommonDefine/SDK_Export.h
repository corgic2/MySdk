#pragma once
// include/SDK_Export.h
#pragma once
#define SDK_BUILD_DLL
#ifdef SDK_BUILD_DLL
#define SDK_API __declspec(dllexport)
#else
#define SDK_API __declspec(dllimport)
#endif
