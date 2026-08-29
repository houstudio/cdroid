# Device-side package-management CLI tools — the AOSP frameworks/base/cmds
# role: pm (install/uninstall/list, the PackageManagerService face) and am
# (start/force-stop, the ActivityManagerService face) for adb-installable pak
# bundles. Both link cdroid ONLY for ZIPArchive + XmlPullParser (bundle
# manifest / packages.xml parsing); they never instantiate App and pull no
# graphics stack.
project(cdroid-cmds CXX)

set(CMAKE_CXX_STANDARD 14)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/gui
    ${CMAKE_SOURCE_DIR}/src/gui/private
    ${CMAKE_SOURCE_DIR}/src/porting/include
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/include/gui
    ${CMAKE_BINARY_DIR}/include/porting
)

foreach(tool pm am)
    add_executable(${tool} ${tool}/main.cc packagedb.cc)
    target_link_libraries(${tool} PRIVATE cdroid)
    install(TARGETS ${tool} DESTINATION bin)
endforeach()
