enable_testing()
include_directories(${PNG_INCLUDE_DIR})

if(UNIX)
    set(SYSTEM_LIBRARIES "m")
    if(THREADS_FOUND)
        set(SYSTEM_LIBRARIES "${SYSTEM_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}")
    endif(THREADS_FOUND)
endif(UNIX)

add_library (tests_core OBJECT utils.c utils-prng.c)

add_executable(a1-trap-test a1-trap-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(a1-trap-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(a1-trap-test a1-trap-test)
set_tests_properties (a1-trap-test PROPERTIES TIMEOUT 100)

add_executable(affine-bench affine-bench.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(affine-bench ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(affine-bench affine-bench)
set_tests_properties (affine-bench PROPERTIES TIMEOUT 100)

add_executable(affine-test affine-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(affine-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(affine-test affine-test)
set_tests_properties (affine-test PROPERTIES TIMEOUT 100)

add_executable(alpha-loop alpha-loop.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(alpha-loop ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(alpha-loop alpha-loop)
set_tests_properties (alpha-loop PROPERTIES TIMEOUT 100)

add_executable(alphamap alphamap.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(alphamap ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(alphamap alphamap)
set_tests_properties (alphamap PROPERTIES TIMEOUT 100)

add_executable(blitters-test blitters-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(blitters-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(blitters-test blitters-test)
set_tests_properties (blitters-test PROPERTIES TIMEOUT 100)

add_executable(check-formats check-formats.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(check-formats ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(check-formats check-formats)
set_tests_properties (check-formats PROPERTIES TIMEOUT 100)

add_executable(combiner-test combiner-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(combiner-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(combiner-test combiner-test)
set_tests_properties (combiner-test PROPERTIES TIMEOUT 100)

add_executable(composite composite.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(composite ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(composite composite)
set_tests_properties (composite PROPERTIES TIMEOUT 100)

add_executable(composite-traps-test composite-traps-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(composite-traps-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(composite-traps-test composite-traps-test)
set_tests_properties (composite-traps-test PROPERTIES TIMEOUT 100)

add_executable(cover-test cover-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(cover-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(cover-test cover-test)
set_tests_properties (cover-test PROPERTIES TIMEOUT 100)

add_executable(fence-image-self-test fence-image-self-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(fence-image-self-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(fence-image-self-test fence-image-self-test)
set_tests_properties (fence-image-self-test PROPERTIES TIMEOUT 100)

add_executable(fetch-test fetch-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(fetch-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(fetch-test fetch-test)
set_tests_properties (fetch-test PROPERTIES TIMEOUT 100)

add_executable(glyph-test glyph-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(glyph-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(glyph-test glyph-test)
set_tests_properties (glyph-test PROPERTIES TIMEOUT 100)

add_executable(gradient-crash-test gradient-crash-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(gradient-crash-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(gradient-crash-test gradient-crash-test)
set_tests_properties (gradient-crash-test PROPERTIES TIMEOUT 100)

add_executable(infinite-loop infinite-loop.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(infinite-loop ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(infinite-loop infinite-loop)
set_tests_properties (infinite-loop PROPERTIES TIMEOUT 100)

add_executable(lowlevel-blt-bench lowlevel-blt-bench.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(lowlevel-blt-bench ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(lowlevel-blt-bench lowlevel-blt-bench)
set_tests_properties (lowlevel-blt-bench PROPERTIES TIMEOUT 100)

add_executable(matrix-test matrix-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(matrix-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(matrix-test matrix-test)
set_tests_properties (matrix-test PROPERTIES TIMEOUT 100)

add_executable(oob-test oob-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(oob-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(oob-test oob-test)
set_tests_properties (oob-test PROPERTIES TIMEOUT 100)

add_executable(pdf-op-test pdf-op-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(pdf-op-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(pdf-op-test pdf-op-test)
set_tests_properties (pdf-op-test PROPERTIES TIMEOUT 100)

add_executable(pixel-test pixel-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(pixel-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(pixel-test pixel-test)
set_tests_properties (pixel-test PROPERTIES TIMEOUT 100)

add_executable(prng-test prng-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(prng-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(prng-test prng-test)
set_tests_properties (prng-test PROPERTIES TIMEOUT 100)

add_executable(radial-perf-test radial-perf-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(radial-perf-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(radial-perf-test radial-perf-test)
set_tests_properties (radial-perf-test PROPERTIES TIMEOUT 100)

add_executable(region-contains-test region-contains-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(region-contains-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(region-contains-test region-contains-test)
set_tests_properties (region-contains-test PROPERTIES TIMEOUT 100)

add_executable(region-test region-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(region-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(region-test region-test)
set_tests_properties (region-test PROPERTIES TIMEOUT 100)

add_executable(region-translate-test region-translate-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(region-translate-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(region-translate-test region-translate-test)
set_tests_properties (region-translate-test PROPERTIES TIMEOUT 100)

add_executable(rotate-test rotate-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(rotate-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(rotate-test rotate-test)
set_tests_properties (rotate-test PROPERTIES TIMEOUT 100)

add_executable(scaling-bench scaling-bench.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(scaling-bench ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(scaling-bench scaling-bench)
set_tests_properties (scaling-bench PROPERTIES TIMEOUT 100)

add_executable(scaling-crash-test scaling-crash-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(scaling-crash-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(scaling-crash-test scaling-crash-test)
set_tests_properties (scaling-crash-test PROPERTIES TIMEOUT 100)

add_executable(scaling-helpers-test scaling-helpers-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(scaling-helpers-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(scaling-helpers-test scaling-helpers-test)
set_tests_properties (scaling-helpers-test PROPERTIES TIMEOUT 100)

add_executable(scaling-test scaling-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(scaling-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(scaling-test scaling-test)
set_tests_properties (scaling-test PROPERTIES TIMEOUT 100)

add_executable(stress-test stress-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(stress-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(stress-test stress-test)
set_tests_properties (stress-test PROPERTIES TIMEOUT 100)

add_executable(thread-test thread-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(thread-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(thread-test thread-test)
set_tests_properties (thread-test PROPERTIES TIMEOUT 100)

add_executable(tolerance-test tolerance-test.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(tolerance-test ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(tolerance-test tolerance-test)
set_tests_properties (tolerance-test PROPERTIES TIMEOUT 100)

add_executable(trap-crasher trap-crasher.c $<TARGET_OBJECTS:tests_core>)
target_link_libraries(trap-crasher ${SYSTEM_LIBRARIES} ${PNG_LIBRARY} pixman-1_static)
add_test(trap-crasher trap-crasher)
set_tests_properties (trap-crasher PROPERTIES TIMEOUT 100)
