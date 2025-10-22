from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout

class WebRTCExampleConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def validate(self):
        # Only allow Debug and RelWithDebInfo
        valid_build_types = ["Debug", "RelWithDebInfo"]
        if str(self.settings.build_type) not in valid_build_types:
            raise ConanInvalidConfiguration(
                f"Build type '{self.settings.build_type}' is not supported. "
                f"Only {valid_build_types} are allowed."
            )

    def requirements(self):
        self.requires("webrtc/7151")

    def configure(self):
        # Configure WebRTC options if needed
        # self.options["webrtc"].shared = False
        # self.options["webrtc"].with_h264 = True
        # self.options["webrtc"].enable_rtti = False
        pass

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()  # configure using CMake
        cmake.build()  # build project