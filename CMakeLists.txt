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

include_directories(include)
add_definitions(-std=c++11)

option(BUILD_EXT_EXAMPLES   "Builds the examples" ON)

#==========================================================================


macro_add_subdirectory_if(example BUILD_EXT_EXAMPLES)

option(BUILD_SHARED_LIBRARY "Builds binary version as a shared library" ON)
if (BUILD_SHARED_LIBRARY)

	message (STATUS "* Will build shared library!")

	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

	file (GLOB EXT_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/cm/*.h )
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

	add_library ( ${PROJECT_NAME} SHARED
		${EXT_HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/src/lib.cpp
	)

	set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

	install (TARGETS ${PROJECT_NAME} DESTINATION lib COMPONENT library)

endif(BUILD_SHARED_LIBRARY)

install ( DIRECTORY include/cm
	DESTINATION ${CMAKE_INSTALL_PREFIX}/include
	COMPONENT headers
	FILES_MATCHING PATTERN "*.h"
)
