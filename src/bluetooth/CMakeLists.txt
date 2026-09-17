cmake_minimum_required(VERSION 3.8)
project(cdblue CXX)

# cdblue — android.bluetooth port on BlueZ, laid out by the android package
# tree (the cdnet template):
#   bluetoothadapter.cc   android.bluetooth.BluetoothAdapter
#   bluetoothdevice.cc    android.bluetooth.BluetoothDevice
#   bluetoothclass.h      android.bluetooth.BluetoothClass (CoD constants)
#   bluezclient.cc        sd-bus transport to bluetoothd (org.bluez)
# Self-contained: does not link cdroid, so it can be consumed (and later
# graduated into src/gui/bluetooth) without touching other modules.
#
# Dependency: libsystemd (sd-bus) from the vcpkg installed tree — the same
# classic-mode resolution every other dependency uses (CMAKE_PREFIX_PATH).

find_path(SDBUS_INCLUDE_DIR systemd/sd-bus.h)
find_library(SDBUS_LIBRARY NAMES systemd)
# sd-bus has no library of its own — its symbols live inside libsystemd (see
# bluezclient.h), which this project takes from the vcpkg installed tree via
# classic-mode CMAKE_PREFIX_PATH. Without this guard a missing libsystemd
# leaks the literal "SDBUS_LIBRARY-NOTFOUND" into the link line
# (-lSDBUS_LIBRARY-NOTFOUND) — and a tree that has the headers but not the
# library half-resolves (find_path ok, find_library NOTFOUND), so report what
# was found.
if(NOT SDBUS_INCLUDE_DIR OR NOT SDBUS_LIBRARY)
    message(FATAL_ERROR
        "cdblue needs libsystemd from the vcpkg installed tree (sd-bus lives "
        "inside libsystemd, there is no separate sdbus library). Install it "
        "with 'vcpkg install libsystemd --triplet <triplet>' and configure "
        "with the tree on CMAKE_PREFIX_PATH. Found: "
        "SDBUS_INCLUDE_DIR='${SDBUS_INCLUDE_DIR}', "
        "SDBUS_LIBRARY='${SDBUS_LIBRARY}'")
endif()

add_library(cdblue
    internal/sdppdu.cc
    internal/sdpclient.cc
    bluetoothadapter.cc
    bluetoothdevice.cc
    bluetoothsocket.cc
    bluetoothgatt.cc
    bluetoothle.cc
    bluetoothprofile.cc
    bluetoothpan.cc
    bluezclient.cc)
target_include_directories(cdblue PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDBUS_INCLUDE_DIR}
    ${CDROID_INCLUDE_DIRS}
    ${CDROID_DEPINCLUDES}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/include/gui
    ${CMAKE_BINARY_DIR}/include/porting)
target_link_libraries(cdblue PUBLIC pthread ${SDBUS_LIBRARY})

add_executable(bttest bttest.cc)
target_link_libraries(bttest cdblue)

# org.bluez stand-in for the no-hardware transport bench (dbus-run-session).
add_executable(btmock btmock.cc)
target_link_libraries(btmock ${SDBUS_LIBRARY})
target_include_directories(btmock PRIVATE ${SDBUS_INCLUDE_DIR})

# Pure-logic suite (no daemon / privileges required) — the wifitests analog.
add_executable(bluetoothtests bluetoothtests.cc)
target_link_libraries(bluetoothtests cdblue)
