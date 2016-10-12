cmake_minimum_required(VERSION 2.8)

project(kfusion)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(CUDA REQUIRED)
find_package(TooN REQUIRED)
find_package(OpenGL)
find_package(Freenect)
find_package(GLUT)


# avoid gcc
set(CUDA_HOST_COMPILER clang++)
#set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_20,code=sm_20;--std=c++11)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${TOON_INCLUDE_PATHS}
    )


#cuda_add_executable(devicequery
#        devicequery.cpp
#        )

#target_link_libraries(devicequery
#    ${CUDA_LIBRARIES}
#    )


message(STATUS "${CUDA_LIBRARIES}")


set(kfusion_cuda_srcs
    helpers.cu
    helpers.h
    kfusion.cu
    kfusion.h
    cutil_math.h
    )

cuda_add_library(kfusion STATIC
    ${kfusion_cuda_srcs}
    perfstats.h
    OPTIONS
    -O3
    )

if (GLUT_FOUND)

    include_directories(${GLUT_INCLUDE_DIR})

    add_executable(kfusion_test
        test.cpp
        )

    target_link_libraries(kfusion_test kfusion ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES})

    if(FREENECT_FOUND)
        include_directories(${FREENECT_INCLUDE_PATHS})
        add_definitions(-DLIBFREENECT_INTERFACE)
        add_executable(kfusion_kinect
            kinect.cpp interface.h interface.cpp
            )
        target_link_libraries(kfusion_kinect kfusion ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES} ${FREENECT_LIBRARIES})
    else()

        include_directories($ENV{KINECTSDK10_DIR}/inc)
        add_definitions(-DMS_KINECT_INTERFACE)
        add_executable(kfusion_kinect
            kinect.cpp interface.h interface.cpp
            )
        target_link_libraries(kfusion_kinect kfusion ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES} $ENV{KINECTSDK10_DIR}/lib/amd64/Kinect10.lib)
    endif()

endif()
