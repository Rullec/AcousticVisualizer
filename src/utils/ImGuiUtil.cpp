#include "utils/ImGuiUtil.h"
#include "imgui.h"
void cImGuiUtil::SliderScalar(std::string name, _FLOAT *data, _FLOAT min,
                              _FLOAT max, std::string format)
{
    if constexpr (std::is_same<float, _FLOAT>::value)
    {
        // float
        ImGui::SliderScalar(name.c_str(), ImGuiDataType_Float, data, &min, &max,
                            format.c_str());
    }
    else
    {
        // double
        ImGui::SliderScalar(name.c_str(), ImGuiDataType_Double, data, &min,
                            &max, format.c_str());
    }
}
void cImGuiUtil::SliderScalar3(std::string name, _FLOAT *data, _FLOAT min,
                               _FLOAT max, std::string format)
{
    if constexpr (std::is_same<float, _FLOAT>::value)
    {
        // float
        ImGui::SliderScalarN(name.c_str(), ImGuiDataType_Float, data, 3, &min,
                             &max, format.c_str());
    }
    else
    {
        // double
        ImGui::SliderScalarN(name.c_str(), ImGuiDataType_Double, data, 3, &min,
                             &max, format.c_str());
    }
}
void cImGuiUtil::DragScalar(std::string name, _FLOAT *data, _FLOAT speed,
                            _FLOAT min, _FLOAT max, std::string format)
{
    if constexpr (std::is_same<float, _FLOAT>::value)
    {
        // float
        ImGui::DragScalar(name.c_str(), ImGuiDataType_Float, data, speed, &min,
                          &max, format.c_str());
    }
    else
    {
        // double
        ImGui::DragScalar(name.c_str(), ImGuiDataType_Double, data, speed, &min,
                          &max, format.c_str());
    }
}
void cImGuiUtil::DragScalar3(std::string name, _FLOAT *data, _FLOAT speed,
                             _FLOAT min, _FLOAT max, std::string format)
{
    if constexpr (std::is_same<float, _FLOAT>::value)
    {
        // float
        ImGui::DragScalarN(name.c_str(), ImGuiDataType_Float, data, 3, speed,
                           &min, &max, format.c_str());
    }
    else
    {
        // double
        ImGui::DragScalarN(name.c_str(), ImGuiDataType_Double, data, 3, speed,
                           &min, &max, format.c_str());
    }
}