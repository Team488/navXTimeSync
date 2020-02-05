cmake_minimum_required (VERSION 2.8)
project (NavX)

if(NOT CMAKE_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD 11)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_compile_options(-std=c++11 -Wall -Wextra -Wno-switch -lpthread)
endif()

# Sources
if (WIN32)
	file(GLOB LIB_SOURCES
		"src/*"
		"src/windows/*")
else()
	file(GLOB LIB_SOURCES
		"src/*"
		"src/linux/*")
endif()

# Library
add_library(NavX STATIC ${LIB_SOURCES})
target_include_directories(NavX PUBLIC include)

# Application
add_executable(NavXTimeStamp "src/app/TimeStamp.cpp")
target_link_libraries(NavXTimeStamp NavX)

# Install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE)
endif()
install(
  TARGETS NavX NavXTimeStamp
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
install(
  DIRECTORY include/
  DESTINATION include/)
