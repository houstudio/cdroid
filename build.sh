#!/bin/bash
#处理参数，规范化参数
ARGS=`getopt -a -o p:b:h:: --long product:,build::,options::,help:: -- "$@"`
#echo $ARGS
#将规范化后的命令行参数分配至位置参数（$1,$2,...)
eval set -- "${ARGS}"
TOPDIR=$(dirname "$(readlink -f "$0")")
declare -A TOOLCHAINS #key/value dict ,key is platform,value is toolchain,key must be uppercase

if [ -n "$VCPKG_ROOT" ]; then
    VCPKGROOT="$VCPKG_ROOT"
elif [ -n "$VCPKGROOT" ]; then
    VCPKGROOT="$VCPKGROOT"
elif [ -d "$HOME/vcpkg" ]; then
    VCPKGROOT="$HOME/vcpkg"
elif [ -d "/opt/vcpkg" ]; then
    VCPKGROOT=/opt/vcpkg
fi

if [ ! -f "$VCPKGROOT/vcpkg" ] && [ ! -f "$VCPKGROOT/vcpkg.exe" ] && [ ! -f "$VCPKGROOT/vcpkg.bat" ]; then
    echo "vcpkg not found"
    exit 1
fi

TOOLCHAINS["SIGMA"]=${VCPKGROOT}/scripts/toolchains/ssd202-mtitoolchain.cmake
TOOLCHAINS["ALI3528"]=${VCPKGROOT}/scripts/toolchains/ali3528-mtitoolchain.cmake
TOOLCHAINS["R818"]=${VCPKGROOT}/scripts/toolchains/r818-toolchain.cmake
TOOLCHAINS["D211"]=${VCPKGROOT}/scripts/toolchains/riscv64-d211-toolchain.cmake
TOOLCHAINS["HI3536"]=${VCPKGROOT}/scripts/toolchains/hisi3536-toolchain.cmake
TOOLCHAINS["INGENIC"]=${VCPKGROOT}/scripts/toolchains/ingenic-x2600-toolchain.cmake
TOOLCHAINS["TINAT113"]=${VCPKGROOT}/scripts/toolchains/tinat113-toolchain.cmake
TOOLCHAINS["RK3506"]=${VCPKGROOT}/scripts/toolchains/rk3506-toolchain.cmake
TOOLCHAINS["SSD2351"]=${VCPKGROOT}/scripts/toolchains/ssd2351-toolchain.cmake
TOOLCHAINS["RK1126"]=${VCPKGROOT}/scripts/toolchains/rk1126-toolchain.cmake
TOOLCHAINS["ANDROID"]=${VCPKGROOT}/scripts/buildsystems/vcpkg.cmake
declare -A DEPLIBS #key/value dict,key is platform,value is deplibs dir in vcpkg,key must be uppercase

DEPLIBS["X64"]=${VCPKGROOT}/installed/x64-linux-dynamic
DEPLIBS["SIGMA"]=${VCPKGROOT}/installed/sigma-linux-dynamic
DEPLIBS["R818"]=${VCPKGROOT}/installed/r818-linux
DEPLIBS["D211"]=${VCPKGROOT}/installed/riscv64-d211-linux
DEPLIBS["HI3536"]=${VCPKGROOT}/installed/hisi3536-linux
DEPLIBS["INGENIC"]=${VCPKGROOT}/installed/ingenic-linux
DEPLIBS["TINAT113"]=${VCPKGROOT}/installed/tinat113-linux-dynamic
DEPLIBS["RK3506"]=${VCPKGROOT}/installed/rk3506-linux-dynamic
DEPLIBS["SSD2351"]=${VCPKGROOT}/installed/ssd2351-linux-dynamic
DEPLIBS["RK1126"]=${VCPKGROOT}/installed/rk1126-linux-dynamic
DEPLIBS["WIN32"]=${VCPKGROOT}/installed/x64-windows:${VCPKGROOT}/installed/x64-windows-release
DEPLIBS["ANDROID"]=${VCPKGROOT}/installed/arm64-android

OSNAME=""
if [ "$(uname)" = "Linux" ]; then
    OSNAME="x64"
elif [ -d "/c" ]; then
    OSNAME="win32"
fi
CDROID_VALID_PORTS="${OSNAME}"
SHOWHELP=0
PRODUCT="${OSNAME}"
BUILD_TYPE="Release"

for key in "${!TOOLCHAINS[@]}"
do
  CDROID_VALID_PORTS="${CDROID_VALID_PORTS},$key"
done

while :
do
   case $1 in
        -p|--product)
                PRODUCT=$2
                echo "product=$PRODUCT"
                shift
                ;;
        -b|--build)
                BUILD_TYPE=$2
                echo "build=$BUILD_TYPE"
                shift
                ;;
        -h|--help)
                SHOWHELP=1
                echo "showhelp"
                shift
                ;;
        --options)
                OPTIONS_FILE="../$2"
                ;;
        --)
                shift
                break
                ;;
        *)
                break
                ;;
   esac
   shift
done

PORT=${PORT^^}
PRODUCT=${PRODUCT^^}
BUILD_TYPE=${BUILD_TYPE,,}
BUILD_TYPE=${BUILD_TYPE^}
OPTIONS_FILE="../${PRODUCT,,}.txt"
CDROID_DIR=${TOPDIR}/out${PRODUCT}-${BUILD_TYPE}
echo "VALID_PORTS=${CDROID_VALID_PORTS}"
echo "product=$PRODUCT ${PRODUCT,,}"
echo "build=${BUILD_TYPE}/${BUILD_TYPE,,}"

if [ "$PRODUCT" = "X64" ] || [ "$PRODUCT" = "WIN32" ]; then
    echo "x64"
    TOOLCHAIN_FILE=""
elif [ "$PRODUCT" != "X64" ] && [ "$PRODUCT" != "WIN32" ]; then
    TOOLCHAIN_FILE=${TOOLCHAINS[${PRODUCT}]}
    if [ "$TOOLCHAIN_FILE" = "" ]; then
       SHOWHELP=1
    fi
fi

OUTDIR=out${PRODUCT}-${BUILD_TYPE}
DEPLIBS_DIR=${DEPLIBS[$PRODUCT]}


#Debug version'sDEPLIB seems has some trouble in some platform(r818)
if [ "${BUILD_TYPE,,}" = "debug" ]; then
   DEPLIBS_DIR="${DEPLIBS_DIR}" #/debug:${DEPLIBS_DIR}"
fi

echo "DEPLIBS_DIR=${DEPLIBS_DIR} product=$PRODUCT"
echo "TOOLCHAIN_FILE=${TOOLCHAIN_FILE} SHOWHELP=${SHOWHELP}"
echo "========DEPLIBS_DIR=${DEPLIBS_DIR} BUILDTYPE=${BUILD_TYPE}"
export PATH=$DEPLIBS_DIR:$PATH

if [ $SHOWHELP -gt 0 ] ;then
    echo "Usage: $0 [options] $#"
    echo "-P|--product [${CDROID_VALID_PORTS}] default is x64"
    echo "-b|--build[Debug,Release,RelWithDebInfo,MinSizeRel]"
    echo "-h|--help Show Help Info,Usage"
    echo ""
    exit
fi

mkdir -p ${OUTDIR}
pushd ${OUTDIR}

export PKG_CONFIG_PATH=$DEPLIBS_DIR/lib/pkgconfig
export PKG_CONFIG_LIBDIR=$DEPLIBS_DIR/lib/pkgconfig
echo PKG_CONFIG_PATH=${PKG_CONFIG_PATH}

# Create cmake -D options
CMAKE_SWITCHES=""
echo "OPTIONS_FILE=$OPTIONS_FILE"
if [ -f "$OPTIONS_FILE" ]; then
    echo "Fetch options from '$OPTIONS_FILE' "
    while IFS= read -r line; do
        if [[ -n "$line" && ! "$line" =~ ^# ]]; then
            key="${line%%=*}"
            value="${line#*=}"
            expanded_value=$(echo "$value" | envsubst)
            CMAKE_SWITCHES+=" -D${key}=${expanded_value}"
            echo ${key}=${expanded_value}
        fi
    done < "$OPTIONS_FILE"
    echo "$CMAKE_SWITCHES"
fi

cmake \
    -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE} \
    -DCMAKE_INSTALL_PREFIX=./ \
    -DCMAKE_PREFIX_PATH=${DEPLIBS_DIR} \
    -DCMAKE_MODULE_PATH=${DEPLIBS_DIR} \
    -DCDROID_CHIPSET=${PRODUCT,,} \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    ${CMAKE_SWITCHES} \
        ..
