#ifndef LIBASS_H
#define LIBASS_H

#include <QObject>
#include <QString>

class LibassPrivate;
class Libass : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Libass)
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
    void setFrameSize(int, int);
    void flush();
private:
    static void callback(int level, const char *fmt, va_list va, void *data);
private:
    void initAAS();
    void uninit();
private:
    LibassPrivate* d_ptr;
};

#endif // LIBASS_H
