# ------------------------------------------------------- MINIMUM CMAKE VERSION
cmake_minimum_required(VERSION 3.15.0)

# ---------------------------------------------------------------- PROJECT NAME
set(LOCAL_PROJ_NAME application.elf)

get_filename_component(PRJ_BASENAME ${LOCAL_PROJ_NAME} NAME_WE)

# -------------------------------------------------------------- FIND SRC FILES
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)
    file(GLOB_RECURSE CSRC ${CMAKE_CURRENT_LIST_DIR}/*.c
                           ${CMAKE_CURRENT_LIST_DIR}/../stm32cubemx/*.s
                           ${CMAKE_CURRENT_LIST_DIR}/../stm32cubemx/Core/Src/*.c)
endif()

# --------------------------------------------------------- DEFINE BUILD TARGET
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)
    add_executable(${LOCAL_PROJ_NAME} ${CSRC})

    target_link_libraries(${LOCAL_PROJ_NAME} stm32cubemx SEGGER_RTT common)

    target_link_libraries(${LOCAL_PROJ_NAME} ${CFLAGS} ${LD_FLAGS} "-Wl,-Map=${PRJ_BASENAME}.map")

    target_link_libraries(${LOCAL_PROJ_NAME} "-T ${CMAKE_CURRENT_LIST_DIR}/../stm32cubemx/STM32H723ZGTX_FLASH.ld -L${CMAKE_BINARY_DIR}")

    target_include_directories(${LOCAL_PROJ_NAME}
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Inc
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../stm32cubemx/Core/Inc
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../stm32cubemx/Drivers/CMSIS/Include
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../stm32cubemx/Drivers/CMSIS/Device/ST/STM32H7xx/Include
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../stm32cubemx/Drivers/STM32H7xx_HAL_Driver/Inc)
endif()

# ------------------------------------------------------- CREATE CUSTOM TARGETS
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)
    # Build the HEX file
    add_custom_target(${PRJ_BASENAME}.hex ALL ${CMAKE_OBJCOPY} -O ihex ${LOCAL_PROJ_NAME} ${PRJ_BASENAME}.hex DEPENDS ${LOCAL_PROJ_NAME} COMMENT "APPLICATION Building Hex File")
    # Create a binary file
    add_custom_target(${PRJ_BASENAME}.bin ALL ${CMAKE_OBJCOPY} -O binary ${LOCAL_PROJ_NAME} ${PRJ_BASENAME}.bin DEPENDS ${LOCAL_PROJ_NAME} COMMENT "APPLICATION Building Binary File")
endif()

# ---------------------------------------------------------- PRINT PROJECT SIZE
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)
    add_custom_command(TARGET ${LOCAL_PROJ_NAME} POST_BUILD COMMAND ${CMAKE_SIZE_UTIL} --format=berkeley ${LOCAL_PROJ_NAME})
endif()
