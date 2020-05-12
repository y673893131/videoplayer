#ifndef _VIDEOFRAME_H
#define _VIDEOFRAME_H
#include <memory>
#include <iostream>
class _video_frame_
{
public:
    _video_frame_(unsigned char* buffer, int w, int h)
        :framebuffer(nullptr)
    {
//        std::cout << "_video_frame_: " << std::endl;
        this->w = w;
        this->h = h;
        framebuffer = new unsigned char[w * h * 3 / 2];
//        std::cout << "_video_frame_: "<< (unsigned int)framebuffer << " " << this << std::endl;
        memcpy(framebuffer, buffer, w * h * 3 / 2);
    }

    ~_video_frame_()
    {
//        std::cout << "~_video_frame_: "<<  (unsigned int)framebuffer << " " <<  this << std::endl;
        if(framebuffer){
            delete[] framebuffer;
            framebuffer = nullptr;
        }
    }

    unsigned char* Y(){
        return framebuffer;
    }

    unsigned char* U(){
        return framebuffer + w * h;
    }

    unsigned char* V(){
        return framebuffer + w * h * 5 / 4;
    }

    int size_y(){
        return w * h;
    }

    int size_u(){
        return w * h / 4;
    }

    int size_v(){
        return w * h / 4;
    }

    unsigned char* data(int index, int& width, int& height){
        unsigned char* data = nullptr;
        switch (index) {
        case 0:
            width = w;
            height = h;
            data = Y();
        break;
        case 1:
            width = w / 2;
            height = h / 2;
            data = U();
        break;
        case 2:
            width = w / 2;
            height = h / 2;
            data = V();
        break;
        }

        return data;
    }
    int w;
    int h;
    unsigned char* framebuffer;
};

typedef std::shared_ptr<_video_frame_> _VideoFramePtr;

#endif // _VIDEOFRAME_H
