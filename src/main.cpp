#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include <VCELL/SolverMain.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace nb = nanobind;

// Built against the Python stable ABI (abi3, Python >= 3.12): one wheel per platform serves every
// later CPython. nanobind translates std::runtime_error and std::invalid_argument thrown by solve()
// into RuntimeError and ValueError carrying the C++ message.
NB_MODULE(_core, m) {
    m.doc() = R"pbdoc(
        VCell FiniteVolume solver
        -------------------------

        .. currentmodule:: pyvcell_fvsolver

        .. autosummary::
           :toctree: _generate

           version
           solve
    )pbdoc";

    m.def("version", &version, R"pbdoc(
        version of build

        version string of build using git hash
    )pbdoc");

    m.def("solve", &solve, R"pbdoc(
        solve the ODE

        The inputFilename expects a .fvinput file, the outputDir will be created as needed.
    )pbdoc",
        nb::arg("fvInputFilename"), nb::arg("vcgInputFilename"), nb::arg("outputDir"));

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
