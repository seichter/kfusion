cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

# use CMake detection of CUDA
project(kfusion LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# internal stuff
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# get all required packages
find_package(TooN REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(Threads REQUIRED)
find_package(CUDA REQUIRED)

# capture libs for depth cameras
find_package(Freenect)
find_package(OpenNI2)
find_package(LibRealSense)
find_package(KinectSDK)

# general IO
include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${TOON_INCLUDE_PATHS}
    ${CUDA_INCLUDE_DIRS}
    )

# build CUDA backend library
add_library(kfusion STATIC
    helpers.cu
    helpers.h
    kfusion.cu
    kfusion.h
    cutil_math.h
    README.md
    )


target_link_libraries(kfusion
	${CUDA_LIBRARIES}
	)

#message("'OpenNI2: ${OPENNI2_INCLUDES}'")
#message("'CUDA: ${CUDA_INCLUDE_DIRS}'")
#message("'GLUT: ${GLUT_INCLUDE_DIR}'")

#
# skip these for Windows
#
if (NOT WIN32)

	set_target_properties(kfusion PROPERTIES POSITION_INDEPENDENT_CODE ON)

	set_target_properties(kfusion PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

endif()

set_target_properties(kfusion PROPERTIES INCLUDE_DIRECTORIES "${CUDA_INCLUDE_DIRS};${TOON_INCLUDE_PATHS}")

#
# Libs
#
set(kfusion_libs ${CUDA_CUDA_LIBRAY})
set(kfusion_incs "${CUDA_INCLUDE_DIRS};${TOON_INCLUDE_PATHS}")
set(kfusion_defs)
set(kfusion_srcs)


option(KFUSION_SCAN_ENABLE_FREENECT "Enable Kinect/Freenect Interface" OFF)
option(KFUSION_SCAN_ENABLE_OPENNI2 "Enable Kinect/OpenNI2 Interface" OFF)
option(KFUSION_SCAN_ENABLE_REALSENSE "Enable Realsense Interface" OFF)
option(KFUSION_SCAN_ENABLE_MSKINECT1 "Enable MS Kinect SDK 1.0" OFF)


if (FREENECT_FOUND AND KFUSION_SCAN_ENABLE_FREENECT)

    list(APPEND kfusion_libs ${FREENECT_LIBRARIES})
    list(APPEND kfusion_incs ${FREENECT_INCLUDE_PATHS})
    list(APPEND kfusion_srcs interface_kinect.hpp interface_kinect.cpp)

    set(KFUSION_INTERFACE_HAVE_FREENECT TRUE)

endif()

if (OPENNI2_FOUND AND KFUSION_SCAN_ENABLE_OPENNI2)

    list(APPEND kfusion_libs ${OPENNI2_LIBRARY})
    list(APPEND kfusion_incs ${OPENNI2_INCLUDES})
    list(APPEND kfusion_srcs interface_openni2.hpp interface_openni2.cpp)

    set(KFUSION_INTERFACE_HAVE_OPENNI2 TRUE)

endif()


if (REALSENSE_FOUND AND KFUSION_SCAN_ENABLE_REALSENSE)

    list(APPEND kfusion_libs ${REALSENSE_LIBRARIES})
    list(APPEND kfusion_incs ${REALSENSE_INCLUDE_PATHS})
    list(APPEND kfusion_srcs interface_librealsense.hpp interface_librealsense.cpp)

    set(KFUSION_INTERFACE_HAVE_LIBREALSENSE TRUE)


endif()


if (KinectSDK_FOUND AND KFUSION_SCAN_ENABLE_MSKINECT1)

    list(APPEND kfusion_libs ${KinectSDK_LIBRARIES})
    list(APPEND kfusion_incs ${KinectSDK_INCLUDE_DIRS})
    list(APPEND kfusion_srcs interface_mskinect.hpp interface_mskinect.cpp)

	set(KFUSION_INTERFACE_HAVE_MSKINECT1 TRUE)

endif()


configure_file(interface_config.hpp.cmake
    ${CMAKE_BINARY_DIR}/include/interface_config.hpp
    )

add_executable(kfusion_scan
    ${CMAKE_BINARY_DIR}/include/interface_config.hpp
    kinect.cpp
    interface.h
    interface.cpp
    interface_config.hpp.cmake
    ${kfusion_srcs}
    )


set_target_properties(kfusion_scan PROPERTIES
    INCLUDE_DIRECTORIES
    "${kfusion_incs};${CMAKE_BINARY_DIR}/include;${GLUT_INCLUDE_DIR}"
    )

target_link_libraries(
    kfusion_scan
    kfusion
    ${GLUT_LIBRARIES}
    ${OPENGL_LIBRARIES}
	${kfusion_libs}
    )

# message("LIBS : ${kfusion_libs}")
# message("LIBS : ${OPENGL_LIBRARIES}")
# message("LIBS : ${GLUT_LIBRARIES}")
# message("CUDA : ${CUDA_LIBRARIES}")


add_executable(kfusion_test
    test.cpp
    )

set_target_properties(kfusion_test PROPERTIES
    INCLUDE_DIRECTORIES
    "${kfusion_incs};${CMAKE_BINARY_DIR}/include;${GLUT_INCLUDE_DIR}"
    )

target_link_libraries(
    kfusion_test
    kfusion
    ${GLUT_LIBRARIES}
    ${OPENGL_LIBRARIES}
	${CUDA_LIBRARIES}
    )

