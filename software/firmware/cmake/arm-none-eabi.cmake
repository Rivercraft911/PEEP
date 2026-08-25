set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

file(GLOB ARM_TOOLCHAIN_DIRS "/Applications/ArmGNUToolchain/*/arm-none-eabi/bin")
list(SORT ARM_TOOLCHAIN_DIRS ORDER DESCENDING)

find_program(ARM_GCC arm-none-eabi-gcc PATHS ${ARM_TOOLCHAIN_DIRS} NO_DEFAULT_PATH)
if(NOT ARM_GCC)
  find_program(ARM_GCC arm-none-eabi-gcc REQUIRED)
endif()

get_filename_component(ARM_BIN_DIR ${ARM_GCC} DIRECTORY)
set(CMAKE_C_COMPILER ${ARM_GCC})
set(CMAKE_ASM_COMPILER ${ARM_GCC})
find_program(CMAKE_OBJCOPY arm-none-eabi-objcopy PATHS ${ARM_BIN_DIR} NO_DEFAULT_PATH REQUIRED)
find_program(CMAKE_SIZE arm-none-eabi-size PATHS ${ARM_BIN_DIR} NO_DEFAULT_PATH REQUIRED)
