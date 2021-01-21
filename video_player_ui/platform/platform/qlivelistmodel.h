#ifndef QLIVELISTMODEL_H
#define QLIVELISTMODEL_H

#include <QAbstractItemModel>
#include "platform/model/qlivedatamodel.h"
#include "platform/model/datadef.h"

class QLiveListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum user_role
    {
        Game_url_role = Qt::UserRole + 1,
    };
public:
    explicit QLiveListModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int columnCount(const QModelIndex &parent) const;

    CRoomManager *getRoomManager();
    QLiveDataModel* curData() const;
private:
    void initTypeData(int, QLiveDataModel*);

signals:
    void loadRoomsInfo(int);
    void play(const QString&);
    void resetPage(int);
public slots:
    void setCurrentType(int);
    void selectGame(const QModelIndex&);
    void selectRoom(const QModelIndex&);
    void onPageChanged(int);

private:
    QLiveDataModel* m_data[Live_Max];
    int m_currentType;
    int m_nPageCount;
    QString m_sPagePath;
};

#endif // QLIVELISTMODEL_H
