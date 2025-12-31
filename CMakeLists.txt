cmake_minimum_required(VERSION 3.20)
project(logforge VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_SANITIZERS "Enable ASan/UBSan" OFF)
option(BUILD_TESTING "Build tests" ON)

add_library(logforge_lib
  src/aggregator.cpp
  src/parser_nginx.cpp
  src/report_csv.cpp
  src/report_json.cpp
  src/buffered_reader.cpp
)
target_include_directories(logforge_lib PUBLIC include)
target_compile_options(logforge_lib PRIVATE -Wall -Wextra -Wpedantic)

add_executable(logforge src/main.cpp)
target_link_libraries(logforge PRIVATE logforge_lib)

if(ENABLE_SANITIZERS)
  include(cmake/Sanitizers.cmake)
  enable_sanitizers(logforge_lib)
  enable_sanitizers(logforge)
endif()

if(BUILD_TESTING)
  include(CTest)
  add_subdirectory(tests)
endif()
