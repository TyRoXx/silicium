from conans import ConanFile
import os

class SiliciumConan(ConanFile):
    name = "silicium"
    version = "0.2"
    generators = "cmake"
    requires = "Boost/1.59.0@lasote/stable", "sqlite3/3.10.2@TyRoXx/stable"
    url="http://github.com/tyroxx/silicium"
    license="MIT"
    exports="silicium/*"

    def package(self):
        self.copy(pattern="*.hpp", dst="include/silicium", src="silicium", keep_path=True)
