#ifndef QWINTASKBARMENU_H
#define QWINTASKBARMENU_H

#if WIN32
#include <QObject>
#include "video_pimpl.h"

class QWinTaskbarMenuPrivate;
class QWinTaskbarMenu : public QObject
{
    Q_OBJECT
    VP_DECLARE(QWinTaskbarMenu)
    VP_DECLARE_PRIVATE(QWinTaskbarMenu)
public:
    explicit QWinTaskbarMenu(QObject* parent = nullptr);

signals:
    void cmd(int, const QString&);

public slots:
    void onPlay(const QString&);
};

#endif
#endif // QWINTASKBARMENU_H
