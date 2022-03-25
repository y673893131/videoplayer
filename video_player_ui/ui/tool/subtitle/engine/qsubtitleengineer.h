#ifndef QSUBTITLEENGINEER_H
#define QSUBTITLEENGINEER_H

#include <QObject>
#include <QWidget>
#include "video_player_core.h"
#include "video_pimpl.h"

class QSubtitleEngineerPrivate;
class QSubtitleEngineer : public QObject
{
    Q_OBJECT
    VP_DECLARE(QSubtitleEngineer)
    VP_DECLARE_PRIVATE(QSubtitleEngineer)
public:
    explicit QSubtitleEngineer(QObject *parent = nullptr);
    ~QSubtitleEngineer() override;

    QWidget *target();
    void setTarget(QWidget*);

    void setPos(int);

    subtitle_header header();
    void setHeader(const subtitle_header&);
signals:

public slots:
    virtual void setFrameSize(QSize size);
    virtual void onHeader(const subtitle_header &infos) = 0;
    virtual void onRender(const QString&, unsigned int, int type, int64_t start, int64_t end) = 0;
    virtual void flush() = 0;
    virtual void clean() = 0;
    virtual void update() = 0;

protected:
    bool isFlush(int pos);
    bool updateDelay(int64_t start, int64_t end);
};

#endif // QSUBTITLEENGINEER_H
