file(GLOB sources "*.hpp" "*.cpp")
add_library(cdm_websocketpp SHARED ${sources})
target_link_libraries(cdm_websocketpp ${Boost_LIBRARIES})
set_target_properties(cdm_websocketpp PROPERTIES FOLDER cdm)
