from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake


class VCellFvSolverRecipe(ConanFile):
    """Dependency provider for vcell-fvsolver.

    Deliberately does NOT implement layout(): the Dockerfile and
    .github/workflows/cd.yml both drive this with
    ``conan install . --output-folder build``, which puts conan_toolchain.cmake
    and conanbuild.sh directly in build/. A cmake_layout() would move them to
    build/generators and break those call sites.
    """

    name = "vcell-fvsolver"
    version = "0.0.1"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "include_messaging": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "include_messaging": False,
        # HDF5's own zlib filter; the solver writes deflate-compressed datasets.
        "hdf5/*:with_zlib": True,
    }

    def validate(self):
        check_min_cppstd(self, "20")

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def requirements(self):
        self.requires("hdf5/[~1.14.5]")
        self.requires("libzip/[~1.11.3]")
        self.requires("zlib/[~1.2.11]")
        # libaec is only consumed on macOS, where the top-level CMakeLists does
        # find_package(libaec REQUIRED) to satisfy Homebrew's HDF5. Nothing on
        # Linux or Windows references it, and its sole upstream
        # (gitlab.dkrz.de) rate-limits hard enough that requiring it everywhere
        # makes builds fail for no benefit.
        if self.settings.os == "Macos":
            self.requires("libaec/[~1.1.2]")
        if self.options.include_messaging:
            self.requires("libcurl/[<9.0]")

    def build_requirements(self):
        self.tool_requires("cmake/[~3.31.6]")
        self.tool_requires("ninja/[~1.12.1]")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
