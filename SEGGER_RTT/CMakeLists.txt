# ------------------------------------------------------- MINIMUM CMAKE VERSION
cmake_minimum_required(VERSION 3.15.0)


# ---------------------------------------------------------------- PROJECT NAME
set(LOCAL_PROJ_NAME SEGGER_RTT)


# -------------------------------------------------------------- FIND SRC FILES
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)
    file(GLOB_RECURSE CSRC ${CMAKE_CURRENT_LIST_DIR}/RTT/*.c)
    file(GLOB_RECURSE ASRC ${CMAKE_CURRENT_LIST_DIR}/RTT/*.S)
endif()

# ---------------------------------------------------------------- DEFINE BUILD
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)
    # Add library
    add_library(${LOCAL_PROJ_NAME} STATIC ${CSRC} ${ASRC})

    # Add include paths
    target_include_directories(${LOCAL_PROJ_NAME}
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/RTT
    )
else()
    # TODO: What should we do when we're not building for arm
endif()
