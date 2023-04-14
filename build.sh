#处理参数，规范化参数
ARGS=`getopt -a -o p:b:h:: --long product:,build::,help:: -- "$@"`
#echo $ARGS
#将规范化后的命令行参数分配至位置参数（$1,$2,...)
eval set -- "${ARGS}"

declare -A TOOLCHAINS #key/value dict ,key is platform,value is toolchain,key must be uppercase
TOOLCHAINS["SIGMA"]=$HOME/cdroid/cmake/ssd202-mtitoolchain.cmake
TOOLCHAINS["ALI3528"]=$HOME/cdroid/cmake/ali3528-mtitoolchain.cmake

declare -A DEPLIBS #key/value dict,key is platform,value is deplibs dir in vcpkg,key must be uppercase
DEPLIBS["X64"]=$HOME/vcpkg/installed/x64-linux-dynamic
DEPLIBS["SIGMA"]=$HOME/vcpkg/installed/arm-linux-dynamic

SHOWHELP=false
PRODUCT="x64"
BUILD_TYPE="Release"

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
                SHOWHELP=true
                echo "showhelp"
                shift
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
CDROID_DIR=$HOME/cdroid/out${PRODUCT}-${BUILD_TYPE}

echo "product=$PRODUCT ${PRODUCT,,}"
echo "showhelp=$SHOWHELP"
echo "build=${BUILD_TYPE}"

if [ "$PRODUCT" = "X64" ]; then
    echo "x64"
    TOOLCHAIN_FILE=""
elif [ "$PRODUCT" = "SIGMA" ]; then
    TOOLCHAIN_FILE="-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAINS[${PRODUCT}]}"
fi

OUTDIR=out${PRODUCT}-${BUILD_TYPE}
DEPLIBS_DIR=${DEPLIBS[$PRODUCT]}

if [ "$PRODUCT" != "X64" ]; then
    echo "TOOLCHAIN_FILE=${TOOLCHAIN_FILE}"
fi

if [ SHOWHELP ];then
    echo ""
    echo "Usage: $0 [options] $#"
    echo "-P|--product [x64,sigma] default is x64"
    echo "-b|--build[Debug,Release,RelWithDebInfo,MinSizeRel]"
    echo "-h|--help Show Help Info,Usage"
    echo ""
fi
echo "DEPLIBS_DIR=${DEPLIBS_DIR} product=$PRODUCT TOOLCHAIN_FILE=${TOOLCHAIN_FILE}"
#exit

mkdir -p ${OUTDIR}
pushd ${OUTDIR}

export PKG_CONFIG_PATH=$DEPLIBS_DIR/lib/pkgconfig
export PKG_CONFIG_LIBDIR=$DEPLIBS_DIR/lib/pkgconfig
echo PKG_CONFIG_PATH=${PKG_CONFIG_PATH}
cmake ${TOOLCHAIN_FILE} \
   -DCMAKE_INSTALL_PREFIX=./ \
   -DCMAKE_PREFIX_PATH=${DEPLIBS_DIR} \
   -DCDROID_CHIPSET=${PRODUCT,,} \
   -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
   ..
