from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout

class BaseRecipe(ConanFile):
    name = "uberBackend"
    version = "1.0"

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("protobuf/3.21.12", transitive_headers=True, transitive_libs=True)
        self.requires("grpc/1.54.3")
        self.requires("abseil/20230125.3")
        self.requires("jwt-cpp/0.7.0")
        self.requires("nlohmann_json/3.11.2")
        self.requires("hiredis/1.0.0")
    
        self.requires("app_utils/1.0@pasan/testing")
        self.requires("app_kafka/1.0@pasan/testing")
        self.requires("app_rabbitmq/1.0@pasan/testing")
        self.requires("app_h3/1.0@pasan/testing")
        self.requires("app_blockchain/1.0@pasan/testing")
        self.requires("app_algorithms/1.0@pasan/testing")
        self.requires("app_database/1.0@pasan/testing")

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
