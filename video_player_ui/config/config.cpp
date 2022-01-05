#include "config.h"
#include <QJsonDocument>
#include <QTimer>

Config* Config::instance()
{
    static Config* c = new Config();
    return c;
}

Config::Config()
    :QObject()
{
    m_obj["load_path"] = "";
    m_obj["is_mute"] = false;
    m_obj["volume_num"] = 50;
    m_obj["is_top_window"] = false;
    m_obj["is_adjust_frame"] = true;
    m_obj["render"] = "opengl";
    m_obj["decode"] = 0;
    m_obj["is_loop_play"] = false;
}

void Config::init(const QVariant &va)
{
    auto doc = QJsonDocument::fromJson(va.toByteArray());
    if(!doc.isNull() && !doc.isEmpty() && doc.isObject())
    {
        m_obj = doc.object();
    }

    QTimer::singleShot(0, [this]{ emit loadConfig();});
}

QVariant Config::getData(Config::Data_Type type) const
{
    switch (type)
    {
    case Data_Path:
        return m_obj.value("load_path");
    case Data_Mute:
        return m_obj.value("is_mute");
    case Data_Vol:
        return m_obj.value("volume_num");
    case Data_TopWindow:
        return m_obj.value("is_top_window");
    case Data_Adjust:
        return m_obj.value("is_adjust_frame");
    case Data_Render:
        return m_obj.value("render");
    case Data_Decode:
        return m_obj.value("decode");
    case Data_LoopPlay:
        return m_obj.value("is_loop_play");
    }

    return QVariant();
}

void Config::setData(const QVariant &value, Config::Data_Type type)
{
    switch (type)
    {
    case Data_Path:
        m_obj["load_path"] = value.toString();
        break;
    case Data_Mute:
        m_obj["is_mute"] = value.toBool();
        break;
    case Data_Vol:
        m_obj["volume_num"] = value.toInt();
        break;
    case Data_TopWindow:
        m_obj["is_top_window"] = value.toBool();
        break;
    case Data_Adjust:
        m_obj["is_adjust_frame"] = value.toBool();
        break;
    case Data_Render:
        m_obj["render"] = value.toString();
        break;
    case Data_Decode:
        m_obj["decode"] = value.toInt();
        break;
    case Data_LoopPlay:
        m_obj["is_loop_play"] = value.toBool();
        break;
    }

    emit setConfig(QString::fromUtf8(QJsonDocument(m_obj).toJson().toStdString().c_str()));
}
