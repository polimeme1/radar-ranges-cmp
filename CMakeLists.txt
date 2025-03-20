 cmake_minimum_required(VERSION 3.15)

include_guard(GLOBAL)

project(test
  DESCRIPTION "test"
  LANGUAGES C CXX
)

set(PROJECT_NAMESPACE ${PROJECT_NAME}::)
set(PROJECT_FULL_NAME ${PROJECT_NAMESPACE}${PROJECT_NAME})

# <-- msvc-related options -->
if ("${CMAKE_GENERATOR}" MATCHES "^Visual Studio")
  set(CMAKE_GENERATOR_PLATFORM "x64" CACHE STRING "" FORCE)
endif ()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# <-- dependencies -->
find_package(Catch2 REQUIRED)

add_executable(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

target_sources(${PROJECT_NAME}
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/solution.cc
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  Catch2::Catch2WithMain
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(${PROJECT_NAME} PRIVATE -DTEST_DATA_DIR="${CMAKE_SOURCE_DIR}/test_data")

if (WIN32)
  if (MSVC)
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
  endif ()
endif ()

if (NOT WIN32)
  message(STATUS "[${PROJECT_FULL_NAME}] copying ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json to the root folder ${CMAKE_CURRENT_SOURCE_DIR}")
  file(CREATE_LINK
    ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    SYMBOLIC
  )
endif ()