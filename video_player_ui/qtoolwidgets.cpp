#include "qtoolwidgets.h"
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QApplication>
#include <QSlider>
#include <QListView>
#include <QFileIconProvider>
#include <QDebug>
#include <QPropertyAnimation>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDesktopWidget>
#include "qplayfilelistmodel.h"
#include "qfilelistview.h"
QToolWidgets::QToolWidgets(QWidget *parent)
    : QWidget(parent)
{
    auto title = CreateTitle(parent);
    auto center = CreateCenterToolbar(parent);
    auto process = CreateProcessbar(parent);
    auto toolbar = CreateToolbar(parent);
    auto filelist = CreateFilelist(parent);

    auto layout = new QVBoxLayout(parent);
    auto layoutWd = new QVBoxLayout(this);
    auto layoutMid = new QHBoxLayout;
    auto layoutMidL = new QVBoxLayout;
    parent->setLayout(layout);
    layout->setContentsMargins(0, 10, 0, 10);
    layout->addWidget(this);

    this->setLayout(layoutWd);
    layoutWd->setMargin(0);
    layoutWd->setSpacing(0);
    layoutWd->addLayout(title);
    layoutWd->addLayout(layoutMid);
    layoutWd->addLayout(process);
    layoutWd->addLayout(toolbar);

    layoutMid->addStretch();
    layoutMid->addLayout(layoutMidL);
    layoutMid->addStretch();
    layoutMid->addWidget(filelist);

    layoutMidL->addStretch();
    layoutMidL->addLayout(center);
    layoutMidL->addStretch();
}

bool QToolWidgets::isUnderValid()
{
    return m_filelistIndicator->underMouse();
}

void QToolWidgets::selectMode(int index)
{
    qobject_cast<QPlayFileListModel*>(m_filelist->model())->setMode(index);
    m_openfile->setVisible(index == QPlayFileListModel::play_mode_local);
}

QBoxLayout *QToolWidgets::CreateTitle(QWidget * parent)
{
    auto name = new QLabel(parent);
    auto min = new QPushButton(parent);
    auto max = new QPushButton(parent);
    auto close = new QPushButton(parent);

    min->setObjectName("btn_min");
    max->setObjectName("btn_max");
    close->setObjectName("btn_close");

    auto layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(name);
    layout->addStretch();
    layout->addWidget(min);
    layout->addWidget(max);
    layout->addWidget(close);

    connect(min, &QPushButton::clicked, [parent]{
        parent->showMinimized();
    });
    connect(max, &QPushButton::clicked, [parent]{
        if(parent->isMaximized())
            parent->showNormal();
        else
            parent->showMaximized();
    });
    connect(close, &QPushButton::clicked, []{
        qApp->quit();
    });

    return layout;
}

QBoxLayout *QToolWidgets::CreateCenterToolbar(QWidget *parent)
{
    m_openfile = new QPushButton(tr("open media file..."), parent);
    m_openfile->setObjectName("btn_openfile");

    QFileIconProvider f;
    m_openfile->setIcon(f.icon(QFileIconProvider::File));

    auto layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_openfile);

    connect(m_openfile, &QPushButton::clicked,[this]{
        QFileDialog dialog(
            this,
            QString(),
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
            "All Files (*.*);;mp4 (*.mp4);;flv (*.flv);;avi (*.avi);;mkv (*.mkv)");
        dialog.setFileMode(QFileDialog::ExistingFiles);
        qDebug() << dialog.size();
        QStringList fileNames;
        if (dialog.exec())
            fileNames = dialog.selectedFiles();
        if(!fileNames.isEmpty())
            emit play(fileNames.at(0));
    });

    return layout;
}

QBoxLayout *QToolWidgets::CreateProcessbar(QWidget *parent)
{
    auto process = new QSlider(Qt::Orientation::Horizontal, parent);
    process->setObjectName("slider_pro");
    process->setValue(0);
    auto layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(process);

    return layout;
}

QBoxLayout *QToolWidgets::CreateToolbar(QWidget *parent)
{
    auto stop = new QPushButton(parent);
    auto prev = new QPushButton(parent);
    auto play = new QPushButton(parent);
    auto next = new QPushButton(parent);
    auto volMute = new QPushButton(parent);
    auto volNum = new QSlider(Qt::Orientation::Horizontal, parent);
    auto fullScreen = new QPushButton(parent);
    auto fileList = new QPushButton(parent);

    stop->setObjectName("btn_stop");
    prev->setObjectName("btn_prev");
    play->setObjectName("btn_pause");
    next->setObjectName("btn_next");
    volMute->setObjectName("btn_volume");
    volMute->setIcon(parent->style()->standardIcon(QStyle::SP_MediaVolume));
    volNum->setObjectName("slider_voice");
    fullScreen->setObjectName("btn_fullscreen");
    fileList->setObjectName("file_list");

    auto layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(stop);
    layout->addWidget(prev);
    layout->addWidget(play);
    layout->addWidget(next);
    layout->addWidget(volMute);
    layout->addWidget(volNum);
    layout->addStretch();
    layout->addWidget(fullScreen);
    layout->addWidget(fileList);

    play->setCheckable(true);
    play->setChecked(true);
    volMute->setCheckable(true);
    fullScreen->setCheckable(true);
    fullScreen->setChecked(false);
    connect(play, &QPushButton::released, [play, this]{
        qDebug() << "play:" << play->isChecked();
        if(play->isChecked()){
            emit this->pause();
            play->setObjectName("btn_pause");
        }else{
            emit this->play();
            play->setObjectName("btn_play");
        }

        play->setStyleSheet(qApp->styleSheet());
    });

    connect(stop, &QPushButton::clicked, this, &QToolWidgets::stop);
    connect(fullScreen, &QPushButton::released, [fullScreen, parent]{
        qDebug() << "fullscreen:" << fullScreen->isChecked();
        if(!parent->isFullScreen()){
            parent->showFullScreen();
        }else{
            parent->showNormal();
        }
    });

    connect(volMute, &QPushButton::clicked, [volMute, this]{
        if(!volMute->isChecked())
            volMute->setIcon(volMute->style()->standardIcon(QStyle::SP_MediaVolumeMuted));
        else
            volMute->setIcon(volMute->style()->standardIcon(QStyle::SP_MediaVolumeMuted));
        emit mute(volMute->isChecked());
    });

    connect(fileList, &QPushButton::clicked, [this]{
        m_filelistWd->setVisible(!m_filelistWd->isVisible());
    });

    return layout;
}

QWidget *QToolWidgets::CreateFilelist(QWidget *parent)
{
    m_filelistWd = new QWidget(parent);
    auto left = new QWidget(m_filelistWd);
    auto right = new QWidget(m_filelistWd);
    m_filelistIndicator = new QPushButton(">",left);
    m_filelist = new QFileListView(parent);
    auto toolbar = new QWidget(parent); // live or play
    auto localMode = new QPushButton("localMode", toolbar);
    auto liveMode = new QPushButton("liveMode", toolbar);

    m_filelistWd->setObjectName("file_list_wd");
    left->setObjectName("list_file_indicator");
    m_filelistIndicator->setObjectName("btn_file_indicator");
    m_filelist->setObjectName("list_file");
    localMode->setObjectName("btn_mode");
    liveMode->setObjectName("btn_mode");

    auto layout0 = new QHBoxLayout;
    auto layoutL = new QHBoxLayout;
    auto layoutR = new QGridLayout;
    m_filelistWd->setLayout(layout0);
    left->setLayout(layoutL);
    right->setLayout(layoutR);

    layout0->setSpacing(0);
    layout0->setMargin(0);
    layout0->addWidget(left);
    layout0->addWidget(right);

    layoutL->setMargin(0);
    layoutL->addWidget(m_filelistIndicator,0, Qt::AlignCenter);

    layoutR->setMargin(0);
    layoutR->setSpacing(0);
    layoutR->addWidget(m_filelist, 0, 0, 1, 2);
    layoutR->addWidget(localMode, 1, 0);
    layoutR->addWidget(liveMode, 1, 1);

    localMode->setCheckable(true);
    liveMode->setCheckable(true);
    auto modeGroup = new QButtonGroup(this);
    modeGroup->addButton(localMode, QPlayFileListModel::play_mode_local);
    modeGroup->addButton(liveMode, QPlayFileListModel::play_mode_live);
    localMode->setChecked(true);

    m_filelistWd->hide();
    connect(m_filelistIndicator, &QPushButton::clicked, [this, parent, right, left]{
        static auto an = new QPropertyAnimation(m_filelistWd, "pos");
        an->setDuration(200);

        if(!an->currentValue().isValid())
            an->setStartValue(m_filelistWd->pos());
        else
            an->setStartValue(an->currentValue());

        if(m_filelistIndicator->text() == ">"){
            m_filelistIndicator->setText("<");
            an->setEndValue(QPoint(parent->width() - m_filelistIndicator->width(), m_filelistWd->y()));
        }else{
            m_filelistIndicator->setText(">");
            right->show();
            an->setEndValue(QPoint(parent->width() - m_filelistWd->width(), m_filelistWd->y()));
            return;
        }

        an->start(/*QPropertyAnimation::DeleteWhenStopped*/);
        connect(an, &QPropertyAnimation::finished, [this, right]{
            if(m_filelistIndicator->text() == "<")
                right->hide();
        });
    });

    connect(modeGroup, SIGNAL(buttonClicked(int)), this, SLOT(selectMode(int)));
    return m_filelistWd;
}
