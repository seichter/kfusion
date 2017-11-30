#ifndef INTERFACE_MSKINECT_HPP
#define INTERFACE_MSKINECT_HPP


#include "interface.h"


#include <Windows.h>
#include <NuiApi.h>

#include <thread>


class MSKinectDevice1 : public RGBD {


	HANDLE        m_hNextDepthFrameEvent;
	HANDLE        m_hNextVideoFrameEvent;
	HANDLE        m_pDepthStreamHandle;
	HANDLE        m_pVideoStreamHandle;

	INuiSensor * m_pSensor;

	// thread handling
	HANDLE        m_hThNuiProcess;
	HANDLE        m_hEvNuiProcessStop;



    bool gotDepth;

    bool die;

    // RGBD interface
public:

    MSKinectDevice1();

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

