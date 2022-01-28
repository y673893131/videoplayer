#ifndef CONFIG_H
#define CONFIG_H
#include <QObject>
#include <QVariant>
#include <QJsonObject>

class Config : public QObject
{
    Q_OBJECT
public:
    enum Data_Type
    {
        Data_Path,
        Data_Mute,
        Data_Vol,
        Data_TopWindow,
        Data_Adjust,
        Data_Render,
        Data_Decode,
        Data_PlayMode
    };

signals:
    void loadConfig();
    void setConfig(const QString&);
public:
    static Config* instance();
    void init(const QVariant&);
    QVariant getData(Data_Type type) const;
    void setData(const QVariant&, Data_Type);
private:
    explicit Config();
    Config(const Config&) = delete;
private:
    QJsonObject m_obj;
};

#define GET_CONFIG_DATA(x) Config::instance()->getData(x)
#define SET_CONFIG_DATA(x,y) Config::instance()->setData(x, y)
#endif // CONFIG_H
