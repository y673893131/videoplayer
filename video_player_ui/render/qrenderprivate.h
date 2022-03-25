#ifndef QRENDERPRIVATE_H
#define QRENDERPRIVATE_H

#include "videoframe.h"
#include <QObject>
#include <QSize>

class QRenderPrivate
{
public:
    QRenderPrivate();

    void setVideoSize(int w, int h);
    void setWindowSize(int w, int h);
    bool setScale(int windowWidth, int windowHeight);
    void setScaleType(int);

    void removeFrame();
private:
    QSize scale(int w, int h);
    void scale(int frameW, int frameH, int w, int h, float &x, float &y);
protected:
    _VideoFramePtr m_frame;
    float* m_freq;
    float* m_freqLast;
    unsigned int m_freqCount;

    QSize m_video;
    QSize m_window;
    float m_fScaleX;
    float m_fScaleY;

    int m_scaleType;
};

#endif // QRENDERPRIVATE_H
