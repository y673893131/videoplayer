#ifndef QSUBTITLELABEL_H
#define QSUBTITLELABEL_H

#include <QLabel>
#include <QPen>

class QSubTitleLabel : public QLabel
{
    Q_OBJECT
public:
    explicit QSubTitleLabel(QWidget* parent = nullptr);
    QSubTitleLabel(const QString&, QWidget* parent = nullptr);

public:
    void setMode(bool);
    void setOutlineThickness(double);
    double outlineThickness() const;
    void setBrush(QColor);
    void setPen(QColor);

protected:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void paintEvent(QPaintEvent *) override;

private:
    void init();
private:
    double m_scale;
    bool m_mode;
    QBrush m_brush;
    QPen m_pen;
};

#endif // QSUBTITLELABEL_H
