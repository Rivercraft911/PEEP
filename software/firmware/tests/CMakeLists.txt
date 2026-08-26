cmake_minimum_required(VERSION 3.23)
project(peep_tests C)

enable_testing()

add_executable(peep_tests
  test_main.c
  ../src/app.c
  ../src/hid_ascii.c
  ../src/macro.c
  ../src/trigger_gate.c
)
target_include_directories(peep_tests PRIVATE ../include)
target_compile_features(peep_tests PRIVATE c_std_11)
target_compile_options(peep_tests PRIVATE -Wall -Wextra -Werror)

add_test(NAME peep_tests COMMAND peep_tests)
