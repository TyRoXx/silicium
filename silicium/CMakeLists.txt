file(GLOB sources
	"*.hpp" "*.cpp"
	"http/*.hpp" "http/*.cpp")

file(GLOB asioSources "asio/*.hpp" "asio/*cpp")

if(LIBGIT2_FOUND)
	file(GLOB gitSources "git/*.hpp" "git/*cpp")
endif()

add_library(silicium ${sources} ${asioSources} ${gitSources})
target_link_libraries(silicium ${Boost_LIBRARIES})

install(TARGETS silicium
	RUNTIME DESTINATION "bin"
	LIBRARY DESTINATION "lib"
	ARCHIVE DESTINATION "lib")

install(DIRECTORY "." DESTINATION "include/silicium")
