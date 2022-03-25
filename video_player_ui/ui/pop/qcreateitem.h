#ifndef QCREATEITEM_H
#define QCREATEITEM_H

#include <QWidget>

class QCreateItem
{
public:
    QCreateItem();

    QWidget *createTitle(const QString&, QWidget* parent);
    bool isTitleCapture();
private:
    QWidget* m_title;
};

#endif // QCREATEITEM_H
