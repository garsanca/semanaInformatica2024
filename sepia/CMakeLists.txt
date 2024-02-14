cmake_minimum_required(VERSION 3.22.1)
project(SEPIA LANGUAGES CXX)

set(CMAKE_CXX_COMPILER icpx)

set(This sepia)

set(Headers
)

set(Source
    sepia.cpp
)

set(EXTRA_FLAGS "")

if(DEBUG)
    message(STATUS "Added -g to compilation.")
    set(EXTRA_FLAGS "-g")
endif()

find_package(OpenCV REQUIRED)
set(CXXFLAGS "-O3" "-fsycl" ${EXTRA_FLAGS})
set(LXXFLAGS "-fsycl-device-code-split=per_kernel")

if(NGPU)
    list(APPEND CXXFLAGS "-fsycl-targets=nvidia_gpu_sm_61")
endif()

include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(${This} ${Source} ${Headers})
target_compile_options(${This} PRIVATE ${CXXFLAGS})

target_link_directories(${This} PRIVATE)
target_link_libraries(${This} ${OpenCV_LIBS} ${CXXFLAGS} ${LXXFLAGS} sycl)
