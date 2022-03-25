#ifndef LIBASS_H
#define LIBASS_H

#include <QObject>
#include <QString>
#include "video_pimpl.h"

class LibassPrivate;
class Libass : public QObject
{
    Q_OBJECT
    VP_DECLARE(Libass)
    VP_DECLARE_PRIVATE(Libass)
public:
    explicit Libass(QObject* parent = nullptr);
    ~Libass() override;

    QImage &image();
    bool isFlush();

signals:
    void reportError(const QString&);

public slots:
    void initStreamHeader(const QString&);
    void initAASFile(const QString&);
    void initTrack(const QByteArray&);
    void render(const QString& , int, int);
    void posChange(int);
    void setFrameSize(int, int);
    void flush();
private:
    void initAAS();
};

#endif // LIBASS_H
