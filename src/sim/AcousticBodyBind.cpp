#include "pybind11/pybind11.h"
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#include "sim/AcousticBody.h"

namespace py = pybind11;

// int add(int a, int b) { return a + b; }

PYBIND11_MODULE(AcousticBinder, m)
{
    // m.doc() = "pybind11 example plugin";
    // m.def("add", &add, "A function to add");

    py::class_<cAcousticBody, std::shared_ptr<cAcousticBody>>(m,
                                                              "acoustic_body")
        .def(py::init<int>())
        .def("InitFromIni", &cAcousticBody::InitFromIni)
        .def("GetIniPath", &cAcousticBody::GetIniPath)
        .def("GetVertexPosVec", &cAcousticBody::GetVertexPosVec)
        .def("GetTriIdVec", &cAcousticBody::GetTriIdVec)
        .def("UpdateCamPos", &cAcousticBody::UpdateCamPos)
        .def("GetCamPos", &cAcousticBody::GetCamPos)
        .def("ClickTriangle", &cAcousticBody::ClickTriangle);
}