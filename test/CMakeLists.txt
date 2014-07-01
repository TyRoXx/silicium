file(GLOB sources "*.hpp" "*.cpp" "../silicium/*.hpp" "../reactive/*.hpp")
add_executable(test ${sources})
target_link_libraries(test silicium ${Boost_LIBRARIES})
