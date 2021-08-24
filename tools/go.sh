#!/bin/bash
rm -rf ./build
./generate_arm_build.py make debug_pins

if [[ $? -eq 0 ]]; then
    # Erase the chip and flash the application
    tools/flash.sh -e -j -f build/Arm/application/application.hex
    # Start an JLinkExe console session.  This will allow RTT to work and allow
    # us to enter commands
    tools/flash.sh -c -j
fi
