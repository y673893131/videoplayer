#ifndef QDRAGBORDER_H
#define QDRAGBORDER_H

#include <QWidget>

class QDragBorder : public QWidget
{
    Q_OBJECT
public:
    explicit QDragBorder(QWidget *parent = nullptr);

    void setStartPos(const QPoint&, const QPoint&);
    void setMovePos(const QPoint&);
    void setWidth(int);
signals:

public slots:
private:
    int m_width;
    QWidget* m_parent;
    QPoint m_startPos, m_mouseStart;
    QImage m_bk;
    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event);
};

#endif // QDRAGBORDER_H
