#pragma once
#include "utils/BaseTypeUtil.h"
#include <string>

class cImGuiUtil
{
public:
    static void SliderScalar(std::string name, _FLOAT *data, _FLOAT min = 0.0,
                             _FLOAT max = 0.0, std::string format = "%.3f");
    static void SliderScalar3(std::string name, _FLOAT *data, _FLOAT min = 0.0,
                              _FLOAT max = 0.0, std::string format = "%.3f");
    static void DragScalar(std::string name, _FLOAT *data, _FLOAT speed = 1.0,
                           _FLOAT min = 0.0, _FLOAT max = 0.0,
                           std::string format = "%.3f");
    static void DragScalar3(std::string name, _FLOAT *data, _FLOAT speed = 1.0,
                            _FLOAT min = 0.0, _FLOAT max = 0.0,
                            std::string format = "%.3f");
};