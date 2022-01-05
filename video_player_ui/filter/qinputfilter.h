#ifndef QINPUTFILTER_H
#define QINPUTFILTER_H

#include <QObject>
#include <QSet>

#define APPEND_EXCEPT_FILTER(x) QInputFilter::instance()->appendExceptObj(x)

class QInputFilter : public QObject
{
    Q_OBJECT
public:
    static QInputFilter *instance(QObject* parent = nullptr);
    void appendExceptObj(QObject*);

protected:
    explicit QInputFilter(QObject* parent = nullptr);

    // QObject interface
signals:
    void volumeJump(bool);
    void progressJump(bool);
    void space();
    void escap();
    void mouseMove();

public:
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    static QInputFilter* s_instance;
    QSet<QObject*> m_exceptObjs;
};

#endif // QINPUTFILTER_H
