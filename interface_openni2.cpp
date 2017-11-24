#include "interface_openni2.hpp"




#include <iostream>
#include <stdexcept>

//using namespace std;
//using namespace openni;



//Device device;
//VideoStream depth_stream;
//VideoStream color_stream;
//bool gotDepth = false; // set to true as soon as the first depth frame is received
//int depth_index = 0; // for flipping between the depth double buffers

//pthread_t openni_thread; // thread for the readFrame() loop
//volatile bool die = false; // tells the readFrame() loop to stop

// We use OpenNI frame allocators to let it write new images directly
// into our buffers (one for RGB and a double-buffer for depth) so that
// we don't have to memcpy each frame.

class KFusionDepthFrameAllocator : public openni::VideoStream::FrameAllocator
{
private:
    uint16_t * depth_buffers_[2];
public:
    KFusionDepthFrameAllocator(uint16_t * depth_buffers[2])
    {
        depth_buffers_[0] = depth_buffers[0];
        depth_buffers_[1] = depth_buffers[1];
    }
    void *allocateFrameBuffer(int size)
    {
        if (size != 640*480*2) {
            std::cout << "KFusionDepthFrameAllocator size request of " << size << " (should be " << 640*480*2 << ")" << std::endl;
            throw std::runtime_error("KFusionDepthFrameAllocator got bad size request, currently only supports 640*480*2");
        }
//        return depth_buffers_[depth_index];
    }

    // We have static buffers, nothing to do.
    void freeFrameBuffer(void *data) {}
};

class KFusionColorFrameAllocator : public openni::VideoStream::FrameAllocator
{
private:
    unsigned char * rgb_buffer_;
public:
    KFusionColorFrameAllocator(unsigned char * rgb_buffer)
    {
        rgb_buffer_ = rgb_buffer;
    }
    void *allocateFrameBuffer(int size)
    {
        if (size != 640*480*3) {
            std::cout << "KFusionColorFrameAllocator size request of " << size << " (should be " << 640*480*3 << ")" << std::endl;
            throw std::runtime_error("KFusionColorFrameAllocator got bad size request, currently only supports 640*480*3");
        }
        return rgb_buffer_;
    }

    // We have static buffers, nothing to do.
    void freeFrameBuffer(void *data) {}
};

// This thread continuously reads depth and RGB images from the camera
// using the blocking readFrame().
// We could have used OpenNI::waitForAnyStream() as an alternative,
// but there is no direct benefit of it.
void *openni_threadfunc(void *arg)
{
    OpenNIDevice* device = static_cast<OpenNIDevice*>(arg);

    while(!device->stopped()){
        device->update();
    }

    device->close();

    return NULL;
}

OpenNIDevice::OpenNIDevice()
    : openni_thread(0L)
    , die(false)
    , gotDepth(false)
{
}

int OpenNIDevice::open()
{

    std::cout << "Opening " << std::endl;

    using namespace openni;

    // The allocators must survive this initialization function.
    KFusionDepthFrameAllocator *depthAlloc = new KFusionDepthFrameAllocator(depth_buffer);
    KFusionColorFrameAllocator *colorAlloc = new KFusionColorFrameAllocator(rgb_buffer);

    Status status = STATUS_OK;

    // Initialize OpenNI
    status = OpenNI::initialize();

    if (status != STATUS_OK) {

        std::cerr << "OpenNI: Initialize failed" << std::endl;

        OpenNI::shutdown();

        return 1;
    }

    std::cout << "Initialized ... " << std::endl;

    // Check if a camera is connected
    Array<openni::DeviceInfo> deviceList;
    OpenNI::enumerateDevices(&deviceList);
    int nr_devices = deviceList.getSize();

    if(nr_devices < 1) {
        std::cout << "OpenNI: No devices found" << std::endl;
        OpenNI::shutdown();
        return 1;
    }

    std::cout << "Devices " << nr_devices << std::endl;

    // Open device
    status = device.open(ANY_DEVICE);

    if (status != STATUS_OK) {
        std::cerr << "OpenNI: Could not open device: " << OpenNI::getExtendedError() << std::endl;
        OpenNI::shutdown();
        return 1;
    }

    std::cout << "Device "
              << device.getDeviceInfo().getVendor() << " "
              << device.getDeviceInfo().getName() << " "
              << device.getDeviceInfo().getUri()
              << std::endl;

    // Create depth stream
    if (device.getSensorInfo(SENSOR_DEPTH) != NULL) {
        status = depth_stream.create(device, SENSOR_DEPTH);
        if (status != STATUS_OK) {

            std::cerr << "OpenNI: Could not create depth stream " <<  OpenNI::getExtendedError() << std::endl;

            OpenNI::shutdown();

            return 1;
        }

        const openni::SensorInfo* si = device.getSensorInfo(SENSOR_DEPTH);

        const Array<openni::VideoMode>& vms = si->getSupportedVideoModes();

         for (int i = 0; i < vms.getSize();i++) {
             std::cout << "Depth " << vms[i].getResolutionX() << "x" << vms[i].getResolutionY()
                       << " " << vms[i].getFps() << " " << vms[i].getPixelFormat()
                       << std::endl;
         }

    }

    // Create color stream
    if (device.getSensorInfo(SENSOR_COLOR) != NULL) {

       const openni::SensorInfo* si = device.getSensorInfo(SENSOR_COLOR);

       const Array<openni::VideoMode>& vms = si->getSupportedVideoModes();

        for (int i = 0; i < vms.getSize();i++) {
            std::cout << vms[i].getResolutionX() << "x" << vms[i].getResolutionY()
                      << " " << vms[i].getFps() << " " << vms[i].getPixelFormat()
                      << std::endl;
        }

        status = color_stream.create(device, SENSOR_COLOR);
        if (status != STATUS_OK) {
            std::cerr << "OpenNI: Could not create color stream" << OpenNI::getExtendedError() << std::endl;
            OpenNI::shutdown();
            return 1;
        }
    }


    std::cout << "Line " << __LINE__ << std::endl;

    // Choose what depth format we want from the camera
    VideoMode depth_mode;

    depth_mode.setPixelFormat(PIXEL_FORMAT_DEPTH_1_MM);
    depth_mode.setResolution(640, 480);
    depth_mode.setFps(30);

    status = depth_stream.setVideoMode(depth_mode);
    if (status != STATUS_OK) {

        std::cerr << "OpenNI: Could not set depth video mode:" << OpenNI::getExtendedError() << std::endl;
        OpenNI::shutdown();
        return 1;
    }

    // Choose what color format we want from the camera
    VideoMode color_mode;
    color_mode.setPixelFormat(PIXEL_FORMAT_RGB888);
    color_mode.setResolution(640, 480);
    color_mode.setFps(30);
    status = color_stream.setVideoMode(color_mode);
    if (status != STATUS_OK) {
        std::cerr  << "OpenNI: Could not set color video mode:" << OpenNI::getExtendedError() << std::endl;
        OpenNI::shutdown();
        return 1;
    }

    // Enable registration mode
    status = device.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);

    if (status != STATUS_OK) {
        printf("OpenNI: Could not enable registration mode:\n%s\n", OpenNI::getExtendedError());
        OpenNI::shutdown();
        return 1;
    }

    // Enable color-to-depth synchronization
    status = device.setDepthColorSyncEnabled(true);

    if (status != STATUS_OK) {
        printf("OpenNI: Could not enable color sync:\n%s\n", OpenNI::getExtendedError());
        OpenNI::shutdown();
        return 1;
    }


    std::cout << "Line " << __LINE__ << std::endl;


    // Use allocator to have OpenNI write directly into our color buffer
    status = color_stream.setFrameBuffersAllocator(colorAlloc);

    if (status != STATUS_OK) {
        printf("OpenNI: Could not set color frame buffer allocator\n%s\n", OpenNI::getExtendedError());
        OpenNI::shutdown();
        return 1;
    }

#if 1

    // Disable depth mirroring (we want to see the perspective of the camera)
    status = depth_stream.setMirroringEnabled(false);

    if (status != STATUS_OK) {
        printf("OpenNI: Could enable mirroring on depth stream\n%s\n", OpenNI::getExtendedError());
        OpenNI::shutdown();
        return 1;
    }

    // Disable color mirroring (we want to see the perspective of the camera)
    status = color_stream.setMirroringEnabled(false);

    if (status != STATUS_OK) {
        printf("OpenNI: Could enable mirroring on color stream\n%s\n", OpenNI::getExtendedError());
        OpenNI::shutdown();
        return 1;
    }


    // Use allocator to have OpenNI write directly into our depth buffers
    status = depth_stream.setFrameBuffersAllocator(depthAlloc);

    if (status != STATUS_OK) {
        printf("OpenNI: Could not set depth frame buffer allocator\n%s\n", OpenNI::getExtendedError());
        OpenNI::shutdown();
        return 1;
    }



    std::cout << "Line " << __LINE__ << std::endl;
#endif

    // Start color
    status = color_stream.start();

    if (status != STATUS_OK) {
        std::cerr << "OpenNI: Could not start the color stream" << OpenNI::getExtendedError() << std::endl;
        OpenNI::shutdown();
        return 1;
    }


    std::cout << "Line " << __LINE__ << std::endl;

    // Start depth
    status = depth_stream.start();

    if (status != STATUS_OK) {
        std::cerr << "OpenNI: Could not start the depth stream " << OpenNI::getExtendedError() << std::endl;
        OpenNI::shutdown();
        return 1;
    }

    std::cout << "Line " << __LINE__ << std::endl;

    // Start spawn thread running openni_threadfunc to poll for new frames
    int res = pthread_create(&openni_thread, NULL, openni_threadfunc, this);
    if(res) {

        std::cerr << "error starting OpenNI thread " << res << std::endl;

        OpenNI::shutdown();

        return 1;
    }

    std::cout << "Ready!" << std::endl;

    return 0;
}


int OpenNIDevice::update() {

    openni::Status status = openni::STATUS_OK;

    // Our FrameAllocators make sure the data lands in our buffers;
    // that's why we never have to use the VideoFrameRefs.

    // Next depth frame
    openni::VideoFrameRef depthFrame;
    status = depth_stream.readFrame(&depthFrame);
    if (status != openni::STATUS_OK) {
        std::cerr << "OpenNI: readFrame failed " << openni::OpenNI::getExtendedError() << std::endl;
//        break;
    } else {
        depth_index = (depth_index+1) % 2; // Flip double buffers
        gotDepth = true;
    }

    // Next RGB frame
    openni::VideoFrameRef colorFrame;
    status = color_stream.readFrame(&colorFrame);
    if (status != openni::STATUS_OK) {
        std::cerr << "OpenNI: readFrame failed " << openni::OpenNI::getExtendedError() << std::endl;
//        break;
    }
}

void OpenNIDevice::close() {
    die = true;
    pthread_join(openni_thread, NULL);

    depth_stream.destroy();
    color_stream.destroy();
    device.close();
    openni::OpenNI::shutdown();
}

//bool KinectFrameAvailable() {
//    bool result = gotDepth;
//    gotDepth = false;
//    return result;
//}

//int GetKinectFrame() {
//    return depth_index;
//}
