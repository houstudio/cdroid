project(achart C CXX)

include_directories(
    ../../
)

add_executable(acharttest main.cc)
target_link_libraries(acharttest cdroid)
