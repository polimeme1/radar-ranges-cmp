from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout, CMakeDeps
from conan.tools.build import check_min_cppstd


class DailyChallengeRecipe(ConanFile):
    name = "test"
    version = "30.01.2025"
    settings = "os", "arch", "compiler", "build_type"
    exports = "CMakeLists.txt", "cmake/*"
    exports_sources = "*", "!build/*"

    @property
    def _min_cppstd(self):
        return "20"

    def requirements(self):
        self.requires("catch2/[=3.7.1]")

    def layout(self):
        cmake_layout(self)

    def validate(self):
        if self.settings.get_safe("compiler.cppstd"):
            check_min_cppstd(self, self._min_cppstd)

    def configure(self):
        pass

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
