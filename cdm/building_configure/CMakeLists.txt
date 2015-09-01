file(GLOB sources "*.hpp" "*.cpp")
add_library(building_configure ${sources})
target_link_libraries(building_configure ${Boost_LIBRARIES})
set_target_properties(building_configure PROPERTIES FOLDER cdm)
