#include "qwinthumbnail.h"

#if WIN32
#include <QPixmap>
#include <QBitmap>
#include "Log/Log.h"
#include "qwinthumbnail_p.h"

QWinThumbnail::QWinThumbnail(QWinThumbnailPrivate *pri, QWidget *parent)
    : QFrameLessWidget(pri, parent)
{
    VP_D(QWinThumbnail);
    d->init();
}

QWinThumbnail::~QWinThumbnail()
{
}

void QWinThumbnail::onStart()
{
    VP_D(QWinThumbnail);
    d->modifyBtn(true);
}

void QWinThumbnail::onPause()
{
    VP_D(QWinThumbnail);
    d->modifyBtn(false);
}

void QWinThumbnail::onEnd()
{
    VP_D(QWinThumbnail);
    d->modifyBtn(false);
}

#endif
