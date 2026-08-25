cmake_minimum_required(VERSION 3.23)
project(peep C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXECUTABLE_SUFFIX .elf)

set(MCU_FLAGS -mcpu=cortex-m0 -mthumb)
set(DEVICE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cmsis-device-f0)
set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cmsis-core)

add_executable(peep
  src/board.c
  src/main.c
  src/runtime.c
  ${DEVICE_DIR}/Source/Templates/system_stm32f0xx.c
  ${DEVICE_DIR}/Source/Templates/gcc/startup_stm32f042x6.s
)

target_compile_definitions(peep PRIVATE STM32F042x6)
target_include_directories(peep PRIVATE
  include
  ${DEVICE_DIR}/Include
  ${CORE_DIR}/CMSIS/Core/Include
)
target_compile_options(peep PRIVATE
  ${MCU_FLAGS}
  -ffunction-sections
  -fdata-sections
  -fno-common
  -Wall
  -Wextra
  -Werror
  $<$<CONFIG:Debug>:-Og;-g3>
  $<$<NOT:$<CONFIG:Debug>>:-Os>
)
target_link_options(peep PRIVATE
  ${MCU_FLAGS}
  -nostartfiles
  --specs=nano.specs
  --specs=nosys.specs
  -Wl,--gc-sections
  -Wl,-Map,$<TARGET_FILE_DIR:peep>/peep.map
  -T${CMAKE_CURRENT_SOURCE_DIR}/linker/stm32f042g6.ld
)

add_custom_command(TARGET peep POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:peep> $<TARGET_FILE_DIR:peep>/peep.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:peep> $<TARGET_FILE_DIR:peep>/peep.hex
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:peep>
)
