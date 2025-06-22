from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout

class BaseRecipe(ConanFile):
    name = "uber"
    version = "1.0"
    
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("app_utils/1.0@pasan/testing")
        self.requires("app_kafka/1.0@pasan/testing")
        self.requires("app_h3/1.0@pasan/testing")
        self.requires("app_algorithms/1.0@pasan/testing")
        self.requires("app_database/1.0@pasan/testing")
        
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def layout(self):
        cmake_layout(self)