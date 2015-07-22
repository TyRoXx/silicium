if(NOT (Boost_VERSION LESS 105400))
	add_executable(hellohttp "hellohttp.cpp")
	target_link_libraries(hellohttp ${Boost_LIBRARIES})
endif()

add_executable(rxhttp "rxhttp.cpp")
target_link_libraries(rxhttp ${Boost_LIBRARIES})

add_executable(trivialhttp "trivialhttp.cpp")
target_link_libraries(trivialhttp ${Boost_LIBRARIES})

add_executable(directory_watcher "directory_watcher.cpp")
target_link_libraries(directory_watcher ${Boost_LIBRARIES})

if(NOT (Boost_VERSION LESS 105300)) #Boost.Coroutine required
	add_executable(sleep "sleep.cpp")
	target_link_libraries(sleep ${Boost_LIBRARIES})
endif()

add_executable(http_hello "http_hello.cpp")
target_link_libraries(http_hello ${Boost_LIBRARIES})

add_executable(http_proxy "http_proxy.cpp")
target_link_libraries(http_proxy ${Boost_LIBRARIES})

add_executable(dynamic_web_site "dynamic_web_site.cpp")
target_link_libraries(dynamic_web_site ${Boost_LIBRARIES})

add_executable(variant "variant.cpp")
target_link_libraries(variant ${Boost_LIBRARIES})

add_executable(silicium_build_tester "silicium_build_tester.cpp")
target_link_libraries(silicium_build_tester ${Boost_LIBRARIES} ${URIPARSER_LIBRARIES})

if(SILICIUM_HAS_WEBSOCKETPP)
	add_executable(websocketpp "websocketpp.cpp" "websocketpp.hpp.js")
	if(MSVC)
		target_compile_options(websocketpp PRIVATE "/wd4127")
	else()
		target_compile_options(websocketpp PRIVATE "-Wno-conversion")
		target_compile_options(websocketpp PRIVATE "-Wno-unused-parameter")
	endif()
	target_link_libraries(websocketpp ${Boost_LIBRARIES})
endif()

if(UNIX) #TODO: Windows
	add_executable(run_process "run_process.cpp")
	target_link_libraries(run_process ${Boost_LIBRARIES})
endif()
