# ---------------------------------------------------------------- PROJECT NAME
set(LOCAL_PROJ_NAME common)


# -------------------------------------------------------------- FIND SRC FILES
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)
    file(GLOB_RECURSE CSRC ${CMAKE_CURRENT_LIST_DIR}/*.c)
endif()

# ---------------------------------------------------------------- DEFINE BUILD
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)

    # Add library
    add_library(${LOCAL_PROJ_NAME} STATIC ${CSRC} ${ASRC})

    target_link_libraries(${LOCAL_PROJ_NAME} PRIVATE stm32cubemx SEGGER_RTT)

    # Add include paths
    target_include_directories(${LOCAL_PROJ_NAME}
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}/
            ${CMAKE_CURRENT_LIST_DIR}/../SEGGER_RTT/RTT
            ${CMAKE_CURRENT_LIST_DIR}/../stm32cubemx/Core/Inc
            ${CMAKE_CURRENT_LIST_DIR}/../stm32cubemx/Drivers/CMSIS/Include
            ${CMAKE_CURRENT_LIST_DIR}/../stm32cubemx/Drivers/CMSIS/Device/ST/STM32H7xx/Include
            ${CMAKE_CURRENT_LIST_DIR}/../stm32cubemx/Drivers/STM32H7xx_HAL_Driver/Inc
            ${CMAKE_CURRENT_LIST_DIR}/../stm32cubemx/Middlewares/Third_Party/FreeRTOS/Source/include
            ${CMAKE_CURRENT_LIST_DIR}/../stm32cubemx/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
            ${CMAKE_CURRENT_LIST_DIR}/../stm32cubemx/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    )
endif()
