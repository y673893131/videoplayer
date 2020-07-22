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
    const QString& configPath();
    int configVol();
signals:
    void loadsuccessed(const QVector<QStringList>&);

public slots:
    void onAddUrl(const QString& file);
private:
    void initConfig();
    void initTable(const QString& sql);
    void loadConfig();
    void reportError(const QSqlQuery & query);
private:
    QSqlDatabase m_db;
    QString m_sPath;
    int m_nVol;
};

#endif // QDATAMODEL_H
