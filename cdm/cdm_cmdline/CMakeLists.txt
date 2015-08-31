file(GLOB sources "*.hpp" "*.cpp")
add_executable(cdm ${sources})
target_link_libraries(cdm ${Boost_LIBRARIES})
set_target_properties(cdm PROPERTIES FOLDER cdm)
