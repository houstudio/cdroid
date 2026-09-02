# testkit — the in-process test stack that lives under gui/app next to
# autotest/uiautomation but stays OUT of libcdroid (its own SHARED lib):
# product apps simply don't link it. Current content: the Espresso port
# (framework sources -> the `testkit` library; main.cc is the host demo/test
# runner). (uiautomation/autotest --auto-test remain inside libcdroid: they
# are the framework-side bridge and the sweep driver.)
project(testkit C CXX)

set(CMAKE_CXX_STANDARD 14)
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()

# Mirror src/gui's include set (the framework reaches gui internals by header).
include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_SOURCE_DIR}/src/gui
   ${CMAKE_SOURCE_DIR}/src/gui/content/i18n
   ${CMAKE_SOURCE_DIR}/src/gui/private
   ${CMAKE_SOURCE_DIR}/src/gui/core
   ${CMAKE_SOURCE_DIR}/src/gui/view
   ${CMAKE_SOURCE_DIR}/src/gui/cairomm
   ${CMAKE_SOURCE_DIR}/src/gui/image-decoders
   ${CMAKE_SOURCE_DIR}/src/porting/include
   ${CMAKE_SOURCE_DIR}/src/gui/text/minikin/include
   ${CMAKE_SOURCE_DIR}/src/3rdparty/pinyin/include
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/src/gui
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/porting
   ${CMAKE_BINARY_DIR}/include/pinyin
   ${CDROID_DEPINCLUDES}
   ${LIBUNIBREAK_INCLUDE_DIRS}
)

# The Espresso framework as a library: apps that embed in-process espresso
# tests link `testkit`; product apps that don't, don't. SHARED keeps it out of
# libcdroid while staying dlopen-free for the embedding app.
add_library(testkit SHARED
   hamcrest.cc
   espressoexception.cc
   queueinterrogator.cc
   uicontrollerimpl.cc
   idlingresource.cc
   root.cc
   rootsoracle.cc
   rootmatchers.cc
   rootviewpicker.cc
   viewfinder.cc
   treeiterables.cc
   humanreadables.cc
   viewassertions.cc
   defaultfailurehandler.cc
   viewmatchers.cc
   motionevents.cc
   generallocation.cc
   press.cc
   tap.cc
   swipe.cc
   viewactions.cc
   viewinteraction.cc
   adapterviewprotocol.cc
   adapterviewprotocols.cc
   adapterdataloaderaction.cc
   datainteraction.cc
   espresso.cc
   espressotests.cc
)
target_link_libraries(testkit cdroid)
# Consumers include the framework headers directly (<espresso.h> & friends).
target_include_directories(testkit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# The in-process Espresso host (framework demo/test runner).
add_executable(espresso main.cc)
target_link_libraries(espresso testkit cdroid)
