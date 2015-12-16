cmake_minimum_required (VERSION 2.6)
project (NarfLog)

option(TEST "Build narflog unit tests" OFF)
option(STATIC "Use static linking when building executables" OFF)
option(STATIC_LIBSTDCXX "Static link libstdc++" OFF)
option(STATIC_LIBGCC "Static link libgcc" OFF)
option(PIE "Create position-independent executables" ON)

set(EXT_PROJECTS_DIR ${CMAKE_SOURCE_DIR}/ext)

get_directory_property(hasParent PARENT_DIRECTORY)

if (PIE AND STATIC)
	# PIE + STATIC will result in TEXTRELs unless all libraries have been
	# built for PIC/PIE, so don't allow it
	set(PIE OFF)
endif()

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (VERSION_RELEASE "")

if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
	set(CMAKE_COMPILER_IS_CLANG 1)
endif ()

if (TEST)
	find_package (GTest REQUIRED)
endif ()

add_subdirectory(${EXT_PROJECTS_DIR}/narflib)

if (hasParent)
	set(STATIC ON)
	set(NARFLOG_INCLUDE_DIRS ${NARFLOG_INCLUDE_DIRS} PARENT_SCOPE)
	set(NARFLOG_SOURCE_DIR ${NARFLOG_SOURCE_DIR} PARENT_SCOPE)
else()
	if (STATIC)
		set(STATIC_LIBSTDCXX ON)
		set(STATIC_LIBGCC ON)
	endif()

	option(PIE "Create position-independent executables" ON)

	if (PIE AND STATIC)
		message(STATUS "Building as static, disabling PIE")
		# PIE + STATIC will result in TEXTRELs unless all libraries have been
		# built for PIC/PIE, so don't allow it
		set(PIE OFF)
	endif()

# -pie with MinGW-w64 breaks startup (incorrect entry point
# is used), so don't allow PIE on Windows for now
# http://sourceforge.net/p/mingw-w64/mailman/mingw-w64-public/thread/CAGFXeQKg0J%2Bax0Dj-cBXGGrwWT15K-aRsWWaBN6wWSDUtRHoLg@mail.gmail.com/
	if (PIE AND WIN32)
		set(PIE OFF)
	endif()
	if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
		set(CMAKE_COMPILER_IS_CLANG 1)
	endif ()

	if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
		SET(COMMON_C_FLAGS
			"${CMAKE_C_FLAGS} -Wpointer-arith -Wtype-limits -Wwrite-strings -Wuninitialized -Werror=redundant-decls -Wsign-compare -Wconversion -g -fno-ident")

		# don't export symbols (especially for PIC builds, but it doesn't hurt elsewhere)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

		if (STATIC)
			SET(STATIC_C_FLAGS "-static")
		endif()

		if (STATIC_LIBGCC)
			SET(STATIC_C_FLAGS "${STATIC_C_FLAGS} -static-libgcc")
		endif()

		if (STATIC_LIBSTDCXX)
			SET(STATIC_C_FLAGS "${STATIC_C_FLAGS} -static-libstdc++")
		endif()

		if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
			# position-independent executable
			if (PIE)
				SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
				SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie -Wl,-z,relro,-z,now")
			endif()

			SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
		endif()


		SET(COMMON_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_C_FLAGS} -std=gnu++0x")
	endif ()

	if (MSVC)
		# ask MSVC to define M_PI and related constants
		add_definitions( -D_USE_MATH_DEFINES )
	endif ()
endif()

set(CMAKE_C_FLAGS "${COMMON_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${COMMON_CXX_FLAGS}")

include_directories (
	"${NARFLIB_INCLUDE_DIRS}"
	"include/"
	"${PROJECT_BINARY_DIR}"
	"${PROJECT_SOURCE_DIR}"
	)

set (NARFLOG_SOURCE_FILES
	src/Logger.cpp
	src/Entry.cpp
	src/Level.cpp
	src/View.cpp
	)

if(STATIC)
	if (NOT hasParent)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${STATIC_C_FLAGS}")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STATIC_C_FLAGS}")
	endif()
	add_library (narflog STATIC ${NARFLOG_SOURCE_FILES})
	add_dependencies(narflog narflib)
else()
	add_library (narflog SHARED ${NARFLOG_SOURCE_FILES})
	add_dependencies(narflog narflib)
	target_link_libraries (narflog)
endif()


if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set_target_properties (narflog
		PROPERTIES LINK_FLAGS "-Wl,-Map=NarfLog.map"
		)
endif ()

if(TEST)
	include_directories ( "${GTEST_INCLUDE_DIRS}")

	if (NOT hasParent)
		set(CMAKE_C_FLAGS "${COMMON_C_FLAGS}")
		set(CMAKE_CXX_FLAGS "${COMMON_CXX_FLAGS}")
	endif()

	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGTEST_LINKED_AS_SHARED_LIBRARY=1")

	add_executable (narflog-test src/test.cpp)
	add_dependencies(narflog-test narflib)

	target_link_libraries (narflog-test
		narflog
		${NARFLIB_LIBS_DIR}/libnarflib.a
		${GTEST_LIBRARIES}
		)
endif()


if (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
