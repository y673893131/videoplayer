#ifndef QLIVEPLATFORMMANAGER_H
#define QLIVEPLATFORMMANAGER_H

#include <QWidget>
#include <QButtonGroup>
#include <QVector>
#include "datadef.h"


struct _platform_
{
    QString name;
    QString obj;
    Live_Platform type;
};

class QLivePlatformManager : public QObject
{
    Q_OBJECT

public:
    explicit QLivePlatformManager(QWidget *parent = nullptr);

    QButtonGroup* group();
signals:

public slots:
private:
    QButtonGroup* m_group;
    QVector<_platform_> m_platforms;
};

#endif // QLIVEPLATFORMMANAGER_H
