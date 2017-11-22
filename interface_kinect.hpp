#ifndef INTERFACE_KINECT_HPP
#define INTERFACE_KINECT_HPP


#include "interface.h"

#include <libfreenect/libfreenect.h>
#include <pthread.h>



class KinectDevice : public RGBD {

    freenect_context *f_ctx;
    freenect_device *f_dev;
    bool gotDepth;

    pthread_t freenect_thread;
    bool die;

    // RGBD interface
public:

    KinectDevice();

    int open();
    bool available() const;
    int update();
    void close();

    bool stopped() const { return this->die; }

    void setDirty(bool isDirty) { gotDepth = isDirty; }
    bool dirty() const { return gotDepth; }


    void setDepthBuffer();
};



#endif

