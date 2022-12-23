#cmake_minimum_required (VERSION 3.8)
project(haltest C CXX)
find_package(GTEST REQUIRED)
set(SRCS_NORMAL
   event_unittests.cc  
   mem_unittests.cc    
   sem_unittests.cc    
   graph_unittest.cc   
   msgq_unittests.cc   
   testmain.cc         
   input_unittests.cc  
   mutex_unittests.cc  
   timer_unittests.cc  
   )
set(SRCS_DTV
   tvtestutils.cc
   dmx_unittests.cc  
   tspes_unittests.cc  
   av_unittests.cc   
   tuner_unittests.cc)

set(SRCS_HAL ${SRCS_NORMAL})
if ( ENABLE_DTV )
   set(SRCS_HAL ${SRCS_NORMAL} ${SRCS_DTV})
endif(ENABLE_DTV)

include_directories(
   ./
   ${GTEST_INCLUDE_DIRS}
   ${CMAKE_BINARY_DIR}/includegtest
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/porting
   ${CMAKE_BINARY_DIR}/include/rfb
   ${CMAKE_BINARY_DIR}/include/pixman
)

link_directories(${CMAKE_BINARY_DIR}/lib)

add_executable(hal_tests ${SRCS_HAL}) 

target_link_libraries(hal_tests tvhal dl ${GTEST_LIBRARIES})

install (TARGETS hal_tests  DESTINATION bin/tests)
