include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.4
)
FetchContent_MakeAvailable(Catch2)

add_executable(logforge_tests
  test_parser.cpp
  test_aggregator.cpp
)
target_link_libraries(logforge_tests PRIVATE logforge_lib Catch2::Catch2WithMain)
target_compile_options(logforge_tests PRIVATE -Wall -Wextra -Wpedantic)

include(Catch)
catch_discover_tests(logforge_tests)
