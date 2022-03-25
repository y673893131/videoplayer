#ifndef QASSENGINEER_H
#define QASSENGINEER_H

#include "video_pimpl.h"
#include "qsubtitleengineer.h"

class QASSEngineerPrivate;
class QASSEngineer : public QSubtitleEngineer
{
    Q_OBJECT
    VP_DECLARE(QASSEngineer)
    VP_DECLARE_PRIVATE(QASSEngineer)
public:
    explicit QASSEngineer(QObject* parent = nullptr);
    ~QASSEngineer() override;

public slots:
    void setFrameSize(QSize size) override;
    void onHeader(const subtitle_header &infos) override;
    void onRender(const QString &, unsigned int, int type, int64_t start, int64_t end) override;
    void flush() override;
    void clean() override;
    void update() override;
};

#endif // QASSENGINEER_H
