cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

# use CMake detection of CUDA
project(kfusion LANGUAGES CXX CUDA)

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
#find_package(LibRealSense)

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


message("'OpenNI2: ${OPENNI2_INCLUDES}'")
message("'CUDA: ${CUDA_INCLUDE_DIRS}'")

set_target_properties(kfusion PROPERTIES POSITION_INDEPENDENT_CODE ON)

set_target_properties(kfusion PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

set_target_properties(kfusion PROPERTIES INCLUDE_DIRECTORIES "${CUDA_INCLUDE_DIRS};${TOON_INCLUDE_PATHS}")

target_compile_features(kfusion PUBLIC cxx_std_11)


#
#
#
set(kfusion_libs)
set(kfusion_incs "${CUDA_INCLUDE_DIRS};${TOON_INCLUDE_PATHS}")
set(kfusion_defs)
set(kfusion_srcs)


option(KFUSION_SCAN_ENABLE_FREENECT "Enable Kinect/Freenect Interface" OFF)
option(KFUSION_SCAN_ENABLE_OPENNI2 "Enable Kinect/OpenNI2 Interface" OFF)
#option(KFUSION_SCAN_ENABLE_FREENECT "Enable Kinect/Freenect Interface" OFF)


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

configure_file(interface_config.hpp.cmake
    ${CMAKE_BINARY_DIR}/include/interface_config.hpp
    )

add_executable(kfusion_kinect
    kinect.cpp
    interface.h
    interface.cpp
    interface_config.hpp.cmake
    ${CMAKE_BINARY_DIR}/include/interface_config.hpp
    ${kfusion_srcs}
    )


set_target_properties(kfusion_kinect PROPERTIES
    INCLUDE_DIRECTORIES
    "${kfusion_incs};${CMAKE_BINARY_DIR}/include"
    )

target_link_libraries(
    kfusion_kinect
    kfusion
    ${GLUT_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${kfusion_libs}
    )


add_executable(kfusion_test
    test.cpp
    )

target_link_libraries(
    kfusion_test
    kfusion
    ${GLUT_LIBRARIES}
    ${OPENGL_LIBRARIES}
    )

#if (GLUT_FOUND)

#    include_directories(${GLUT_INCLUDE_DIR})

#    add_executable(kfusion_test
#        test.cpp
#        )

#    target_link_libraries(kfusion_test kfusion ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

#    if(FREENECT_FOUND)
#        include_directories(${kfusion_incs})
#        add_definitions(-DLIBFREENECT_INTERFACE)
#        add_executable(kfusion_kinect
#            kinect.cpp
#            interface.h
#            interface.cpp
#            interface_kinect.hpp
#            interface_kinect.cpp
##            interface_librealsense.hpp
##            interface_librealsense.cpp
##            interface_openni2.hpp
##            interface_openni2.cpp
#            )

#        target_link_libraries(kfusion_kinect
#            kfusion
#            ${GLUT_LIBRARIES}
#            ${OPENGL_LIBRARIES}
#            ${FREENECT_LIBRARIES}
##            realsense
#            ${CMAKE_THREAD_LIBS_INIT}
##            ${OPENNI2_LIBRARY}
#            )


#        install(TARGETS kfusion_kinect kfusion_test
#            RUNTIME DESTINATION bin
#            LIBRARY DESTINATION bin
#            )


#    else()

#        include_directories($ENV{KINECTSDK10_DIR}/inc)
#        add_definitions(-DMS_KINECT_INTERFACE)
#        add_executable(kfusion_kinect
#            kinect.cpp interface.h interface.cpp
#            )
#        target_link_libraries(kfusion_kinect kfusion ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES} $ENV{KINECTSDK10_DIR}/lib/amd64/Kinect10.lib)
#    endif()

#endif()
