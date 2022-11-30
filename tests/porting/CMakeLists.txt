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
   ${DEPS_DIR}/includegtest
   ${DEPS_DIR}/include
   ${DEPS_DIR}/include/gui
   ${DEPS_DIR}/include/porting
   ${DEPS_DIR}/include/rfb
   ${DEPS_DIR}/include/pixman
)

message("HAL_EXTRA_LIBS=${HAL_EXTRA_LIBS}")
link_directories(${DEPS_DIR}/lib)

add_executable(hal_tests ${SRCS_HAL}) 
add_executable(hal_testsd ${SRCS_HAL}) 

target_link_libraries(hal_tests tvhal_static ${GTEST_LIBRARIES})
target_link_libraries(hal_testsd tvhal $${GTEST_LIBRARIES})#GTest::gtest)

install (TARGETS hal_tests  DESTINATION bin/tests)
