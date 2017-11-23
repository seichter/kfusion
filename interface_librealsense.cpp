#include "interface_librealsense.hpp"

#include <iomanip>
#include <cstring>

//void RealSenseDevice_depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp)
//{
//    RealSenseDevice * device = static_cast<RealSenseDevice*>(freenect_get_user(dev));
//    device->setDepthBuffer();
//}


bool align_depth_to_color = false;
bool align_color_to_depth = false;
bool color_rectification_enabled = false;

rs_stream depthStream = RS_STREAM_DEPTH;
rs_stream colorStream = RS_STREAM_COLOR;


void *RealSenseDevice_freenect_threadfunc(void *arg)
{

    RealSenseDevice* dev = static_cast<RealSenseDevice*>(arg);

    while(!dev->stopped()) {
        int res = dev->update();
    }
    return NULL;
}

RealSenseDevice::RealSenseDevice()
    : RGBD()
    , gotDepth(false)
    , die(false)
    , depthScale(1.f)
{
}

int RealSenseDevice::open()
{

    rs_error * e = 0;
    ctx = rs_create_context(RS_API_VERSION, &e);

    int devices = rs_get_device_count(ctx, &e);
    if (devices == 0) {

        std::cerr << "no realsense capture device" << std::endl;
        return 1;
    }

    dev = rs_get_device(ctx, 0, &e);

    std::cout << "Realsense Device: " << rs_get_device_name(dev, &e) << std::endl;
    std::cout << "Realsense Serial: " << rs_get_device_serial(dev, &e) << std::endl;
    std::cout << "Realsense FW ver: " << rs_get_device_firmware_version(dev, &e) << std::endl;

    int framerate = 30;
    rs_enable_stream(dev, depthStream, 0, 0, RS_FORMAT_Z16, framerate, &e);

    int depthWidth = rs_get_stream_width(dev,depthStream,NULL);
    int depthHeight = rs_get_stream_height(dev,depthStream,NULL);

    rs_enable_stream(dev, colorStream, depthWidth, depthHeight, RS_FORMAT_RGB8, framerate, &e);


    rs_get_stream_intrinsics(dev,colorStream,&intrinsics,0);



    std::cout << "width:" << intrinsics.width << " height:" << intrinsics.height
              << " focalx:" << intrinsics.fx << " focaly:" << intrinsics.fy
              << " ppx:" << intrinsics.ppx << " ppy:" << intrinsics.ppy
              << " coeff0:" << intrinsics.coeffs[0]
              << " coeff1:" << intrinsics.coeffs[1]
              << " coeff2:" << intrinsics.coeffs[2]
              << " coeff3:" << intrinsics.coeffs[3]
              << std::endl;



    int colorWidth = rs_get_stream_width(dev,colorStream,NULL);
    int colorHeight = rs_get_stream_height(dev,colorStream,NULL);
    depthScale = rs_get_device_depth_scale(dev,0);

    std::cout << "depth image " << depthWidth << "x" << depthHeight << " scale:" << depthScale << std::endl;
    std::cout << "color image " << colorWidth << "x" << colorHeight << std::endl;



    rs_start_device(dev, &e);

    int res = pthread_create(&_thread, NULL, RealSenseDevice_freenect_threadfunc, this);

    if(res){
        std::cerr << "error starting realsense thread " << res << std::endl;
        return 1;
    }

    depth_index = 0;

    return 0;
}

bool RealSenseDevice::available() const
{
    return gotDepth;
}

int RealSenseDevice::update()
{
    rs_error * e = 0;

    //    gotDepth = false;

    rs_wait_for_frames(dev, &e);

    setDepthBuffer();
}

void RealSenseDevice::setDepthBuffer() {

    rs_error * e = 0;

    int next_buffer = (depth_index + 1) % 2;

    const uint16_t* depthPtr = (const uint16_t*)rs_get_frame_data(dev, depthStream, &e);
    const size_t depthBufferSize = 640*480;
    memcpy(depth_buffer[depth_index],depthPtr,depthBufferSize * sizeof(uint16_t));

    //    uint16_t rescale = 1.f / depthScale;

    for (size_t i = 0; i < depthBufferSize;i++) {
        //        depth_buffer[depth_index][i] = 1 / (depth_buffer[depth_index][i] + 1);
//        depth_buffer[depth_index][i] -= 1;
    }

    const unsigned char* rgbPtr = (const unsigned char*)rs_get_frame_data(dev, colorStream, &e);
    memcpy(rgb_buffer,rgbPtr,640*480*3);

    depth_index = next_buffer;

    gotDepth = true;
}

float RealSenseDevice::focalX() const
{
    return intrinsics.fx;
}

float RealSenseDevice::focalY() const
{
    return intrinsics.fy;
}



void RealSenseDevice::close()
{
    die = true;
    pthread_join(_thread, NULL);


    rs_delete_context(ctx,0);


}
