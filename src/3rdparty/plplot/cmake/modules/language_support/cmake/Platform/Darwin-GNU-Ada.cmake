include(${CMAKE_ROOT}/Modules/Platform/Darwin-GNU.cmake)
__darwin_compiler_gnu(Ada)
cmake_gnu_set_sysroot_flag(Ada)
cmake_gnu_set_osx_deployment_target_flag(Ada)
