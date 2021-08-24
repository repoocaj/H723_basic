# ------------------------------------------------------- MINIMUM CMAKE VERSION
cmake_minimum_required(VERSION 3.15.0)


# ---------------------------------------------------------------- PROJECT NAME
set(LOCAL_PROJ_NAME stm32cubemx)


# -------------------------------------------------------------- FIND SRC FILES
file(GLOB_RECURSE CSRC ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/*.c
                       ${CMAKE_CURRENT_LIST_DIR}/Middlewares/Third_Party/FreeRTOS/Source/*.c)


# ---------------------------------------------------------------- DEFINE BUILD
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)
    # Add library
    add_library(${LOCAL_PROJ_NAME} ${CSRC})

    # Add include paths
    target_include_directories(${LOCAL_PROJ_NAME}
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Core/Inc
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Drivers/CMSIS/Include
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../common
    )
else()
    # Add library
    add_library(${LOCAL_PROJ_NAME} INTERFACE)

    # Add headers
    file(GLOB_RECURSE LOCAL_HEADERS ${CMAKE_CURRENT_LIST_DIR}/Drivers/*.h)
    target_sources(${LOCAL_PROJ_NAME} INTERFACE ${LOCAL_HEADERS})

    # Add include paths
    target_include_directories(${LOCAL_PROJ_NAME}
        INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Core/Inc
        INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Drivers/CMSIS/Include
        INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
        INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
        INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
        INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
        INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    )
endif()
