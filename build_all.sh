#!/bin/bash

generator=make

# usage
function usage {
    echo "Usage: "
    echo -e "\t--generator          <build system [codelite-][ninja/make]>          default=${generator}"
    echo
}

# read arguments
while [ "$1" != "" ]; do
    case $1 in
        --generator)    shift
                        generator=$1
                        ;;
        -h | --help)    usage
                        exit
                        ;;
        * )             echo "unrecognized option $1"
                        echo
                        usage
                        exit 1
    esac
    shift
done

generator=$(echo ${generator} | tr '[:upper:]' '[:lower:]')
if [ "${generator}" == "ninja" ]; then
    generator="Ninja"
    maker="ninja"
elif [ "${generator}" == "make" ]; then
    generator="Unix Makefiles"
    maker="make"
else
    echo "Invalid generator"
    usage
    echo
    exit 1
fi

baseDir=$(pwd)

# Web Debug
mkdir -p build_debug_web
cd build_debug_web
cmake -G "${generator}" -DNANO_BROWSER=ON -DCMAKE_BUILD_TYPE=Debug $baseDir
${maker}
cd $baseDir

# Web Release
mkdir -p build_release_web
cd build_release_web
cmake -G "${generator}" -DNANO_BROWSER=ON -DCMAKE_BUILD_TYPE=Release $baseDir
${maker}
cd $baseDir

# Desktop Debug
mkdir -p build_debug_desktop
cd build_debug_desktop
cmake -G "${generator}" -DNANO_BROWSER=OFF -DCMAKE_BUILD_TYPE=Debug $baseDir
${maker}
cd $baseDir

# Desktop Release
mkdir -p build_release_desktop
cd build_release_desktop
cmake -G "${generator}" -DNANO_BROWSER=OFF -DCMAKE_BUILD_TYPE=Release $baseDir
${maker}
cd $baseDir
