cmake_minimum_required(VERSION 2.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

#==========================================================================
# Useful macros
#==========================================================================

macro(macro_add_subdirectory_if subdir expression)
	if (${expression})
		message (STATUS "* Including subdirectory ${subdir}")
		add_subdirectory(${subdir})
	endif()
endmacro()

#==========================================================================
#==========================================================================

project(checkmark C CXX)
set(PROJECT_VERSION 0.0.1)

set (CMAKE_CXX_FLAGS_DEBUG "-O0 -pedantic -g -Wall -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Woverloaded-virtual -Wwrite-strings -Wdeprecated -Wdeprecated-declarations -Wno-variadic-macros" )
set (CMAKE_C_FLAGS_DEBUG "-O0 -pedantic -g -Wall -Wshadow -Wno-unused-parameter -W  -Wno-unused-function -Wunused -Wno-system-headers -Wwrite-strings -Wdeprecated -Wdeprecated-declarations -Wno-variadic-macros" )
add_definitions(-std=c++11)

option(BUILD_EXT_EXAMPLES   "Builds the examples" ON)


include_directories(include)
#==========================================================================


macro_add_subdirectory_if(example BUILD_EXT_EXAMPLES)

option(BUILD_SHARED_LIBRARY "Builds binary version as a shared library" ON)
if (BUILD_SHARED_LIBRARY)

	message (STATUS "* Will build shared library!")

	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

	file (GLOB EXT_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/cm/*.h )
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

	add_library ( ${PROJECT_NAME} SHARED ${EXT_HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/src/lib.cpp)

	set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
	set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION})

	install (TARGETS ${PROJECT_NAME} DESTINATION lib COMPONENT library)

endif(BUILD_SHARED_LIBRARY)

install ( DIRECTORY include/cm
	DESTINATION ${CMAKE_INSTALL_PREFIX}/include
	COMPONENT headers
	FILES_MATCHING PATTERN "*.h"
)
