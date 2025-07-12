#ifndef INTERFACE_LIBREALSENSE_HPP
#define INTERFACE_LIBREALSENSE_HPP

#include <librealsense/rs.h>
#include <pthread.h>


#include "interface.h"


class RealSenseDevice : public RGBD {

    rs_context *ctx = nullptr;
    rs_device *dev = nullptr;

    bool gotDepth = false;

    pthread_t _thread {};
    bool die = false;

    double lastTimeStamp {};

    float depthScale = float(1);

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
