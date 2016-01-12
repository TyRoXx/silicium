if(NOT (Boost_VERSION LESS 105400))
	add_executable(hellohttp "hellohttp.cpp")
	target_link_libraries(hellohttp ${Boost_LIBRARIES})
	set_target_properties(hellohttp PROPERTIES FOLDER examples)
endif()

add_executable(rxhttp "rxhttp.cpp")
target_link_libraries(rxhttp ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(rxhttp PROPERTIES FOLDER examples)

add_executable(trivialhttp "trivialhttp.cpp")
target_link_libraries(trivialhttp ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(trivialhttp PROPERTIES FOLDER examples)

if(NOT (Boost_VERSION LESS 105300)) #Boost.Coroutine required
	add_executable(sleep "sleep.cpp")
	target_link_libraries(sleep ${Boost_LIBRARIES})
	set_target_properties(sleep PROPERTIES FOLDER examples)
endif()

add_executable(http_hello "http_hello.cpp")
target_link_libraries(http_hello ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(http_hello PROPERTIES FOLDER examples)

add_executable(http_proxy "http_proxy.cpp")
target_link_libraries(http_proxy ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(http_proxy PROPERTIES FOLDER examples)

add_executable(html "html.cpp")
target_link_libraries(html ${Boost_LIBRARIES})
set_target_properties(html PROPERTIES FOLDER examples)

add_executable(dynamic_web_site "dynamic_web_site.cpp")
target_link_libraries(dynamic_web_site ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(dynamic_web_site PROPERTIES FOLDER examples)

add_executable(variant "variant.cpp")
target_link_libraries(variant ${Boost_LIBRARIES})
set_target_properties(variant PROPERTIES FOLDER examples)

if(SILICIUM_HAS_WEBSOCKETPP)
	add_executable(websocketpp "websocketpp.cpp" "websocketpp.hpp.js")
	if(${CMAKE_VERSION} VERSION_GREATER "2.8.8")
		if(MSVC)
			target_compile_options(websocketpp PRIVATE "/wd4127")
		else()
			target_compile_options(websocketpp PRIVATE "-Wno-conversion")
			target_compile_options(websocketpp PRIVATE "-Wno-unused-parameter")
		endif()
	endif()
	target_link_libraries(websocketpp ${Boost_LIBRARIES})
	set_target_properties(websocketpp PROPERTIES FOLDER examples)
endif()

if(MSVC14 AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
	add_executable(vs2015_await "vs2015_await.cpp")
	target_link_libraries(vs2015_await ${Boost_LIBRARIES})
	target_compile_options(vs2015_await PRIVATE "/await")
	set_target_properties(vs2015_await PROPERTIES FOLDER examples)
endif()

file(GLOB exampleSources "*.cpp" "*.hpp")
set(formatted ${formatted} ${exampleSources} PARENT_SCOPE)
