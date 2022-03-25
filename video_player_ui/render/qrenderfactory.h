#ifndef QRENDERFACTORY_H
#define QRENDERFACTORY_H

#include <QWidget>

class QRenderFactory : public QObject
{
    Q_OBJECT
public:
    explicit QRenderFactory(QWidget *parent = nullptr);

    bool isUpdate();
    QWidget *renderWidget();
public slots:
    void onCap();
private:
    QWidget* m_renderWidget;
    bool m_bUpdate;
};

#endif // QRENDERFACTORY_H
