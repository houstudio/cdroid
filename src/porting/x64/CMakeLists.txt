project (x86 C CXX)

set(X64_SRCS
    ../common/cdlog.cc
    ngl_panel.c
    ngl_mediaplayer.c
    input_linux.cc
   )

if (ENABLE_DTV)
    list(APPEND X64_SRCS  ngl_dsc.c ngl_smc.c  ngl_tuner.c ngl_dmx.cc
         ngl_nvm.c  ngl_pvr.c ngl_snd.c ngl_video.c dmxreader.cc
         ngl_disp.c ngl_os.c
        )
    endif()

    include(CheckIncludeFile)
    check_include_file(poll.h HAVE_POLL_H)
    check_include_file(sys/epoll.h  HAVE_EPOLL_H)
    check_include_file(linux/input.h HAVE_INPUT_H)
    check_include_file(execinfo.h HAVE_EXECINFO_H)
    check_include_file(drm.h HAVE_DRM_H)

    if(DRM_LIBRARY AND DRM_INCLUDE_DIR)
        list(APPEND X64_LIBRARIES $ {DRM_LIBRARY})
        endif()

        if(HAVE_POLL_H)
            list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_POLL_H)
            endif()
            if(HAVE_EPOLL_H)
                list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_EPOLL_H)
                endif()
                if(HAVE_INPUT_H)
                    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_INPUT_H)
                    endif()
                    if(HAVE_EXECINFO_H)
                        list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_EXECINFO_H)
                        endif()

                        find_package(PkgConfig)
                        find_package(SDL2)
                        find_package(DirectFB)
                        find_package(X11)

                        if(FALSE AND DIRECTFB_FOUND)
                            list(APPEND X64_INCLUDE_DIRS $ {DIRECTFB_INCLUDE_DIRS})
                            list(APPEND X64_LIBRARIES $ {DIRECTFB_LIBRARIES})
                            list(APPEND GRAPH_SRCS ../common/graph_dfb.c)
                            elseif (X11_FOUND)
                            list(APPEND GRAPH_SRCS graph_xlib.c)
                            list(APPEND X64_LIBRARIES $ {X11_LIBRARIES})
                            list(APPEND X64_INCLUDE_DIRS $ {X11_INCLUDE_DIR})
                            elseif (SDL2_FOUND)
                            list(APPEND X64_INCLUDE_DIRS $ {SDL2_INCLUDE_DIRS})
                            list(APPEND X64_LIBRARIES $ {SDL2_LIBRARIES})
                            list(APPEND GRAPH_SRCS graph_sdl.c)
                            else()
                                message(FATAL_ERROR "graph must implemented")
                                endif()

                                if(NOT GRAPH_SRCS)
                                    find_package(ZLIB)
                                    add_definitions(-DENABLE_RFB=1)
                                    list(APPEND GRAPH_SRCS graph_rfb.c ../common/rfbcommon.cc)
                                    list(APPEND X64_LIBRARIES vncserver)
                                    endif()

                                    include_directories(./
                                                        ../common ../include
                                                        $ {CMAKE_SOURCE_DIR}/src/gui/
                                                        $ {CMAKE_SOURCE_DIR}/src/gui/core
                                                        $ {X64_INCLUDE_DIRS}
                                                       )
                                    message("CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR} GRAPH_SRCS=${GRAPH_SRCS}")
                                    add_library(tvhal SHARED $ {X64_SRCS} $ {GRAPH_SRCS})
                                    target_link_libraries(tvhal PRIVATE $ {X64_LIBRARIES})

                                    set(PREFIX $ {CMAKE_INSTALL_PREFIX})
                                    set(NONPKG_LIBS "-L${CMAKE_INSTALL_PREFIX}/lib")
                                    configure_file(cdroidhal.pc.in cdroidhal.pc @ONLY)
                                    install(FILES $ {CMAKE_CURRENT_BINARY_DIR}/cdroidhal.pc
                                            DESTINATION $ {CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
                                    install (TARGETS tvhal  DESTINATION lib)

