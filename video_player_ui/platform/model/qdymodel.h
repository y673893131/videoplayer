#ifndef QDYMODEL_H
#define QDYMODEL_H

#include "qlivedatamodel.h"



class QDyModel : public QLiveDataModel
{
    Q_OBJECT
public:
    explicit QDyModel(QObject *parent = nullptr);

    void loadGameRooms(const QString&);
    void loadGameRoomPage(const QString&, int = 1);
private:
    void initClass();
    void getLiveArg(const QString& rid);
    void getLiveUrl(const _Dy_Sign_Arg_& arg);
signals:
    void loadVered();
public slots:
    void getVerArg();
    void getPreviewUrl(const QString&);
};

#endif // QDYMODEL_H
