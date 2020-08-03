#ifndef QDATAMODEL_H
#define QDATAMODEL_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>


class QDataModel : public QObject
{
    Q_OBJECT
public:
    explicit QDataModel(QObject *parent = nullptr);

    void init();
signals:
    void loadsuccessed(const QVector<QStringList>&);
    void addUrlSuccess(const QString&);
public slots:
    void onAddUrl(const QString& file);
    void removeUrl(const QString&);
    void onExecSql(const QString&);
private:
    void initConfig();
    void initTable(const QString& sql);
    void loadConfig();
    void reportError(const QSqlQuery & query);
private:
    QSqlDatabase m_db;
};

#endif // QDATAMODEL_H
