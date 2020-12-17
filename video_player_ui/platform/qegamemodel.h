#ifndef QEGAMEMODEL_H
#define QEGAMEMODEL_H

#include "qlivedatamodel.h"

class QEGameModel : public QLiveDataModel
{
    Q_OBJECT
public:
    explicit QEGameModel(QObject* parent = nullptr);

    void loadGameRooms(const QString&);
    void loadGameRoomPage(const QString&, int = 1);
private:
    void initClass();
public slots:
    void getPreviewUrl(const QString&);
};

#endif // QEGAMEMODEL_H
