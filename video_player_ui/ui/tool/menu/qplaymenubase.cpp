#include "qplaymenubase.h"
#include <QFileInfo>
#include <QFile>
#include <QTimer>
#include "framelesswidget/util.h"
#include <QDebug>
QPlayMenuBase::QPlayMenuBase(QWidget *parent)
    :QMenu(parent)
    ,m_datas(nullptr)
{
    setObjectName("menu1");
//    CALC_WIDGET_WIDTH(this, 80);
}

QPlayMenuBase::QPlayMenuBase(const QString &path, QWidget *parent)
    :QMenu(parent)
{
    setObjectName("menu1");
    initMenuFile(path);
}

QPlayMenuBase::~QPlayMenuBase()
{
    if(m_datas)
    {
        delete m_datas;
        m_datas = nullptr;
    }
}

QActionGroup *QPlayMenuBase::group(const QString &name)
{
    auto it = m_groups.find(name);
    if(it != m_groups.end())
        return it->second;
    return nullptr;
}

QMenu *QPlayMenuBase::subMenu(const QString &name)
{
    auto it = m_menus.find(name);
    if(it != m_menus.end())
        return it->second;
    return nullptr;
}

QAction *QPlayMenuBase::action(const QString &menu, const QString &name)
{
    auto it = m_menus.find(menu);
    if(it != m_menus.end())
    {
        for(auto&& action : it->second->actions())
        {
            if(action->property("name") == name)
                return action;
        }
    }

    return nullptr;
}

void QPlayMenuBase::initMenuFile(const QString &path)
{
    m_datas = new menu_attr_t();

    if (!QFileInfo::exists(path))
        return;

    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return;

    QDomDocument  dom;
    dom.setContent(&file);

    file.close();

    analyzeMenu(dom.firstChildElement(), m_datas);
    appendMenu(m_datas->sub[0], this);
}

void QPlayMenuBase::analyzeMenu(const QDomElement &element, menu_attr_t* attr)
{
    auto menu = analyzeAction(element, attr);
    auto subNode = element.firstChild();
    while (!subNode.isNull())
    {
        auto subNodeElement = subNode.toElement();
        if (subNodeElement.nodeName() == "menu")
        {
            analyzeMenu(subNodeElement, menu);
        }
        else if(subNodeElement.nodeName() == "action")
        {
            analyzeAction(subNodeElement, menu);
        }

        subNode = subNode.nextSibling();
    }
}

menu_attr_t* QPlayMenuBase::analyzeAction(const QDomElement &element, menu_attr_t* attr)
{
    auto action = new menu_attr_t();
    action->name = element.attribute("name");
    action->des = element.attribute("description");
    action->data = element.attribute("data");
    bool ok = false;
    action->type = element.attribute("type").toInt(&ok, 16);
    attr->sub.push_back(action);

    return action;
}

void QPlayMenuBase::appendMenu(menu_attr_t* attr, QMenu* menu)
{
    auto actions = attr->sub;
    QActionGroup* group = nullptr;
    for(auto it : actions)
    {
        if(testType(attr_menu, it->type))
        {
            group = nullptr;
            auto subMenu = menu->addMenu(it->des);
            subMenu->setObjectName("menu1");
            if(!it->data.isEmpty()) subMenu->setProperty("data", it->data);
            appendMenu(it, subMenu);
            if(testType(attr_group, it->type))
            {
                group = new QActionGroup(this);
                m_groups.insert(std::make_pair(it->name, group));
            }

            m_menus.insert(std::make_pair(it->name, subMenu));
        }
        else if(testType(attr_separator, it->type))
        {
            group = nullptr;
            menu->addSeparator();
        }
        else
        {
            auto action = menu->addAction(it->des);
            if(!it->data.isEmpty()) action->setData(it->data);
            action->setProperty("name", it->name);
            if(testType(attr_checkable, it->type))
            {
                action->setCheckable(true);
            }
            if(testType(attr_group, it->type))
            {
                if(!group)
                {
                    group = new QActionGroup(this);
                    m_groups.insert(std::make_pair(attr->name, group));
                }

                group->addAction(action);
            }
        }
    }
}

bool QPlayMenuBase::testType(menu_attr_type type, int flags)
{
    return (flags & type) != 0;
}
