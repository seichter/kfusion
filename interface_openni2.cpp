#include "interface_openni2.hpp"

#include <iostream>
#include <stdexcept>

//
// allocators
// 
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
        return depth_buffers_[0];
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

struct CaptureThread {

	OpenNIDevice& _device;

	CaptureThread(OpenNIDevice& dev) : _device(dev) {

	}
	
	void operator()() {

		while (!_device.stopped()) {
			_device.update();
		}

		// close stuff
		// _device->close();
	}

};

OpenNIDevice::OpenNIDevice()
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

        std::cerr << "OpenNI: Initialize failed: '" << OpenNI::getExtendedError() << "'" << std::endl;

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

    std::cout << "Device " << device.getDeviceInfo().getName() << std::endl;

    // Create depth stream
    if (device.getSensorInfo(SENSOR_DEPTH) != NULL) {
        status = depth_stream.create(device, SENSOR_DEPTH);
        if (status != STATUS_OK) {

            std::cerr << "OpenNI: Could not create depth stream " <<  OpenNI::getExtendedError() << std::endl;

            OpenNI::shutdown();

            return 1;
        }
    }

    // Create color stream
    if (device.getSensorInfo(SENSOR_COLOR) != NULL) {
        status = color_stream.create(device, SENSOR_COLOR);
        if (status != STATUS_OK) {
            std::cerr << "OpenNI: Could not create color stream" << OpenNI::getExtendedError() << std::endl;
            OpenNI::shutdown();
            return 1;
        }
    }

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

    // Use allocator to have OpenNI write directly into our color buffer
    status = color_stream.setFrameBuffersAllocator(colorAlloc);

    if (status != STATUS_OK) {
        printf("OpenNI: Could not set color frame buffer allocator\n%s\n", OpenNI::getExtendedError());
        OpenNI::shutdown();
        return 1;
    }

    // Start depth
    status = depth_stream.start();

    if (status != STATUS_OK) {
        printf("OpenNI: Could not start the depth stream\n%s\n", OpenNI::getExtendedError());
        OpenNI::shutdown();
        return 1;
    }

    // Start color
    status = color_stream.start();

    if (status != STATUS_OK) {
        printf("OpenNI: Could not start the color stream\n%s\n", OpenNI::getExtendedError());
        OpenNI::shutdown();
        return 1;
    }

	// create capture thread
	capture_thread = std::thread(CaptureThread(*this));
	
    return 0;
}


int OpenNIDevice::update() {

    openni::Status status = openni::STATUS_OK;

    // Our FrameAllocators make sure the data lands in our buffers;
    // that's why we never have to use the VideoFrameRefs.

#if 0
    // Next depth frame
    openni::VideoFrameRef depthFrame;
    status = depth_stream.readFrame(&depthFrame);
    if (status != openni::STATUS_OK) {
        std::cerr << "OpenNI: readFrame failed " << openni::OpenNI::getExtendedError() << std::endl;
    } else {
        depth_index = (depth_index + 1) % 2; // Flip double buffers
        gotDepth = true;
    }
#endif

    // Next RGB frame
    openni::VideoFrameRef colorFrame;
    status = color_stream.readFrame(&colorFrame);
    if (status != openni::STATUS_OK) {
        std::cerr << "OpenNI: readFrame failed " << openni::OpenNI::getExtendedError() << std::endl;
    }

	return 0;
}

void OpenNIDevice::close() {
    
	die = true;
	
	capture_thread.join();

    depth_stream.destroy();
    color_stream.destroy();
    device.close();
    openni::OpenNI::shutdown();
}
