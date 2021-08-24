#!/bin/bash

# --------------------------------- Get the directory that this script lives in
SCRIPT_WORKING_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
PROJECT_DIR=${SCRIPT_WORKING_DIR}/..


# ------------------------------------------------ Add the J-Link tools to path
# source ${SCRIPT_WORKING_DIR}/set_paths.sh


# ----------------------------------------------------- Start the J-Link server
JLinkGDBServer -device STM32H730VB -if SWD -speed 4000
