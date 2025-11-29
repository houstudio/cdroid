# FindXCB.cmake

# 查找XCB库
find_path(XCB_INCLUDE_DIR NAMES xcb/xcb.h)
find_library(XCB_LIBRARY NAMES xcb)

# 设置XCB_FOUND变量
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XCB REQUIRED_VARS XCB_INCLUDE_DIR XCB_LIBRARY)

# 如果找到了XCB库，则设置XCB_LIBRARIES和XCB_INCLUDE_DIRS
if(XCB_FOUND)
    set(XCB_LIBRARIES ${XCB_LIBRARY})
    set(XCB_INCLUDE_DIRS ${XCB_INCLUDE_DIR})
endif()

# 标记为高级变量
mark_as_advanced(XCB_INCLUDE_DIR XCB_LIBRARY)
