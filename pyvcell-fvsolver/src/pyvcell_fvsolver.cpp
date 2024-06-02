#include <../../extern/pybind11/include/pybind11/pybind11.h>

#include "SolverMain.h"

namespace py = pybind11;

PYBIND11_MODULE(pyvcell_fvsolver, m) {
    m.doc() = "VCell FiniteVolume plugin"; // optional module docstring

    m.def("version", &version, "A function that returns the version of the FiniteVolume solver");

    m.def("solve", &solve, "A function that invokes the FiniteVolume solver", py::arg("inputFilename"), py::arg("outputDir"));
}
