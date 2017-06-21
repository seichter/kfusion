/*
Copyright (c) 2011-2013 Gerhard Reitmayr, TU Graz

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef INTERFACE_H
#define INTERFACE_H



typedef unsigned short uint16_t;

class RGBD {
protected:

    uint16_t *depth_buffer[2];
    unsigned char *rgb_buffer;


    int depth_index;

public:
    enum RGBDDevice {
        kRGBDDeviceKinect
    };

    RGBD() {}
    virtual ~RGBD() {}

    virtual int open() = 0;
    virtual bool available() const { return false; }
    virtual int update() = 0;
    virtual void close() = 0;

    static RGBD* create(RGBDDevice device,const char* flags = "");

    void setBuffers(uint16_t * dBuffer[2], unsigned char * rgbBuffer) {
        depth_buffer[0] = dBuffer[0];
        depth_buffer[1] = dBuffer[1];
        rgb_buffer = rgbBuffer;
    }


    uint16_t* currentDepthBuffer() const {
        return depth_buffer[depth_index];
    }

    int currentDepthBufferIndex() const {
        return depth_index;
    }

};



//int InitKinect( uint16_t * depth_buffer[2], unsigned char * rgb_buffer );
//bool KinectFrameAvailable();
//int GetKinectFrame();
//void CloseKinect();

#endif
