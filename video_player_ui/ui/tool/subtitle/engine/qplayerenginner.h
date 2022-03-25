#ifndef QPLAYERENGINNER_H
#define QPLAYERENGINNER_H

#include "video_pimpl.h"
#include "qsubtitleengineer.h"

class QPlayerEnginnerPrivate;
class QPlayerEnginner : public QSubtitleEngineer
{
    Q_OBJECT
    VP_DECLARE(QPlayerEnginner)
    VP_DECLARE_PRIVATE(QPlayerEnginner)
public:
    explicit QPlayerEnginner(QObject* parent = nullptr);
    ~QPlayerEnginner() override;

public slots:
    void onHeader(const subtitle_header &infos) override;
    void onRender(const QString &, unsigned int, int type, int64_t start, int64_t end) override;
    void flush() override;
    void clean() override;
    void update() override;
};

#endif // QPLAYERENGINNER_H
