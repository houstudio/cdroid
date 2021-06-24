1.environment
   1.1 modify toolchain.cmake to set corrent toolchain params
   1.2 modify build.sh and set NGL_CHIPSET to correct porting(chipset)

2.build testing
  2.1. run  build.sh  /*build makefile*/
  2.2. cd out
  2.3. make hal_tests //gtest's testcase for porting api
  2.4. make 3rdtest  //main.cc for your own test entry
  notice:gx3213 cant make hal to so,so default make will failed.

3. how to use gtest 
  3.1 --gtest_list_tests  //show all testcase
  3.2 --gtest_filter=OSEVENT* //run all testcase in event_unittests.cc
  3.3 --gtest_filter=DMX.xxx  //run xxx case in dmx_unittests.cc
