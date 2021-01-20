#ifndef QROOMLISTVIEW_H
#define QROOMLISTVIEW_H

#include <QListView>

class QRoomListView : public QListView
{
    Q_OBJECT
public:
    explicit QRoomListView(QWidget *parent = nullptr);

signals:

public slots:
    void go(const QString&);
};

#endif // QROOMLISTVIEW_H
