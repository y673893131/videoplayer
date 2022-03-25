#include "qsubtitlelabel.h"
#include <QtMath>
#include <QPainter>
#include <QPainterPath>
QSubTitleLabel::QSubTitleLabel(QWidget* parent)
    : QLabel(parent)
{
    init();
}

QSubTitleLabel::QSubTitleLabel(const QString & text, QWidget *parent)
    : QLabel(text, parent)
{
    init();
}

void QSubTitleLabel::setMode(bool bMode)
{
    m_mode = bMode;
}

void QSubTitleLabel::setOutlineThickness(double scale)
{
    m_scale = scale;
}

double QSubTitleLabel::outlineThickness() const
{
    if(m_mode)
        return m_scale * font().pointSize();
    else
        return m_scale;
}

void QSubTitleLabel::setBrush(QColor color)
{
    m_brush = color;
}

void QSubTitleLabel::setPen(QColor color)
{
    m_pen = color;
}

QSize QSubTitleLabel::sizeHint() const
{
    auto w = qCeil(outlineThickness() * 2);
    return QLabel::sizeHint() + QSize(w, w);
}

QSize QSubTitleLabel::minimumSizeHint() const
{
    auto w = qCeil(outlineThickness() * 2);
    return QLabel::minimumSizeHint() + QSize(w, w);
}

void QSubTitleLabel::paintEvent(QPaintEvent *)
{
    auto w = outlineThickness();
    auto rect = this->rect();
    auto metrics = QFontMetrics(font());
    auto tr = metrics.boundingRect(text()).adjusted(0, 0, w, w);
    int indent = 0;
    if (this->indent() == -1)
    {
        if (frameWidth())
            indent = (metrics.boundingRect('x').width() + w * 2) / 2;
        else
            indent = w;
    }
    else
        indent = this->indent();

    int x = 0;
    auto align = this->alignment();
    if (align & Qt::AlignLeft)
        x = rect.left() + indent - qMin(metrics.leftBearing(text()[0]), 0);
    else if(align & Qt::AlignRight)
        x = rect.x() + rect.width() - indent - tr.width();
    else
        x = (rect.width() - tr.width()) / 2;

    int y = 0;
    if (align & Qt::AlignTop)
        y = rect.top() + indent + metrics.ascent();
    else if(align & Qt::AlignBottom)
        y = rect.y() + rect.height() - indent - metrics.descent();
    else
        y = (rect.height() + metrics.ascent() - metrics.descent()) / 2;

    auto path = QPainterPath();
    path.addText(x, y, font(), text());
    QPainter qp(this);
    qp.setRenderHint(QPainter::Antialiasing);

    m_pen.setWidthF(w * 2);
//    qp.strokePath(path, m_pen);
//    if (1 < m_brush.style() && m_brush.style() < 15)
//        qp.fillPath(path, palette().window());
//    qp.fillPath(path, m_brush);

    if(true) {
        qp.setBrush(Qt::NoBrush);
        qp.setPen(QPen(m_pen.color(), w * 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        for(const auto &polygon : path.toSubpathPolygons())
            qp.drawPolygon(polygon, Qt::OddEvenFill);
    }

    qp.setBrush(m_brush);
    qp.setPen(Qt::NoPen);
    for(const auto &polygon : path.toFillPolygons())
        qp.drawPolygon(polygon, Qt::OddEvenFill);
}

void QSubTitleLabel::init()
{
    m_scale = 1/ 25.0;
    m_mode = true;
    m_brush = QColor(Qt::white);
    m_pen = QColor(Qt::black);
}
