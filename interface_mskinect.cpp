#include "interface_mskinect.hpp"

#include <iostream>


struct CaptureThread {

	void operator()() {


	}

};


MSKinectDevice1::MSKinectDevice1()
    : RGBD()
    , gotDepth(false)
    , die(false)
{
}

int MSKinectDevice1::open()
{
	// buffers[0] = depth_buffer[0];
	// buffers[1] = depth_buffer[1];
	// rgb = rgb_buffer;
	// depth_index = 0;
	// gotDepth = false;

	HRESULT hr;

	m_hNextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hNextVideoFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	hr = NuiCreateSensorByIndex(0, &m_pSensor);
	if (FAILED(hr)) {
		std::cerr << "MSKinect SDK: Could not open Device" << std::endl;
		return 1;
	}

	hr = m_pSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH);

	hr = m_pSensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_COLOR,
		NUI_IMAGE_RESOLUTION_640x480,
		0,
		2,
		m_hNextVideoFrameEvent,
		&m_pVideoStreamHandle);

	hr = m_pSensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_DEPTH,
		NUI_IMAGE_RESOLUTION_640x480,
		0,
		2,
		m_hNextDepthFrameEvent,
		&m_pDepthStreamHandle);

	// colorpixels.resize(2 * 640 * 480);

	// Start the Nui processing thread
	m_hEvNuiProcessStop = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	
	
	//m_hThNuiProcess = CreateThread(NULL, 0, run, NULL, 0, NULL);


    return 0;
}

bool MSKinectDevice1::available() const
{
    return gotDepth;
}

int MSKinectDevice1::update()
{
    return 0;
}

void MSKinectDevice1::setDepthBuffer() {

    gotDepth = true;
}

void MSKinectDevice1::close()
{

}
