
add_library(mpp_ge SHARED
    cmdq_ops.c
    normal_ops.c
    mpp_ge.c
)

set_target_properties(mpp_ge PROPERTIES FOLDER "mpp_ge")

target_include_directories(mpp_ge PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(DEFINED CMAKE_INSTALL_FULL_LIBDIR)
    install(TARGETS mpp_ge RUNTIME DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}")
endif() # CMAKE_INSTALL_FULL_LIBDIR
