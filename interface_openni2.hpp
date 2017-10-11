#ifndef KFUSION_INTERFACE_OPENNI2
#define KFUSION_INTERFACE_OPENNI2


#include "interface.h"

#include <OpenNI.h>
#include <pthread.h>


class OpenNIDevice : public RGBD {

    openni::Device device;
    openni::VideoStream depth_stream;
    openni::VideoStream color_stream;

    bool gotDepth;

    pthread_t openni_thread;
    bool die;

    // RGBD interface
public:

    OpenNIDevice();

    int open();
    bool available() const;
    int update();
    void close();

    bool stopped() const { return this->die; }

    void setDirty(bool isDirty) { gotDepth = isDirty; }
    bool dirty() const { return gotDepth; }


    void setDepthBuffer();

    openni::VideoStream& getDepthStream();
};

#endif
