#ifndef QPLAYMENUBASE_H
#define QPLAYMENUBASE_H

#include <QMenu>
#include <map>
#include <QXmlStreamReader>
#include <QtXml/QDomDocument>

enum menu_attr_type
{
    attr_checkable      = 0x1,
    attr_group          = 0x10,
    attr_separator      = 0x100,
    attr_menu           = 0x1000
};

struct menu_attr_t
{
    menu_attr_t()
        : type(0)
    {
    }

    ~menu_attr_t()
    {
        for(auto it : sub)
        {
            delete it;
        }

        sub.clear();
    }
    int type;
    QString name;
    QString des;
    QString data;
    std::vector<menu_attr_t*> sub;
};

class QPlayMenuBase : public QMenu
{
    Q_OBJECT
public:
    explicit QPlayMenuBase(QWidget *parent = nullptr);
    QPlayMenuBase(const QString& path, QWidget *parent = nullptr);
    ~QPlayMenuBase();

protected:
    QActionGroup* group(const QString&);
    QMenu* subMenu(const QString&);
private:
    void initMenuFile(const QString& path);
    void analyzeMenu(const QDomElement&, menu_attr_t*);
    menu_attr_t* analyzeAction(const QDomElement&, menu_attr_t*);
    void appendMenu(menu_attr_t* attr, QMenu*);
    bool testType(menu_attr_type type, int flags);
private:
    menu_attr_t* m_datas;
    std::map<QString, QActionGroup*> m_groups;
    std::map<QString, QMenu*> m_menus;
};

#endif // QPLAYMENUBASE_H
