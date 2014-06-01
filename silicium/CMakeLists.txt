file(GLOB sources
	"*.hpp" "*.cpp"
	"http/*.hpp" "http/*.cpp")
add_library(silicium ${sources})
target_link_libraries(silicium ${Boost_LIBRARIES})

file(GLOB sources "asio/*.hpp" "asio/*cpp")
add_library(silicium_asio ${sources})
target_link_libraries(silicium_asio silicium)

if(LIBGIT2_FOUND)
	file(GLOB sources "git/*.hpp" "git/*cpp")
	add_library(silicium_git ${sources})
	target_link_libraries(silicium_git silicium)
endif()
