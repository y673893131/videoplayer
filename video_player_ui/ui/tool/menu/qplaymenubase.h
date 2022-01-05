#ifndef QPLAYMENUBASE_H
#define QPLAYMENUBASE_H

#include <QMenu>

class QPlayMenuBase : public QMenu
{
    Q_OBJECT
public:
    explicit QPlayMenuBase(QWidget *parent = nullptr);
};

#endif // QPLAYMENUBASE_H
