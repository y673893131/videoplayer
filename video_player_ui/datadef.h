#ifndef DATADEF_H
#define DATADEF_H
#include <QMap>

struct _DyClass_
{
    QStringList names;
    QStringList urls;
};

struct _Dy_
{
    QMap<QString, _DyClass_> nameToClass;
    QStringList names;
    QStringList urls;
};

struct _Dy_Sign_Arg_
{
    _Dy_Sign_Arg_()
    {
        rate = "-1";
        did = "10000000000000000000000000001501";
    }

    QString key()
    {
        return rid + did + tt + v;
    }

    QString datas() const
    {
        return QString("ver=%1&tt=%2&rid=%3&v=%4&sign=%5&rate=%6&did=%7").arg(ver).arg(tt).arg(rid).arg(v).arg(sign).arg(rate).arg(did);
    }
    QString v;
    QString rid;
    QString did;
    QString tt;
    QString sign;
    QString ver;
    QString rate;
};

enum Live_Platform
{
    Live_DouYu = 0,
    Live_HuYa,
    Live_EGame,

    Live_Max
};

#endif // DATADEF_H
