file(GLOB sources
	"*.hpp" "*.cpp"
	"http/*.hpp" "http/*.cpp")
add_library(silicium ${sources})

file(GLOB sources "asio/*.hpp" "asio/*cpp")
add_library(silicium_asio ${sources})

if(LIBGIT2_FOUND)
	file(GLOB sources "git/*.hpp" "git/*cpp")
	add_library(silicium_git ${sources})
endif()
