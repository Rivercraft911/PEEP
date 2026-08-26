cmake_minimum_required(VERSION 3.23)
project(peep C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXECUTABLE_SUFFIX .elf)

set(MCU_FLAGS -mcpu=cortex-m0 -mthumb)
set(DEVICE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cmsis-device-f0)
set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cmsis-core)
set(TINYUSB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyusb)

add_executable(peep
  src/app.c
  src/board.c
  src/dfu_runtime.c
  src/hid_ascii.c
  src/macro.c
  src/main.c
  src/runtime.c
  src/usb_descriptors.c
  src/usb_keyboard.c
  ${DEVICE_DIR}/Source/Templates/system_stm32f0xx.c
  ${DEVICE_DIR}/Source/Templates/gcc/startup_stm32f042x6.s
  ${TINYUSB_DIR}/src/tusb.c
  ${TINYUSB_DIR}/src/common/tusb_fifo.c
  ${TINYUSB_DIR}/src/device/usbd.c
  ${TINYUSB_DIR}/src/class/hid/hid_device.c
  ${TINYUSB_DIR}/src/class/dfu/dfu_rt_device.c
  ${TINYUSB_DIR}/src/portable/st/stm32_fsdev/dcd_stm32_fsdev.c
  ${TINYUSB_DIR}/src/portable/st/stm32_fsdev/fsdev_common.c
)

target_compile_definitions(peep PRIVATE
  STM32F042x6
  CFG_TUSB_MCU=OPT_MCU_STM32F0
)
target_include_directories(peep PRIVATE
  include
  ${DEVICE_DIR}/Include
  ${CORE_DIR}/CMSIS/Core/Include
)
target_include_directories(peep SYSTEM PRIVATE ${TINYUSB_DIR}/src)
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
set_property(TARGET peep APPEND PROPERTY LINK_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/linker/stm32f042g6.ld
)

add_custom_command(TARGET peep POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:peep> $<TARGET_FILE_DIR:peep>/peep.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:peep> $<TARGET_FILE_DIR:peep>/peep.hex
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:peep>
)
