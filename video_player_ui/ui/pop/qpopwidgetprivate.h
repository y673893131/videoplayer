#ifndef QPOPWIDGETPRIVATE_H
#define QPOPWIDGETPRIVATE_H


#include "framelesswidget/nativeevent_p.h"
#include "qpopwidget.h"
#include "qcreateitem.h"

class QPopWidgetPrivate : public CNativeEvent_p, public QCreateItem
{
    VP_DECLARE_PUBLIC(QPopWidget)
public:
    QPopWidgetPrivate(QPopWidget* parent);

    void setKey(const QString&);
    QString key();

private:
    void init(QWidget* parent);
    bool isCapture() override;
protected:
    QString m_sKey;
};

#endif // QPOPWIDGETPRIVATE_H
