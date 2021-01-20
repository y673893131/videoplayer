#ifndef QHUYAMODEL_H
#define QHUYAMODEL_H

#include "qlivedatamodel.h"

class QHuYaModel : public QLiveDataModel
{
    Q_OBJECT
public:
    explicit QHuYaModel(QObject *parent = nullptr);

    void loadGameRooms(const QString&);
    void loadGameRoomPage(const QString&, int = 1);
private:
    void initClass();
public slots:
    virtual void getPreviewUrl(const QString&);
};

#endif // QHUYAMODEL_H
