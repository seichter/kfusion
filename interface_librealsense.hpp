#ifndef INTERFACE_LIBREALSENSE_HPP
#define INTERFACE_LIBREALSENSE_HPP


#include "interface.h"

#include <librealsense/rs.h>
#include <pthread.h>
#include <iostream>


class RealSenseDevice : public RGBD {

    rs_context *ctx;
    rs_device *dev;

    bool gotDepth;

    pthread_t _thread;
    bool die;

    double lastTimeStamp;

    float depthScale;

    rs_intrinsics intrinsics;

    // RGBD interface
public:

    RealSenseDevice();

    int open();
    bool available() const;
    int update();
    void close();

    bool stopped() const { return this->die; }

    void setDirty(bool isDirty) { gotDepth = isDirty; }
    bool dirty() const { return gotDepth; }


    void setDepthBuffer();

    float focalX() const;
    float focalY() const;


};



#endif

