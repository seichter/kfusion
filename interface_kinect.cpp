#include "interface_kinect.hpp"

#include <iostream>

void kinectdevice_depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp)
{
    KinectDevice * device = static_cast<KinectDevice*>(freenect_get_user(dev));
    device->setDepthBuffer();
}

void *kinectdevice_freenect_threadfunc(void *arg)
{

    KinectDevice* dev = static_cast<KinectDevice*>(arg);

    while(!dev->stopped()){
        int res = dev->update();
        if (res < 0 && res != -10) {
            std::cerr << "\nError "<< res << " received from libusb - aborting." << std::endl;
            break;
        }
    }
    return NULL;
}

KinectDevice::KinectDevice()
    : RGBD()
    , f_ctx(0L)
    , f_dev(0L)
    , gotDepth(false)
    , die(false)
{
}

int KinectDevice::open()
{
    if (freenect_init(&f_ctx, NULL) < 0) {
        std::cerr << "freenect_init() failed" << std::endl;
        return 1;
    }


    freenect_set_log_level(f_ctx, FREENECT_LOG_WARNING);
    freenect_select_subdevices(f_ctx, (freenect_device_flags)(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));

    int nr_devices = freenect_num_devices (f_ctx);
    if (nr_devices < 1){
        std::cerr << "libfreenect: No devices found" << std::endl;
        return 1;
    }

    if (freenect_open_device(f_ctx, &f_dev, 0) < 0) {
        std::cerr << "libfreenect: Could not open device" << std::endl;
        return 1;
    }

    // set callback user
    freenect_set_user(f_dev,this);

    gotDepth = false;
    depth_index = 0;

    // setup callbacks
    freenect_set_depth_callback(f_dev, kinectdevice_depth_cb);
    freenect_set_depth_mode(f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED));
    freenect_set_depth_buffer(f_dev, depth_buffer[depth_index]);

    freenect_set_video_mode(f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB));
    freenect_set_video_buffer(f_dev, rgb_buffer);

    freenect_start_depth(f_dev);
    freenect_start_video(f_dev);

    int res = pthread_create(&freenect_thread, NULL, kinectdevice_freenect_threadfunc, this);

    if(res){
        std::cerr << "error starting kinect thread " << res << std::endl;
        return 1;
    }

    return 0;
}

bool KinectDevice::available() const
{
    return gotDepth;
}

int KinectDevice::update()
{
    return freenect_process_events(f_ctx);
}

void KinectDevice::setDepthBuffer() {

    int next_buffer = (depth_index + 1) % 2;
    freenect_set_depth_buffer(f_dev, depth_buffer[depth_index]);
    depth_index = next_buffer;
    gotDepth = true;
}

void KinectDevice::close()
{
    die = true;
    pthread_join(freenect_thread, NULL);

    freenect_stop_depth(f_dev);
    freenect_stop_video(f_dev);
    freenect_close_device(f_dev);
    freenect_shutdown(f_ctx);
}
