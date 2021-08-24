#!/usr/bin/env python3
"""
Build script an STM32 application generated from CubeMX.
Requires Make and CMake.
"""

import os
import sys

# Set multi-OS variables
if sys.platform == 'win32':
    RMDIR_CMD = 'rmdir /S /Q'
    MKDIR_CMD = 'mkdir'
    MULTI = '&'
else:
    RMDIR_CMD = 'rm -rf'
    MKDIR_CMD = 'mkdir -p'
    MULTI = '&&'

PWD = os.getcwd()
SEP = os.sep

THIS_SCRIPT_PATH = os.path.dirname(os.path.abspath(__file__)) + SEP
BUILD_ROOT_PATH = THIS_SCRIPT_PATH + "build"

def run_command(cmd):
    """
    Runs a command and immediately exits if it fails
    """
    ret = os.system(cmd)
    if ret > 0:
        sys.exit(1)

def create_build(platform, toolchain_file, debug, options):
    """
    Create the build path and generate CMake outputs
    """
    build_path = BUILD_ROOT_PATH + SEP + platform
    cmake_args = f"-DDEBUG=ON {options}"
    cmake_args = f"{cmake_args} -DCMAKE_BUILD_TYPE=Debug"

    # Enable the verbose debug build
    if debug:
        cmake_args = f"{cmake_args} -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON"

    # Create the build path if it doesn't exist
    if not os.path.exists(build_path):
        print("Creating build path")
        run_command(f"{MKDIR_CMD} {build_path}")

    cmake_args = f"-G\"Unix Makefiles\" -DCMAKE_TOOLCHAIN_FILE:PATH={toolchain_file} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON {cmake_args}"

    print("!!!!!!!!!!!!!!!!!!!!!!")
    print(f"cd {build_path} {MULTI} cmake {cmake_args} {THIS_SCRIPT_PATH}")
    print("######################")

    run_command(f"cd {build_path} {MULTI} cmake {cmake_args} {THIS_SCRIPT_PATH}")

def make_all_builds():
    """
    Run Make on previously generated CMake outputs
    """
    print("#################################################################")
    print(f"# Building in: {BUILD_ROOT_PATH}{SEP}Arm")
    print("#################################################################")

    run_command(f"cd {BUILD_ROOT_PATH}{SEP}Arm {MULTI} make -j16")

# Main function
def main():
    """
    Program entry point
    """
    clean = False
    make_all = False
    is_debug = False
    is_debug_pins = False

    # String of any CMake options set directly by command line args
    options = ""

    for arg in sys.argv:
        # CMake generation or generation+build
        if arg == "make":
            make_all = True
        # Clean
        if arg == "clean":
            clean = True
        # Verbose debug build
        if arg == "debug":
            is_debug = True

        # Verbose debug build
        if arg == "debug_pins":
            is_debug_pins = True

    if is_debug_pins:
        options = f"{options}-DDEBUG_PINS=ON "
    else:
        options = f"{options}-DDEBUG_PINS=OFF "

    if clean:
        if os.path.exists(BUILD_ROOT_PATH):
            run_command(f"{RMDIR_CMD} build{SEP}Arm")
            print("Build directory cleaned")
        else:
            print("Build path not found")

    # If clean was the only argument, don't build
    if (clean and len(sys.argv) > 2) or (not clean):
        create_build("Arm", f"{THIS_SCRIPT_PATH}{SEP}toolchain{SEP}arm_embedded_toolchain.txt", is_debug, options)

    if make_all:
        make_all_builds()

if __name__=="__main__":
    main()
