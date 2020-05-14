#include "framelesswidget.h"
#include <QPainter>
#include <Windows.h>
#include <windowsx.h>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QTimer>
#include <QDebug>
QFrameLessWidget::QFrameLessWidget(QWidget *parent)
    : QWidget(parent), m_drag(DragMove_None),m_bkColor("#80C7ED")
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(800, 600);
    m_normalSize = size();
    m_minSize = QSize(600, 400);
    auto deskCenter = qApp->desktop()->rect().center();
    m_doubleClickPos = QPoint(deskCenter.x() - width() / 2, deskCenter.y() - height() / 2);
}

QFrameLessWidget::~QFrameLessWidget()
{
}

void QFrameLessWidget::resizeBackground(int w, int h, int round, int margin, QColor color)
{
    if(height() == qApp->desktop()->availableGeometry().height())
        round = margin = 0;
    m_bkImg = QImage(size(), QImage::Format::Format_ARGB32);
    m_bkImg.fill(QColor(0,0,0,0));
    QPainter painter(&m_bkImg);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);

    QPainterPath painterPath;
    painter.setPen(color);
    painterPath.addRoundedRect(margin, margin, w - 2 * margin, h - 2 * margin, round, round);
    painter.fillPath(painterPath, color);

    for (int i = 0; i < margin; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRoundedRect(margin - i, margin - i, w - (margin - i) * 2, h - (margin - i) * 2, round, round);
        color.setAlpha(static_cast<int>((margin - i - 1) / (1.0 * margin) * 192));
        QPen pen(color);
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawPath(path);
    }
}

void QFrameLessWidget::region(const QPoint &cursorGlobalPoint, bool &activeFlag)
{
    if(isValid()){
        m_dir = -1;
        this->setCursor(QCursor(Qt::ArrowCursor));
        activeFlag = false;
        return;
    }

    QRect rect = this->rect();
    QPoint tl = mapToGlobal(rect.topLeft());
    QPoint rb = mapToGlobal(rect.bottomRight());
    int x = cursorGlobalPoint.x();
    int y = cursorGlobalPoint.y();
    activeFlag = true;
    if (tl.x() + PADDING + 2 >= x && tl.x() <= x && tl.y() + PADDING + 2 >= y && tl.y() <= y) {
        m_dir = WMSZ_TOPLEFT;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    } else if (x >= rb.x() - PADDING - 2 && x <= rb.x() && y >= rb.y() - PADDING - 2 && y <= rb.y()) {
        m_dir = WMSZ_BOTTOMRIGHT;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    } else if (x <= tl.x() + PADDING + 2 && x >= tl.x() && y >= rb.y() - PADDING - 2 && y <= rb.y()) {
        m_dir = WMSZ_BOTTOMLEFT;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    } else if (x <= rb.x() && x >= rb.x() - PADDING - 2 && y >= tl.y() && y <= tl.y() + PADDING + 2) {
        m_dir = WMSZ_TOPRIGHT;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    } else if (x <= tl.x() + PADDING && x >= tl.x()) {
        m_dir = WMSZ_LEFT;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    } else if (x <= rb.x() && x >= rb.x() - PADDING) {
        m_dir = WMSZ_RIGHT;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    } else if (y >= tl.y() && y <= tl.y() + PADDING){
        m_dir = WMSZ_TOP;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    } else if (y <= rb.y() && y >= rb.y() - PADDING) {
        m_dir = WMSZ_BOTTOM;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    } else {
        m_dir = -1;
        this->setCursor(QCursor(Qt::ArrowCursor));
        activeFlag = false;
    }
}

void QFrameLessWidget::checkMouseMoveRegion()
{
    bool activeFlag;
    region(cursor().pos(), activeFlag);
}

bool QFrameLessWidget::checkLButtonPressRegion()
{
    bool activeFlag;
    region(cursor().pos(), activeFlag);
    if (activeFlag && ReleaseCapture() && m_dir > 0) {
        SendMessage(reinterpret_cast<HWND>(this->winId()), WM_SYSCOMMAND, SC_SIZE | m_dir, 0);
        return true;
    }

    return false;
}

void QFrameLessWidget::checkSize(void *arg)
{
    auto rc = reinterpret_cast<RECT*>(arg);
    if(rc->bottom - rc->top >= m_minSize.height() && rc->right - rc->left >= m_minSize.width()) return;
    switch (m_dir) {
    case WMSZ_TOPLEFT:
        if(rc->bottom - rc->top < m_minSize.height()) rc->top = rc->bottom - m_minSize.height();
        if(rc->right - rc->left < m_minSize.width()) rc->left = rc->right - m_minSize.width();
//        qDebug() << rc->bottom - rc->top << m_minSize.height();
        break;
    case WMSZ_BOTTOMRIGHT:
        if(rc->bottom - rc->top < m_minSize.height()) rc->bottom = rc->top + m_minSize.height();
        if(rc->right - rc->left < m_minSize.width()) rc->right = rc->left + m_minSize.width();
        break;
    case WMSZ_BOTTOMLEFT:
        if(rc->bottom - rc->top < m_minSize.height()) rc->bottom = rc->top + m_minSize.height();
        if(rc->right - rc->left < m_minSize.width()) rc->left = rc->right - m_minSize.width();
        break;
    case WMSZ_TOPRIGHT:
        if(rc->bottom - rc->top < m_minSize.height()) rc->top = rc->bottom - m_minSize.height();
        if(rc->right - rc->left < m_minSize.width()) rc->right = rc->left + m_minSize.width();
        break;
    case WMSZ_LEFT:
        rc->left = rc->right - m_minSize.width();
        break;
    case WMSZ_RIGHT:
        rc->right = rc->left + m_minSize.width();
        break;
    case WMSZ_TOP:
        rc->top = rc->bottom - m_minSize.height();
        break;
    case WMSZ_BOTTOM:
        rc->bottom = rc->top + m_minSize.height();
        break;
    }
}

void QFrameLessWidget::checkDragMove(void *arg)
{
    QPoint pos = cursor().pos();
    auto rc = reinterpret_cast<RECT*>(arg);
    if(pos.x() < PADDING){//DragMove_Left
        if(m_drag == DragMove_None) {m_dragRc = QRect(rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top);}
        rc->top = 0;
        rc->bottom = qApp->desktop()->availableGeometry().height();
        rc->left = 0;
        rc->right = qApp->desktop()->availableGeometry().width() / 2;
        m_drag = DragMove_Left;
    } else if(pos.y() < PADDING){//DragMove_Top
        if(m_drag == DragMove_None) {m_dragRc = QRect(rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top);}
        rc->top = 0;
        rc->bottom = qApp->desktop()->availableGeometry().height();
        rc->left = 0;
        rc->right = qApp->desktop()->availableGeometry().width();
        m_drag = DragMove_Top;
    } else if(abs(pos.x() - qApp->desktop()->availableGeometry().width()) < PADDING){//DragMove_Right
        if(m_drag == DragMove_None) {m_dragRc = QRect(rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top);}
        rc->top = 0;
        rc->bottom = qApp->desktop()->availableGeometry().height();
        rc->left = qApp->desktop()->availableGeometry().width() / 2;
        rc->right = qApp->desktop()->availableGeometry().width();
        m_drag = DragMove_Right;
    } else if(rc->bottom - rc->top != this->height() || rc->right - rc->left != this->width()){//DragMove_None
        if(m_drag != DragMove_Restore && m_drag != DragMove_None){
            rc->left = m_dragRc.x();
            rc->top = m_dragRc.y();
            rc->right = m_dragRc.right();
            rc->bottom = m_dragRc.bottom();
        }
        rc->right = rc->left + m_normalSize.width();
        rc->bottom = rc->top + m_normalSize.height();
        m_drag = DragMove_None;
    } else if(m_drag != DragMove_None && m_drag != DragMove_Restore){//DragMove_Restore
        m_dragRc.setX(pos.x() - static_cast<int>((pos.x() - rc->left) * 1.0 / this->width() * m_normalSize.width()));
        m_dragRc.setY(pos.y() - static_cast<int>((pos.y() - rc->top) * 1.0 / this->height() * m_normalSize.height()));
        m_dragRc.setSize(m_normalSize);
        m_drag = DragMove_Restore;
        m_dragLastPos = pos;
    }else if(m_drag == DragMove_Restore && m_dragLastPos != pos)
    {
        rc->top = m_dragRc.y();
        rc->left = m_dragRc.x();
        rc->bottom = m_dragRc.bottom();
        rc->right = m_dragRc.right();
    }
}

void QFrameLessWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        if(::ReleaseCapture()){
            ::SendMessage(HWND(this->winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
            QTimer::singleShot(0, [&]() {
                if(m_drag == DragMove_None)
                    m_doubleClickPos = pos();
            });
        }
    }

    QWidget::mousePressEvent(event);
}

void QFrameLessWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.drawImage(0, 0, m_bkImg.scaled(this->size()));
}

bool QFrameLessWidget::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result)
    auto msg = reinterpret_cast<MSG*>(message);
//    qDebug() << msg->message << msg->wParam << msg->lParam;
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
    {
        auto msg = reinterpret_cast<MSG*>(message);
        if(msg->hwnd != reinterpret_cast<HWND>(this->winId())) return false;
        if (msg->message == WM_MOUSEMOVE)
            checkMouseMoveRegion();
        else if (msg->message == WM_LBUTTONDOWN){
            if(checkLButtonPressRegion()) return true;
        } else if (msg->message == WM_SIZING)
            checkSize(reinterpret_cast<void*>(msg->lParam));
        else if (msg->message == WM_MOVING)
            checkDragMove(reinterpret_cast<void*>(msg->lParam));
    }
    return false;
}

void QFrameLessWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if(event->button() == Qt::LeftButton)
    {
//        auto desktopsize = qApp->desktop()->availableGeometry().size();
//        if(size() != desktopsize && m_drag == DragMove_None){

//            SetWindowPos(reinterpret_cast<HWND>(this->winId()), HWND_TOP, 0,0, desktopsize.width(),desktopsize.height(), SWP_SHOWWINDOW);
//            m_doubleClickPos = pos();
//            m_drag = DragMove_Top;
//        }
//        else{
//            SetWindowPos(reinterpret_cast<HWND>(this->winId()), HWND_TOP, m_doubleClickPos.x(),m_doubleClickPos.y(), m_normalSize.width(),m_normalSize.height(), SWP_SHOWWINDOW);
//            m_drag = DragMove_None;
//        }

        if(!isFullScreen()){
            showFullScreen();
            m_doubleClickPos = pos();
            m_drag = DragMove_Top;
        }else
        {
            showNormal();
            m_drag = DragMove_None;
        }
    }
}

void QFrameLessWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    if(height() != qApp->desktop()->availableGeometry().height())
        m_normalSize = size();
    resizeBackground(width(), height(), 5, 5, m_bkColor);//set your like
}

void QFrameLessWidget::setBackgroundColor(QColor c)
{
    m_bkColor = c;
}

bool QFrameLessWidget::isValid()
{
    return false;
}

void QFrameLessWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        if(isFullScreen()){
            showNormal();
            m_drag = DragMove_None;
        }
        break;
    }
}
