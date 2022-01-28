#ifndef QDATAMODEL_H
#define QDATAMODEL_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>

struct file_info_t
{
    QString name;
    QString url;
    int times;
};

class QDataModel : public QObject
{
    Q_OBJECT
public:
    static QDataModel *instance();

private:
    explicit QDataModel(QObject *parent = nullptr);

    void init();
signals:
    void loadsuccessed(const QVector<file_info_t>&);
    void addUrlSuccess(const QString&);
public slots:
    void onAddUrl(const QStringList& list);
    void removeUrl(const QString&);
    void onExecSql(const QString&);
    void onUpdateTimees(const QString&, int);
    void onClean();
private:
    void initConfig();
    void initTable(const QString& sql);
    void loadConfig();
    void reportError(const QSqlQuery & query);
private:
    QSqlDatabase m_db;
    static QDataModel* m_instance;
};

#define DATA() QDataModel::instance()
#endif // QDATAMODEL_H
