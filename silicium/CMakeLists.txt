file(GLOB sources
	"*.hpp" "*.cpp"
	"http/*.hpp" "http/*.cpp")
add_library(silicium ${sources})
target_link_libraries(silicium ${Boost_LIBRARIES})

file(GLOB asioSources "asio/*.hpp" "asio/*cpp")
add_library(silicium_asio ${asioSources})
target_link_libraries(silicium_asio silicium)

install(TARGETS silicium silicium_asio
	RUNTIME DESTINATION "bin"
	LIBRARY DESTINATION "lib"
	ARCHIVE DESTINATION "lib")

install(DIRECTORY "." DESTINATION "include/silicium")

if(LIBGIT2_FOUND)
	file(GLOB sources "git/*.hpp" "git/*cpp")
	add_library(silicium_git ${sources})
	target_link_libraries(silicium_git silicium)
	install(TARGETS silicium_git
		RUNTIME DESTINATION "bin"
		LIBRARY DESTINATION "lib"
		ARCHIVE DESTINATION "lib")
endif()
