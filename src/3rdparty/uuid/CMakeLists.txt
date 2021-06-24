project(uuid C)

set(SOURCE_FILES
    clear.c
    compare.c
    copy.c
    gen_uuid.c
    isnull.c
    pack.c
    parse.c
    randutils.c
    test_uuid.c
    unpack.c
    unparse.c
    uuid_time.c
)

add_library(uuid ${SOURCE_FILES})

include(CheckIncludeFile)

check_include_file(fcntl.h HAVE_FCNTL_H)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(limits.h HAVE_LIMITS_H)
check_include_file(netinet/in.h HAVE_NETINET_IN_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(sys/file.h HAVE_SYS_FILE_H)
check_include_file(sys/ioctl.h HAVE_SYS_IOCTL_H)
check_include_file(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(sys/syscall.h HAVE_SYSCALL_H)

include(CheckFunctionExists)

check_function_exists(ftruncate HAVE_FTRUNCATE)
check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)
check_function_exists(memset HAVE_MEMSET)
check_function_exists(socket HAVE_SOCKET)
check_function_exists(strtoul HAVE_STRTOUL)
check_function_exists(usleep HAVE_USLEEP)
check_function_exists(srandom HAVE_SRANDOM)

include(CheckSymbolExists)

set(CMAKE_REQUIRED_QUIET 1)
check_include_file(sys/sysconf.h HAVE_SYS_SYSCONF_H)
set(CMAKE_REQUIRED_QUIET 0)
if(HAVE_SYS_SYSCONF_H)
    check_symbol_exists(_SC_HOST_NAME_MAX sys/sysconf.h HAVE__SC_HOST_NAME_MAX)
endif()

include(CheckTypeSize)

set(CMAKE_EXTRA_INCLUDE_FILES "stdint.h")

if(HAVE_INTTYPES_H)
    list(APPEND CMAKE_EXTRA_INCLUDE_FILES "inttypes.h")
endif()

if(HAVE_STDLIB_H)
    list(APPEND CMAKE_EXTRA_INCLUDE_FILES "stdlib.h")
endif()

set(CMAKE_REQUIRED_QUIET 1)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
set(CMAKE_REQUIRED_QUIET 0)
if(HAVE_SYS_STAT_H)
    list(APPEND CMAKE_EXTRA_INCLUDE_FILES "sys/stat.h")
endif()

check_type_size(int32_t INT32_T)
check_type_size(mode_t MODE_T)
check_type_size(size_t SIZE_T)
check_type_size(ssize_t SSIZE_T)
check_type_size(uint16_t UINT16_T)
check_type_size(uint32_t UINT32_T)
check_type_size(uint64_t UINT64_T)
check_type_size(uint8_t UINT8_T)
check_type_size(loff_t LOFF_T)

function(RemoveNotSetVarList listName)
    set(list)
    foreach(var ${ARGN})
        if(var)
            list(APPEND list ${var})
        endif()
    endforeach(var)
    set(${listName} ${list} PARENT_SCOPE)
endfunction(RemoveNotSetVarList)

function(CheckAll)
    foreach(var ${ARGN})
        if(NOT var)
            message(FATAL_ERROR "${var} is false")
        endif()
    endforeach(var)
endfunction(CheckAll)

RemoveNotSetVarList(ExistHeaderList
    HAVE_FCNTL_H
    HAVE_INTTYPES_H
    HAVE_LIMITS_H
    HAVE_NETINET_IN_H
    HAVE_STDLIB_H
    HAVE_STRING_H
    HAVE_SYS_FILE_H
    HAVE_SYS_IOCTL_H
    HAVE_SYS_SOCKET_H
    HAVE_SYS_TIME_H
    HAVE_UNISTD_H
)

RemoveNotSetVarList(ExistFunctionList
    HAVE_FTRUNCATE
    HAVE_GETTIMEOFDAY
    HAVE_MEMSET
    HAVE_SOCKET
    HAVE_STRTOUL
    HAVE_USLEEP
    HAVE_SRANDOM
)

RemoveNotSetVarList(ExistMacroList
    HAVE__SC_HOST_NAME_MAX
)

CheckAll(
    HAVE_INT32_T
    HAVE_MODE_T
    HAVE_SIZE_T
    HAVE_SSIZE_T
    HAVE_UINT16_T
    HAVE_UINT32_T
    HAVE_UINT64_T
    HAVE_UINT8_T
    HAVE_LOFF_T
)

target_compile_definitions(uuid PRIVATE
    ${ExistHeaderList}
    ${ExistFunctionList}
    ${ExistMacroList}
)

target_include_directories(uuid INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:.>
)
install(TARGETS uuid DESTINATION lib)
file(COPY uuid.h DESTINATION ${CMAKE_BINARY_DIR}/include/uuid)
