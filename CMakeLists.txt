cmake_minimum_required(VERSION 3.5)
project(dp_jetson_board)
subdirs(darknet)
set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV 3.4.0 REQUIRED)
find_package(CUDA REQUIRED)

include_directories(
        ${OpenCV_INCLUDE_DIRS}
        ${CUDA_INCLUDE_DIRS}
)

add_executable(dp_jetson_board
        main.cpp
        yolo_detector.cpp
        yolo_detector.hpp)

target_link_libraries( dp_jetson_board ${OpenCV_LIBS} cuda cudart cublas curand)
