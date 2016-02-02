from conans import ConanFile
import os

class SiliciumConan(ConanFile):
    name = "silicium"
    version = "0.4"
    generators = "cmake"
    requires = "Boost/1.60.0@lasote/stable"
    url="http://github.com/tyroxx/silicium"
    license="MIT"
    exports="silicium/*"

    def package(self):
        self.copy(pattern="*.hpp", dst="include/silicium", src="silicium", keep_path=True)
