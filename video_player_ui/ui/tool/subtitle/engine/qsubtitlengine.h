#ifndef QSUBTITLENGINE_H
#define QSUBTITLENGINE_H

#include <QWidget>
#include "video_pimpl.h"

class QSubtitlEnginePrivate;
class QSubtitlEngine : public QObject
{
    Q_OBJECT
    VP_DECLARE(QSubtitlEngine)
    VP_DECLARE_PRIVATE(QSubtitlEngine)
public:
    explicit QSubtitlEngine(QWidget* parent = nullptr);
    ~QSubtitlEngine() override;

    void clean();
    void update();
    void flush();
    void setPos(int);
private:
    void setTarget(QWidget*);

signals:
    void reportError(const QString&);
public slots:
    void setEngine(int);
};

#endif // QSUBTITLENGINE_H
