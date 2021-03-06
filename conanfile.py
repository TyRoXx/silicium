from conans import ConanFile
import os

class SiliciumConan(ConanFile):
    name = "silicium"
    version = "0.12"
    generators = "cmake"
    requires = "Boost/1.60.0@lasote/stable", "websocketpp/0.7.0@TyRoXx/stable"
    url="http://github.com/tyroxx/silicium"
    license="MIT"
    exports="silicium/*"

    def package(self):
        self.copy(pattern="*.hpp", dst="include/silicium", src="silicium", keep_path=True)

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dylib*", dst="bin", src="lib")
