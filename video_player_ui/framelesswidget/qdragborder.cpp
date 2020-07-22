#include "qdragborder.h"
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QDesktopWidget>
QDragBorder::QDragBorder(QWidget *parent)
    : QWidget(QApplication::desktop()),m_width(10),m_parent(parent)
{
    setWindowFlags(Qt::FramelessWindowHint|Qt::Tool|Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void QDragBorder::setStartPos(const QPoint &startPos,const QPoint &mouse)
{
    m_bk = QGuiApplication::primaryScreen()->grabWindow(0).toImage();
    m_startPos = startPos;
    m_mouseStart = mouse;
    move(m_startPos);
}

void QDragBorder::setMovePos(const QPoint &movePos)
{
    move(m_startPos + movePos - m_mouseStart);
//    update();
}

void QDragBorder::setWidth(int width)
{
    m_width = width;
}
#include <QPainterPath>
void QDragBorder::paintEvent(QPaintEvent */*event*/)
{
//    qDebug() << "paint";
    QPainter painter(this);
    painter.setBrush(Qt::NoBrush);
    painter.setCompositionMode(QPainter::CompositionMode_Difference);

    QPen pen(Qt::SolidLine);

    pen.setColor(QColor(128,128,128,128));
    pen.setWidthF(m_width);
    painter.setPen(pen);

//    QPolygon points((width() + height()) * 2 * m_width);
//    int index = 0;
//    auto pos = this->pos();
//    auto size = this->size();
//    for(auto w = 0; w < m_width; ++w)
//    {
//        for(int n = 0; n < size.width(); ++n)
//        {
//            points.setPoint(index++, pos.x() + w + n, pos.y() + w);
//            points.setPoint(index++, pos.x() + w + n, pos.y() + size.height() - w);
//        }

//        for (int n = 0; n < size.height(); ++n) {
//            points.setPoint(index++, pos.x() + w, pos.y() + w + n);
//            points.setPoint(index++, pos.x() + size.width() - w, pos.y() + w + n);
//        }
//    }
//    painter.drawPoints(points);
    painter.drawRect(rect());
}
