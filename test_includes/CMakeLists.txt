option(SILICIUM_TEST_INCLUDES "" ON)
if(SILICIUM_TEST_INCLUDES)
	file(GLOB_RECURSE sources "dummy.cpp" "sources/*.cpp")
	add_library(test_includes ${sources})

	add_executable(test_includes_generate "generate_sources.cpp")
	target_link_libraries(test_includes_generate silicium ${Boost_LIBRARIES})
endif()
