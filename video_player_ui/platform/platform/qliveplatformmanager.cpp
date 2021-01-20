#include "qliveplatformmanager.h"
#include <QRadioButton>

QLivePlatformManager::QLivePlatformManager(QWidget *parent)
    : QObject(parent)
{
    m_platforms.push_back({tr("douyu live"), "rbutton_douyu", Live_DouYu});
    m_platforms.push_back({tr("huya live"), "rbutton_huya", Live_HuYa});
    m_platforms.push_back({tr("egame live"), "rbutton_egame", Live_EGame});

    m_group = new QButtonGroup(parent);
    for(auto it : m_platforms)
    {
        auto btn = new QRadioButton(it.name, parent);
        btn->setObjectName(it.obj);
        m_group->addButton(btn, it.type);
    }
}

QButtonGroup *QLivePlatformManager::group()
{
    return m_group;
}
