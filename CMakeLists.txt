# ------------------------------------------------------- MINIMUM CMAKE VERSION
cmake_minimum_required(VERSION 3.15.0)

# ---------------------------------------------------------------- PROJECT NAME
project(DEMO C CXX ASM)


# ---------------------------------------------------------- PRINT PROJECT NAME
message(STATUS "CMAKE_PROJECT_NAME          : " ${CMAKE_PROJECT_NAME})


# ----------------------------------------------------------- PRINT SYSTEM INFO
message(STATUS "CMAKE_SYSTEM_PROCESSOR      : " ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "CMAKE_SYSTEM_NAME           : " ${CMAKE_SYSTEM_NAME})
message(STATUS "CMAKE_VERSION               : " ${CMAKE_VERSION})


# ------------------------------------------------------- PRINT BUILD DIRECTORY
message(STATUS "CMAKE_CURRENT_BINARY_DIR    : " ${CMAKE_CURRENT_BINARY_DIR})

# -------------------------------------------------------- PRINT LIST DIRECTORY
message(STATUS "CMAKE_CURRENT_LIST_DIR      : " ${CMAKE_CURRENT_LIST_DIR})

# ------------------------------------------------------------------- UT OPTION
option(BUILD_UT "Builds the unit tests" ON)
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)
    set(BUILD_UT OFF)
endif()


# --------------------------------------------------- PROJECT INSTALL / STAGING
# TODO: This is for makes install option
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/build_output)


# ------------------------------------------------------------- PRINT TIMESTAMP
string(TIMESTAMP DEMO_BUILD_DATE "%m%d%y-%H:%M" UTC)
message(STATUS "DEMO_BUILD_DATE           : ${DEMO_BUILD_DATE}")


# ------------------------------------------------------ C/C++ LANGUAGE VERSION
include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-std=c++14" COMPILER_SUPPORTS_CXX14)
check_cxx_compiler_flag("-std=c++1y" COMPILER_SUPPORTS_CXX1Y)

if(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    message(STATUS "Enabled C++14 support")
elseif(COMPILER_SUPPORTS_CXX1Y)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
    message(STATUS "Enabled C++1y support")
else()
    message(FATAL_ERROR "${CMAKE_CXX_COMPILER} does not have C++14 support!")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
message(STATUS "Enabled C99 support")


# ------------------------------------------------- ARCHITECTURE SPECIFIC FLAGS
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)
    set(CPU_FLAGS "${CPU_FLAGS} -mcpu=cortex-m4")
    set(CPU_FLAGS "${CPU_FLAGS} -mthumb")
    set(CPU_FLAGS "${CPU_FLAGS} -mfloat-abi=hard")
    set(CPU_FLAGS "${CPU_FLAGS} -mfpu=fpv4-sp-d16")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CPU_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPU_FLAGS}")

    add_compile_options(-DSTM32H723xx)
    add_compile_options(-DUSE_HAL_DRIVER)

    # Print what board we are building for
    if(DEBUG)
        message(STATUS "BUILDING DEBUG VERSION")
        add_compile_options(-DDEBUG)
    else()
        message(STATUS "BUILDING RELEASE VERSION")
        add_compile_options(-DNDEBUG)
    endif()

    if(DEBUG_PINS)
        message(STATUS "ENABLING DEBUG PIN USAGE")
        add_compile_options(-DENABLE_DEBUG=1)
    else()
        message(STATUS "DISABLING DEBUG PIN USAGE")
        add_compile_options(-DENABLE_DEBUG=0)
    endif()

    # Use smallest possible enum
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fshort-enums")

    # Don't use exceptions or RTTI on the target
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
else()
    if(ENABLE_LOG)
        message(STATUS "Enabling Log Output")
        add_compile_options(-DENABLE_LOG)
    else()
        message(STATUS "Disabling Log Output")
    endif()

    add_compile_options(-DSTM32H723xx)
    # Test coverage flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage -fprofile-arcs")
endif()


# ---------------------------------------------------------------- LINKER FLAGS

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nano.specs")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nosys.specs")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker --gc-sections")

    # Include the path to the memory map for the linker
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_CURRENT_LIST_DIR}/common/Linker")

    # Enable to echo the linker command line
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -v")

    # This is creating an error during linking: offset out of range
    # TODO: investigate updating tool chain
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
else()
    #TODO: Is this needed for coverage?
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker -lgcov")
endif()


# ----------------------------------------------------- GLOBAL COMPILER OPTIONS

# https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html

# Size optimization
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)
    add_compile_options(-Os)
else()
    add_compile_options(-O0)
endif()

# Architecture specific warning levels
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)
    add_compile_options(-Wno-unused-but-set-variable)
    # Make paths in __FILE__ relative to current directory
    add_compile_options(-ffile-prefix-map=${CMAKE_CURRENT_LIST_DIR}=.)
else()
    add_compile_options(-Wno-int-to-pointer-cast)
    add_compile_options(-Wno-pointer-to-int-cast)
endif()

# Warning levels
add_compile_options(-Wall)
add_compile_options(-Werror)
add_compile_options(-Wno-unused-parameter)
add_compile_options(-Wno-unused-variable)

# Debug symbols only appear in the elf file, not in the bin or hex files
add_compile_options(-g3)
add_compile_options(-ggdb)

# Prevent GCC from replacing standard C function calls with its own
add_compile_options(-fno-builtin)

# Separate data / function sections, garbage collect unused ones during linking
add_compile_options(-ffunction-sections)
add_compile_options(-fdata-sections)
add_compile_options(-fno-strict-aliasing)

# Enable link time optimization

# This is creating an error during linking: offset out of range
# TODO: investigate updating tool chain
#add_compile_options(-flto)

# ----------------------------------------------- PRINT GLOBAL COMPILER OPTIONS
get_directory_property(TEMP_COMPILE_OPTIONS COMPILE_OPTIONS)
message(STATUS "COMPILE_OPTIONS         :  " ${TEMP_COMPILE_OPTIONS})


# ------------------------------------------------------ PRINT C COMPILER FLAGS
message(STATUS "CMAKE_C_FLAGS           : " ${CMAKE_C_FLAGS})


# ---------------------------------------------------- PRINT C++ COMPILER FLAGS
message(STATUS "CMAKE_CXX_FLAGS         : " ${CMAKE_CXX_FLAGS})


# ---------------------------------------------------------- PRINT LINKER FLAGS
message(STATUS "CMAKE_EXE_LINKER_FLAGS  : " ${CMAKE_EXE_LINKER_FLAGS})


# ------------------------------------------------------------ INCLUDE PROJECTS
if((${CMAKE_SYSTEM_NAME}      MATCHES Generic) AND
  (${CMAKE_SYSTEM_PROCESSOR} MATCHES arm))
    add_subdirectory(application)
    add_subdirectory(stm32cubemx)
    add_subdirectory(common)
    add_subdirectory(SEGGER_RTT)
endif()
