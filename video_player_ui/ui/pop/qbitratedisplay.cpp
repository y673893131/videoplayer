#include "qbitratedisplay.h"
#include <QPainter>

class QBitRateDisplayPrivate : public VP_Data<QBitRateDisplay>
{
    VP_DECLARE_PUBLIC(QBitRateDisplay)
    inline QBitRateDisplayPrivate(QBitRateDisplay* parent)
        : VP_Data(parent)
    {
    }

    void paint(QPaintEvent* event);

    QVector<int64_t> m_bitrates;
};

void QBitRateDisplayPrivate::paint(QPaintEvent* /*event*/)
{
    VP_Q(QBitRateDisplay);
    QPainter p(q);
    p.setRenderHint(QPainter::Antialiasing);
    QColor bk = QColor(0x2E2F30);
    p.fillRect(q->rect(), bk);

    auto rc = q->rect();
    QPen pen(QColor(0xFF5C38));
    pen.setWidth(2);
    p.setPen(pen);
    //坐标系

    int padding = 10;
    int step = 10;//1024k
    QVector<QLineF> lines;
    lines.append(QLineF(rc.left() + padding, rc.top(), rc.left()  + padding, rc.bottom() - padding));
    lines.append(QLineF(rc.left()  + padding, rc.bottom() - padding, rc.right(), rc.bottom() - padding));
    p.drawLines(lines);

    //点

    pen.setColor(Qt::red);
    pen.setWidth(2);
    QPoint center(rc.left() + padding, rc.bottom() - padding);
    int x = center.x();

    QPointF last(center);
    QPainterPath path(last);

    for(auto&& it : qAsConst(m_bitrates))
    {
        auto y = it * 8 * 5 / (1024.0 * 1024.0);
        QPointF pt(x, center.y() - y);
        auto end0 = QPointF((pt.x() + last.x()) / 2, last.y());
        auto end1 = QPointF((pt.x() + last.x()) / 2, pt.y());
//        p.drawPoint(pt);
        path.cubicTo(end0, end1, pt);
        x += step;

        p.drawEllipse(last, 2, 2);
        last = pt;
    }

    p.drawPath(path);
}

QBitRateDisplay::QBitRateDisplay(QWidget *parent)
    : QWidget(parent)
    , VP_INIT(new QBitRateDisplayPrivate(this))
{
    setObjectName("bit_rate_display_widget");
}

void QBitRateDisplay::append(int64_t bitrate)
{
    VP_D(QBitRateDisplay);
    auto count = (width() - 10) / 10;
    if(d->m_bitrates.size() > count)
        d->m_bitrates.erase(d->m_bitrates.begin());
    d->m_bitrates.push_back(bitrate);
    update();
}

void QBitRateDisplay::clear()
{
    VP_D(QBitRateDisplay);
    d->m_bitrates.clear();
    update();
}

void QBitRateDisplay::paintEvent(QPaintEvent *event)
{
    VP_D(QBitRateDisplay);
    d->paint(event);
}
