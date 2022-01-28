#include "qroomlistdelegate.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QWidget>
#include <QModelIndex>
#include <QFont>
#include "qroomlistviewmodel.h"

QRoomListDelegate::QRoomListDelegate(QObject* parent)
    :QStyledItemDelegate(parent), m_model(nullptr)
{
}

void QRoomListDelegate::setModel(QRoomListViewModel *model)
{
    m_model = model;
}

void QRoomListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);

    QStyledItemDelegate::paint(painter, option, index);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHints(QPainter::SmoothPixmapTransform, true);
    auto rc = viewOption.rect;
    auto info = reinterpret_cast<_DyRoom_*>(index.data(QRoomListViewModel::room_info_role).value<void*>());
    int borderWidth = 2;
    int offset = borderWidth;
    int padding = 3;
    int scalW = rc.width() - 2*offset;
    int scalH = rc.height() - 80;

    QColor borderColor(0x2E, 0x2F, 0x30);
    if(viewOption.state.testFlag(QStyle::State_MouseOver))
    {
        borderColor = "red";
        padding = 5;
    }

    {
        //img
        painter->save();
        QPainterPath path;
        path.addRoundedRect(rc.x() + offset, rc.y() + offset, scalW, scalH, 5, 5);
        painter->setClipPath(path);
        if(info && info->pm && !info->pm->isNull())
            painter->drawPixmap(rc.x() + offset, rc.y() + offset, scalW, scalH, *info->pm);
        else
            painter->fillPath(path, QColor(0x70, 0x70, 0x70));
        painter->restore();
    }
    {
        // text
        painter->save();
        int nLineHeight = 25;
        int nPosY = 5;
        QFont font;
        QTextOption tp;
        tp.setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        font.setPixelSize(14);
        font.setFamily("Microsoft YaHei");
        painter->setFont(font);

        // class
        painter->setPen(QColor(0x70, 0x70, 0x70));
        painter->drawText(QRect(rc.x() + borderWidth + padding, rc.y() + borderWidth + scalH + nPosY, scalW - 2 * borderWidth - 2 * padding, nLineHeight), info->sC2Name, QTextOption(Qt::AlignRight));

        // room name
        QFontMetrics fm(font);
#if (QT_VERSION < QT_VERSION_CHECK(5,11,0))
        int nWidth = fm.width(info->sC2Name);
#else
        int nWidth = fm.horizontalAdvance(info->sC2Name);
#endif
        auto sNewRn = fm.elidedText(info->sRn, Qt::ElideRight, scalW - 2 * borderWidth - 2 * padding - nWidth);

        painter->setPen("black");
        painter->drawText(QRect(rc.x() + borderWidth + padding, rc.y() + borderWidth + scalH + nPosY, scalW - 2 * borderWidth - 2 * padding - nWidth, nLineHeight), sNewRn);
        nPosY += nLineHeight;

        // name
        painter->setPen(QColor(0x70, 0x70, 0x70));
        font.setPixelSize(12);
        painter->setFont(font);

        painter->drawText(QRect(rc.x() + borderWidth + padding, rc.y() + borderWidth + scalH + nPosY, scalW - 2 * borderWidth - 2 * padding, nLineHeight), info->sName + "(" + info->sRid + ")");

        // online count
        QString sOnline;
        if(info->nOnlineCount > 10000)
        {
            int w = info->nOnlineCount / 10000;
            int d = info->nOnlineCount % 10000;
            d = d / 1000;
            if(d)
                sOnline = QString("%1.%2%3").arg(w).arg(d).arg(tr("wan"));
            else
                sOnline = QString("%1%2").arg(w).arg(tr("wan"));
        }
        else
            sOnline = QString::number(info->nOnlineCount);
        painter->setPen(QColor(0x50, 0x50, 0x50));

        painter->drawText(QRect(rc.x() + borderWidth + padding, rc.y() + borderWidth + scalH + nPosY, scalW - 2 * borderWidth - 2 * padding, nLineHeight), sOnline, tp);
        nPosY += nLineHeight;

        // od
        if(!info->sOd.isEmpty())
        {
            painter->setPen(QColor(0xFF, 0x77, 0x23));
            font.setPixelSize(12);
            painter->setFont(font);
            painter->drawText(QRect(rc.x() + borderWidth + padding, rc.y() + borderWidth + scalH + nPosY, scalW - 2 * borderWidth - 2 * padding, nLineHeight), info->sOd);
//            nPosY += nLineHeight;
        }

        painter->restore();
    }
}
