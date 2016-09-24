if(NOT (Boost_VERSION LESS 105400))
	add_executable(hellohttp "hellohttp.cpp")
	target_link_libraries(hellohttp ${Boost_LIBRARIES} ${CONAN_LIBS})
	set_target_properties(hellohttp PROPERTIES FOLDER examples)
endif()

add_executable(trivialhttp "trivialhttp.cpp")
target_link_libraries(trivialhttp ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${CONAN_LIBS})
set_target_properties(trivialhttp PROPERTIES FOLDER examples)

add_executable(html "html.cpp")
target_link_libraries(html ${Boost_LIBRARIES} ${CONAN_LIBS})
set_target_properties(html PROPERTIES FOLDER examples)

add_executable(variant "variant.cpp")
target_link_libraries(variant ${Boost_LIBRARIES} ${CONAN_LIBS})
set_target_properties(variant PROPERTIES FOLDER examples)

if(MSVC14 AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
	add_executable(vs2015_await "vs2015_await.cpp")
	target_link_libraries(vs2015_await ${Boost_LIBRARIES} ${CONAN_LIBS})
	target_compile_options(vs2015_await PRIVATE "/await")
	set_target_properties(vs2015_await PROPERTIES FOLDER examples)
endif()

file(GLOB exampleSources "*.cpp" "*.hpp")
set(formatted ${formatted} ${exampleSources} PARENT_SCOPE)
