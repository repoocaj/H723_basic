#!/usr/bin/env bash

set -Eeuo pipefail
trap cleanup SIGINT SIGTERM ERR EXIT

# The directory that the script is located in
script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd -P)

# The root directory of the project.  Assumed to be one level above the
# directory containing this script.
root_dir=$(dirname "${script_dir}")

usage() {
  cat <<EOF
Usage:

    ST-LINK:
    $(basename "${BASH_SOURCE[0]}") [-h] [-v] [-s]

    J-Link:
    $(basename "${BASH_SOURCE[0]}") [-h] [-v] [-j] [-r] [-n|-t] [-sn <value>]

Flash a .hex file to the module using a J-Link or ST-Link.

Available options:

-h,  --help             Print this help and exit
-v,  --verbose          Print script debug info
     --no-color         Disable color output
---------------
-f,  --file             File to load

ST-LINK options
---------------

-s,  --stlink           Flash using ST-LINK

J-Link options
--------------

-j,  --jlink            Flash using J-Link
-r,  --rtt              Stay connected after flashing so that RTT communication can occur
-o,  --olimex           Device is a STM32F405RG on a OLIMEX board. Default uses prototype STM32H730VB.
-sn, --serial_number    Flash using the J-Link with the given serial number
-c,  --console          Start the J-Link console
-e,  --erase            Erase the chip

EOF
  exit
}

cleanup() {
  trap - SIGINT SIGTERM ERR EXIT
  # Kill the J-Link app if it's running
  [[ ! ${jlink_pid} -eq 0 ]] && kill ${jlink_pid}
}

setup_colors() {
  if [[ -t 2 ]] && [[ -z "${NO_COLOR-}" ]] && [[ "${TERM-}" != "dumb" ]]; then
    NOFORMAT='\033[0m' RED='\033[0;31m' GREEN='\033[0;32m' ORANGE='\033[0;33m' BLUE='\033[0;34m' PURPLE='\033[0;35m' CYAN='\033[0;36m' YELLOW='\033[1;33m'
  else
    NOFORMAT='' RED='' GREEN='' ORANGE='' BLUE='' PURPLE='' CYAN='' YELLOW=''
  fi
}

msg() {
  echo >&2 -e "${1-}"
}

die() {
  local msg=$1
  local code=${2-1} # default exit status 1
  msg "$msg"
  exit "$code"
}

parse_params() {
  # default values of variables set from params
  stlink=0
  jlink=0
  rtt=0
  serial_number=''
  jlink_pid=0
  device=''
  file=''
  erase=0
  console=0

  while :; do
    case "${1-}" in
    -h | --help) usage ;;
    -v | --verbose) set -x ;;
    --no-color) NO_COLOR=1 ;;
    -f | --file)
      file="${2-}"
      shift
      ;;
    -e | --erase) erase=1 ;;
    -s | --stlink) stlink=1 ;;
    -j | --jlink) jlink=1 ;;
    -r | --rtt) rtt=1 ;;
    -c | --console) console=1 ;;
    -o | --olimex) device='STM32F405RG' ;;
    -sn | --serial_number)
      serial_number="${2-}"
      shift
      ;;
    -?*) die "Unknown option: $1" ;;
    *) break ;;
    esac
    shift
  done

  args=("$@")

  # check required params and arguments
  [[ ${jlink} -eq 0 ]] && [[ ${stlink} -eq 0 ]] && die "J-Link or ST-Link must be specified"
  [[ ${jlink} -eq 1 ]] && [[ ${stlink} -eq 1 ]] && die "Only one of J-Link or ST-LINK can be specified"
  if [[ ${stlink} -eq 1 ]]; then
    # Verify no J-Link arguments given with ST-LINK specified
    [[ ${rtt} -eq 1 ]] && die "-r is only valid with J-Link"
    [[ ! -z ${serial_number} ]] && die "-sn is only valid with J-Link"
    [[ ! -z ${device} ]] && die "-n|-t is only valid with J-Link"
  fi

  if [[ ${jlink} -eq 1 ]]; then
    # Default to the OLIMEX board if -n or -t not given
    [[ -z ${device} ]] && device='STM32H730VB'
  fi

  return 0
}

verify_app_exists() {
    # Verify the application.hex file exists
    if [ -f "${file}" ]; then
        APP=${file}
    else  
        APP="${root_dir}"/build/Arm/application/application.hex
    fi

    if [ -f "${APP}" ]; then
        msg "${APP} found"
    else
        msg "${RED}${APP} does not exist${NOFORMAT}"
        exit 1
    fi
}

flash_stlink() {
    msg "${ORANGE}Flashing via ST-LINK${NOFORMAT}"

    # The latest version (v1.6.1) causes issues on the STM32F4
    DESIRED_VERSION="v1.6.0"
    INSTALLED_VERSION=$(st-flash --version)
    
    if [ "$DESIRED_VERSION" != "$INSTALLED_VERSION" ]; then
        msg "${RED}Programming requires version v1.6.0${NOFORMAT}"
        exit 1
    fi

    verify_app_exists

    st-flash --reset --format ihex write "${APP}"
}

erase_jlink() {
    msg "${ORANGE}Erasing via J-Link${NOFORMAT}"

    interface="SWD"
    speed="4000"
    auto=1

    args="-device ${device} -if ${interface} -speed ${speed} -autoconnect ${auto}"
    # args="-if ${interface} -speed ${speed} -autoconnect ${auto}"
    if [[ ! -z "${serial_number}" ]]; then
        args="-SelectEmuBySN ${serial_number} ${args}"
    fi

    # Commands to send to the J-Link to erase
    commands="erase
        q"

    echo "${commands}" | JLinkExe ${args} &
    jlink_pid=$!

    wait ${jlink_pid}
    jlink_pid=0
}

console_jlink() {
    if [[ ${console} -eq 1 ]]; then
        interface="SWD"
        speed="4000"
        auto=1

        args="-device ${device} -if ${interface} -speed ${speed} -autoconnect ${auto}"
        # args="-if ${interface} -speed ${speed} -autoconnect ${auto}"
        if [[ ! -z "${serial_number}" ]]; then
            args="-SelectEmuBySN ${serial_number} ${args}"
        fi

        JLinkExe ${args}
    fi
}

rtt_jlink() {
    if [[ ${rtt} -eq 1 ]]; then
        interface="SWD"
        speed="4000"
        auto=1

        args="-device ${device} -if ${interface} -speed ${speed} -autoconnect ${auto}"
        # args="-if ${interface} -speed ${speed} -autoconnect ${auto}"
        if [[ ! -z "${serial_number}" ]]; then
            args="-SelectEmuBySN ${serial_number} ${args}"
        fi

        # Commands to send to the J-Link to reset and start running
        commands="r
            g"

        echo "${commands}" | JLinkExe ${args} &
        jlink_pid=$!

        # Delay long enough for the commands to take effect so that the message
        # is the last thing on a terminal
        sleep 2

        msg "${ORANGE}RTT monitoring active${NOFORMAT}"
        msg "${ORANGE}Use JLinkRTTClient or similar RTT Viewer to see log output${NOFORMAT}"
        msg "${ORANGE}^C to exit${NOFORMAT}"

        wait ${jlink_pid}
        jlink_pid=0
    fi
}

flash_jlink() {
    msg "${ORANGE}Flashing via J-Link${NOFORMAT}"

    verify_app_exists

    interface="SWD"
    speed="4000"
    auto=1

    args="-device ${device} -if ${interface} -speed ${speed} -autoconnect ${auto}"
    # args="-if ${interface} -speed ${speed} -autoconnect ${auto}"
    if [[ ! -z "${serial_number}" ]]; then
        args="-SelectEmuBySN ${serial_number} ${args}"
    fi

    # Commands to send to the J-Link to flash the binary.  We're assuming that
    # the chip has already been erased.
    commands="loadfile ${APP}
        q"

    echo "${commands}" | JLinkExe ${args}
}

parse_params "$@"
setup_colors

# script logic here

if [[ ${jlink} -eq 1 ]]; then 
    if [[ ${erase} -eq 1 ]]; then
        erase_jlink
    fi

    if [[ ! -z "${file}" ]]; then
        flash_jlink
    fi

    if [[ ${rtt} -eq 1 ]]; then
        rtt_jlink
    fi

    if [[ ${console} -eq 1 ]]; then
        console_jlink
    fi
elif [[ ${stlink} -eq 1 ]]; then
    flash_stlink
fi
