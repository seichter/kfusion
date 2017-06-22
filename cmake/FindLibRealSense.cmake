
set(LIBRARY_PATHS
	~/usr/lib
	~/usr/local/lib
	/usr/lib
	/usr/local/lib
	)

find_library(REALSENSE_LIBRARY 
	NAMES realsense
	PATHS ${LIBRARY_PATHS}
	)
	
find_path(REALSENSE_INCLUDE_PATH librealsense/rs.h
 	~/usr/include
	~/usr/local/include
	/usr/include
	/usr/local/include
	)
	
find_path(LIBUSB1_INCLUDE_PATH libusb-1.0/libusb.h
 	~/usr/include
	~/usr/local/include
	/usr/include
	/usr/local/include
	)
	
if(REALSENSE_LIBRARY AND REALSENSE_INCLUDE_PATH)
	set(REALSENSE_FOUND TRUE)
	set(REALSENSE_INCLUDE_PATHS ${LIBUSB1_INCLUDE_PATH} ${REALSENSE_INCLUDE_PATH} CACHE STRING "The include paths needed to use freenect")
    set(REALSENSE_LIBRARIES ${REALSENSE_LIBRARY} CACHE STRING "The libraries needed to use freenect")
endif()

mark_as_advanced(
    REALSENSE_INCLUDE_PATHS
    REALSENSE_LIBRARIES
	)
