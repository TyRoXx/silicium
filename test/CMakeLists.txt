file(GLOB sources "*.hpp" "*.cpp" "../silicium/*.hpp")
add_executable(test ${sources})
target_link_libraries(test ${Boost_LIBRARIES})
