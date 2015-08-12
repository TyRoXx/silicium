file(GLOB sources "*.hpp" "*.cpp")
add_library(cdm_core ${sources})
target_link_libraries(cdm_core ${Boost_LIBRARIES})
set_target_properties(cdm_core PROPERTIES FOLDER cdm)
