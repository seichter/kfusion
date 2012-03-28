cmake_minimum_required(VERSION 2.8)

project(kfusion)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_subdirectory(dep/TooN)

include_directories(
	${CMAKE_BINARY_DIR}/include
)

find_package(CUDA REQUIRED)
find_package(Freenect)
find_package(GLUT)

cuda_add_executable(devicequery
	devicequery.cpp
	)


set(kfusion_cuda_srcs
	helpers.cu
	helpers.h
	kfusion.cu
	kfusion.h
	cutil_math.h
	)
	
cuda_add_library(kfusion SHARED
	${kfusion_cuda_srcs}
	perfstats.h
	OPTIONS
	-O3
	)

if (GLUT_FOUND AND FREENECT_FOUND)		
	target_link_libraries(kfusion 
		cvd ${ext_libs} ${FREENECT_LIBRARIES} ${OPENGL_LIBRARIES}
		)
	
	add_executable(kfusion_kinect
		kinect.cpp
		)

	target_link_libraries(kfusion_kinect kfusion) 

endif()


if(GLUT_FOUND)
	add_executable(kfusion_test
		test.cpp
		)
		
		
	target_link_libraries(kfusion_test kfusion) 
endif()
	