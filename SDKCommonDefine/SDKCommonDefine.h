#pragma once

#define SAFE_DELETE_POINTER_VALUE(p) {if(p) {delete p; p = nullptr;}}
#define SAFE_DELETE_POINTER_ARRAY(p) {if(p) {delete []p; p = nullptr;}}

#define DEBUG_LOG 1

#pragma execution_character_set("utf-8")
