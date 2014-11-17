file(GLOB sources
	"*.hpp" "*.cpp"
	"http/*"
	"sink/*"
	"detail/*"
	"source/*"
	"observable/*"
	"zlib/*"
	"posix/*"
)

file(GLOB asioSources "asio/*.hpp" "asio/*.cpp")

if(LIBGIT2_FOUND)
	file(GLOB gitSources "git/*.hpp" "git/*.cpp")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	file(GLOB linuxSources "linux/*.hpp" "linux/*.cpp")
endif()

if(WIN32)
	file(GLOB win32Sources "win32/*.hpp" "win32/*.cpp")
endif()

add_library(silicium ${sources} ${asioSources} ${gitSources} ${linuxSources} ${win32Sources})

install(TARGETS silicium
	RUNTIME DESTINATION "bin"
	LIBRARY DESTINATION "lib"
	ARCHIVE DESTINATION "lib")

install(DIRECTORY "." DESTINATION "include/silicium")
