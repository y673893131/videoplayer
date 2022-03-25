#include "config.h"
#include <QJsonDocument>
#include <QTimer>
#include <QVector>

struct type_struct
{
    QPair<QString, QJsonValue> type;
};

class ConfigPrivate : public VP_Data<Config>
{
    VP_DECLARE_PUBLIC(Config)
    inline ConfigPrivate(Config* parent)
        : VP_Data(parent)
    {
        prepareDefaultData();
    }

    void prepareDefaultData();
    QVariant getData(Config::Data_Type type) const;
    bool setData(const QVariant &value, Config::Data_Type type);
private:
    QVector<QPair<QString, QJsonValue>> m_types;
    QJsonObject m_obj;
};

void ConfigPrivate::prepareDefaultData()
{
#define TO_PAIR(x, y) qMakePair(QString(x), QJsonValue(y))
    m_types << TO_PAIR("load_path", "");
    m_types << TO_PAIR("is_mute", false);
    m_types << TO_PAIR("volume_num", 50);
    m_types << TO_PAIR("is_top_window", false);
    m_types << TO_PAIR("render", "opengl");
    m_types << TO_PAIR("decode", 0);
    m_types << TO_PAIR("play_mode", 0);
    m_types << TO_PAIR("play_size", 0);
    m_types << TO_PAIR("recent", true);
    m_types << TO_PAIR("zero_copy", false);

    for(auto&& it : qAsConst(m_types))
        m_obj[it.first] = it.second;
}

QVariant ConfigPrivate::getData(Config::Data_Type type) const
{
    if(type < m_types.size())
    {
        auto jType = m_types[type];
        return m_obj[jType.first];
    }

    return QVariant();
}

bool ConfigPrivate::setData(const QVariant &value, Config::Data_Type type)
{
    bool bModify = false;
    if(type < m_types.size())
    {
        auto lastValue = getData(type);
        if(lastValue != value)
        {
            bModify = true;
            auto jType = m_types[type];
            m_obj[jType.first] = QJsonValue::fromVariant(value);
        }
    }

    return bModify;
}

Config* Config::instance()
{
    static Config* c = new Config();
    return c;
}

Config::Config()
    :QObject()
    ,VP_INIT(new ConfigPrivate(this))
{
}

Config::~Config()
{
}

void Config::init(const QVariant &va)
{
    VP_D(Config);
    auto doc = QJsonDocument::fromJson(va.toByteArray());
    if(!doc.isNull() && !doc.isEmpty() && doc.isObject())
    {
        d->m_obj = doc.object();
    }

    QTimer::singleShot(0, [this]{ emit loadConfig();});
}

QVariant Config::getData(Config::Data_Type type) const
{
    VP_D(const Config);
    return d->getData(type);
}

void Config::setData(const QVariant &value, Config::Data_Type type)
{
    VP_D(Config);
    if(d->setData(value, type))
        emit setConfig(QString::fromUtf8(QJsonDocument(d->m_obj).toJson().toStdString().c_str()));
}
