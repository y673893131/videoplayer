#include "qtoolwidgets.h"
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QApplication>
#include <QSlider>
#include <QListView>
#include <QLineEdit>
#include <QFileIconProvider>
#include <QDebug>
#include <QPropertyAnimation>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDesktopWidget>
#include <QTimer>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include "qplayfilelistmodel.h"
#include "qfilelistview.h"
#include "qprogressslider.h"
#include "config.h"
#include "qinputurlwidget.h"

QToolWidgets::QToolWidgets(QWidget *parent)
    : QWidget(parent), m_bPlaying(false)
    , m_index(0), m_playMode(QPlayFileListModel::play_mode_local)
{
    m_inputUrl = new QInputUrlWidget(parent);
    CreateMenu(parent);
    auto title = CreateTitle(parent);
    auto center = CreateCenterToolbar(parent);
    auto process = CreateProcessbar(parent);
    auto toolbar = CreateToolbar(parent);
    auto filelist = CreateFilelist(parent);
    auto timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(3000);

    auto layout = new QVBoxLayout(parent);
    auto layoutWd = new QVBoxLayout(this);
    auto layoutMid = new QHBoxLayout;
    auto layoutMidL = new QVBoxLayout;
    parent->setLayout(layout);
    layout->setMargin(0);
    layout->addWidget(this);

    this->setLayout(layoutWd);
    layoutWd->setMargin(0);
    layoutWd->setSpacing(0);
    layoutWd->addWidget(title);
    layoutWd->addLayout(layoutMid);
    layoutWd->addLayout(process);
    layoutWd->addWidget(toolbar);

    layoutMid->addStretch();
    layoutMid->addLayout(layoutMidL);
    layoutMid->addStretch();
    layoutMid->addWidget(filelist);

    layoutMidL->addStretch();
    layoutMidL->addLayout(center);
    layoutMidL->addStretch();

    connect(this, &QToolWidgets::start, [timer, this](int index)
    {
        timer->start();
        qDebug() << "start:" << index;
        m_index = index;
    });
    connect(this, &QToolWidgets::hideOrShow, timer, &QTimer::stop);
    connect(timer, &QTimer::timeout, [this]
    {
        if(m_bPlaying)
            emit hideOrShow(true);
    });

    connect(this,&QToolWidgets::selectMode, this, &QToolWidgets::onSelectMode);
    connect(this,&QToolWidgets::setTotalSeconds, [this]{m_bPlaying = true; });
    connect(this,&QToolWidgets::stop, [this]{m_bPlaying = false; });
    connect(this,&QToolWidgets::move, [timer, this]
    {
        emit hideOrShow(false);
        timer->start();
    });
}

bool QToolWidgets::isUnderValid()
{
    return m_filelist->isVerticalUnder() || m_filelist->underMouse();
}

int QToolWidgets::index()
{
    return m_index;
}

void QToolWidgets::onLoadFile()
{
    auto path = GET_CONFIG_DATA(Config::Data_Path).toString();
    if(path.isEmpty())
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QFileDialog dialog(
        nullptr,
        QString(),
        path,
        "All Files (*.*);;mp4 (*.mp4);;flv (*.flv);;avi (*.avi);;mkv (*.mkv);;rmvb (*.rmvb);;url (*.urls)");
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.show();
    auto desktopSize = qApp->desktop()->size();
    dialog.move((desktopSize.width() - dialog.width()) / 2, (desktopSize.height() - dialog.height()) / 2);
    QStringList fileNames;
    if (dialog.exec())
        fileNames = dialog.selectedFiles();
    qDebug() << fileNames;
    if(!fileNames.isEmpty())
    {
        auto curPath = dialog.directory().path();
        if(path != curPath)
            SET_CONFIG_DATA(curPath, Config::Data_Path);
        emit play(fileNames.at(0));
    }
}

void QToolWidgets::onSelectMode(int index)
{
    qobject_cast<QPlayFileListModel*>(m_filelist->model())->setMode(index);
    m_openfile->setVisible(index == QPlayFileListModel::play_mode_local && !m_bPlaying);
    m_playMode = index;
}

QWidget *QToolWidgets::CreateTitle(QWidget * parent)
{
    auto widget = new QWidget(parent);
    auto name = new QLabel(qApp->applicationName(),widget);
    auto min = new QPushButton(widget);
    auto max = new QPushButton(widget);
    auto close = new QPushButton(widget);

    widget->setObjectName("wd_title");
    name->setObjectName("label_title");
    min->setObjectName("btn_min");
    max->setObjectName("btn_max");
    close->setObjectName("btn_close");

    min->setToolTip(tr("minimize"));
    max->setToolTip(tr("maximize"));
    close->setToolTip(tr("close"));

    auto layout = new QHBoxLayout(widget);
    layout->setSpacing(0);
    layout->setMargin(10);
    layout->addStretch();
    layout->addWidget(name);
    layout->addStretch();
    layout->addWidget(min);
    layout->addWidget(max);
    layout->addWidget(close);

    connect(min, &QPushButton::clicked, [parent]
    {
        parent->showMinimized();
    });
    connect(max, &QPushButton::clicked, [parent]
    {
        if(parent->isMaximized())
            parent->showNormal();
        else
            parent->showMaximized();
    });
    connect(close, &QPushButton::clicked, this, &QToolWidgets::exit);
    connect(this, &QToolWidgets::hideOrShow, [name, min, max, close, widget, this](bool bHide)
    {
        name->setHidden(bHide);
        min->setHidden(bHide);
        max->setHidden(bHide);
        close->setHidden(bHide);
        widget->setHidden(bHide);
        if(bHide)
        {
            if(cursor().shape() != Qt::BlankCursor)
                setCursor(Qt::BlankCursor);
        }
        else if(cursor().shape() == Qt::BlankCursor)
            setCursor(Qt::ArrowCursor);
    });

    connect(this, &QToolWidgets::play, [name](const QString& sUrl)
    {
        auto sName = sUrl.mid(sUrl.lastIndexOf('/') + 1);
        name->setText(sName);
    });

    return widget;
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

    connect(m_openfile, &QPushButton::clicked, this, &QToolWidgets::onLoadFile);
    connect(this, &QToolWidgets::loadFile, this, &QToolWidgets::onLoadFile);

    connect(this, &QToolWidgets::setTotalSeconds, [this]{
        m_openfile->hide();
    });
    return layout;
}

QBoxLayout *QToolWidgets::CreateProcessbar(QWidget *parent)
{
    m_process = new QProgressSlider(Qt::Orientation::Horizontal, parent);
    m_process->setObjectName("slider_pro");
    m_process->setValue(0);
    m_process->setDisabled(true);
    auto layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_process);

    connect(this, &QToolWidgets::start, [this]{
        m_process->setEnabled(true);
    });

    connect(this, &QToolWidgets::stop, this, [this]{
        m_process->setDisabled(true);
    }, Qt::QueuedConnection);

    connect(this,&QToolWidgets::setTotalSeconds, [this]{m_process->setHidden(true); });
    connect(this, &QToolWidgets::hideOrShow, [this](bool bHide){
        m_process->setHidden(bHide || m_playMode == QPlayFileListModel::play_mode_live);
    });

    connect(this, &QToolWidgets::setTotalSeconds, [this](int nSeconds)
    {
        qDebug() << nSeconds;
        m_process->setRange(0, nSeconds);
    });
    connect(this, &QToolWidgets::setPosSeconds, [this](int nSeconds)
    {
        m_process->setValue(nSeconds);
    });

    connect(m_process, &QProgressSlider::sliderMoved, this, &QToolWidgets::setSeekPos);

    connect(this, &QToolWidgets::selectMode, [this](int mode)
    {
        bool bShow = (mode == QPlayFileListModel::play_mode_local);
        m_process->setVisible(bShow);
    });

    return layout;
}

QWidget *QToolWidgets::CreateToolbar(QWidget *parent)
{
    auto widget = new QWidget(parent);
    auto stop = new QPushButton(widget);
    auto prev = new QPushButton(widget);
    auto play = new QPushButton(widget);
    auto next = new QPushButton(widget);
    auto volMute = new QPushButton(widget);
    auto volNum = new QSlider(Qt::Orientation::Horizontal, widget);
    auto fileList = new QPushButton(widget);
    auto framRate = new QLabel(widget);

    widget->setObjectName("wd_toolbar");
    stop->setObjectName("btn_stop");
    prev->setObjectName("btn_prev");
    play->setObjectName("btn_pause");
    next->setObjectName("btn_next");
    volMute->setObjectName("btn_volume");
    volMute->setIcon(parent->style()->standardIcon(QStyle::SP_MediaVolume));
    volNum->setObjectName("slider_voice");
    fileList->setObjectName("file_list");
    fileList->setToolTip(tr("play list"));
    framRate->setObjectName("label_frame_rate");

    auto layout = new QHBoxLayout(widget);
    layout->setSpacing(0);
    layout->setMargin(10);
    layout->addWidget(stop);
    layout->addWidget(prev);
    layout->addWidget(play);
    layout->addWidget(next);
    layout->addWidget(volMute);
    layout->addWidget(volNum);
    layout->addStretch();
    layout->addWidget(framRate);
    layout->addSpacing(10);
    layout->addWidget(fileList);

    volNum->setRange(0, 100);
    volNum->setValue(50);
    volMute->setCheckable(true);
    auto playFunc = [play]
    {
        play->setObjectName("btn_play");
        play->setStyleSheet(qApp->styleSheet());
    };
    auto pauseFunc = [play]
    {
        play->setObjectName("btn_pause");
        play->setStyleSheet(qApp->styleSheet());
    };
    connect(this, &QToolWidgets::setTotalSeconds, playFunc);
    connect(this, &QToolWidgets::continuePlay, playFunc);
    connect(this, &QToolWidgets::pause, this, pauseFunc, Qt::QueuedConnection);
    connect(this, &QToolWidgets::stop, this, pauseFunc, Qt::QueuedConnection);
    connect(play, &QPushButton::released, [play, this]
    {
        if(m_bPlaying)
        {
            if(play->objectName() == "btn_pause")
                emit continuePlay();
            else
                emit pause();
            return;
        }

        if(!m_filelist->currentIndex().isValid())
        {
            emit loadFile();
            play->setChecked(!play->isChecked());
            return;
        }
        else
        {
            auto url = m_filelist->currentIndex().data(QPlayFileListModel::role_url).toString();
            emit this->play(url);
        }
    });

    connect(stop, &QPushButton::clicked, this, &QToolWidgets::stop);
    connect(volNum, &QSlider::valueChanged, this, &QToolWidgets::setVol);
    connect(volMute, &QPushButton::toggled, [volMute, this](bool bChecked)
    {
        if(!bChecked)
            volMute->setIcon(volMute->style()->standardIcon(QStyle::SP_MediaVolume));
        else
            volMute->setIcon(volMute->style()->standardIcon(QStyle::SP_MediaVolumeMuted));
        SET_CONFIG_DATA(bChecked, Config::Data_Mute);
        emit mute(bChecked);
    });

    connect(fileList, &QPushButton::clicked, [this]
    {
        m_filelistWd->setVisible(!m_filelistWd->isVisible());
    });

    connect(this, &QToolWidgets::hideOrShow, [widget,this](bool bHide)
    {
        widget->setHidden(bHide);
        if(bHide)
            m_filelistWd->setHidden(bHide);
    });

    connect(this, &QToolWidgets::selectMode, [stop, prev, play, next](int mode)
    {
        bool bShow = (mode == QPlayFileListModel::play_mode_local);
        stop->setVisible(bShow);
        prev->setVisible(bShow);
//        play->setVisible(bShow);
        next->setVisible(bShow);
    });

    connect(prev, &QPushButton::clicked, [this, play]
    {
        QModelIndex index = m_filelist->currentIndex();
        if(!index.isValid())
            index = m_filelist->model()->index(0, 0);
        else
        {
            auto m = m_filelist->model();
            if(index.row() > 0)
                index = m->index(index.row() - 1, 0);
            else
                index = m->index(m->rowCount() - 1, 0);
        }
        if(!index.isValid())
        {
            emit loadFile();
            play->setChecked(!play->isChecked());
            return;
        }
        else
        {
            m_filelist->setCurrentIndex(index);
            auto url = index.data(QPlayFileListModel::role_url).toString();
            emit this->play(url);
        }
    });

    connect(next, &QPushButton::clicked, [this, play]
    {
        QModelIndex index = m_filelist->currentIndex();
        if(!index.isValid())
            index = m_filelist->model()->index(0, 0);
        else
        {
            auto m = m_filelist->model();
            auto rows = m->rowCount();
            if(rows > index.row() + 1)
                index = m->index(index.row() + 1, 0);
            else
                index = m->index(0, 0);
        }
        if(!index.isValid())
        {
            emit loadFile();
            play->setChecked(!play->isChecked());
            return;
        }
        else
        {
            m_filelist->setCurrentIndex(index);
            auto url = index.data(QPlayFileListModel::role_url).toString();
            emit this->play(url);
        }
    });

    connect(this, &QToolWidgets::frameRate, [framRate](int video)
    {
       framRate->setText(QString("%1: %2/s").arg(tr("video")).arg(video));
    });

    connect(Config::instance(), &Config::loadConfig, [volNum, volMute]
    {
        volNum->setValue(GET_CONFIG_DATA(Config::Data_Vol).toInt());
        volMute->setChecked(GET_CONFIG_DATA(Config::Data_Mute).toBool());
    });

    auto timer = new QTimer(this);
    timer->setInterval(500);
    timer->setSingleShot(true);
    connect(volNum, &QSlider::valueChanged, [timer]
    {
        timer->stop();
        timer->start();
    });

    connect(timer, &QTimer::timeout, [volNum]
    {
        SET_CONFIG_DATA(volNum->value(), Config::Data_Vol);
    });


    return widget;
}

QWidget *QToolWidgets::CreateFilelist(QWidget *parent)
{
    m_filelistWd = new QWidget(parent);
    auto right = new QWidget(m_filelistWd);
    m_filelist = new QFileListView(parent);
    auto searchEdit = new QLineEdit(m_filelistWd);
    auto localMode = new QPushButton(tr("localMode"), m_filelist);
    auto liveMode = new QPushButton(tr("liveMode"), m_filelist);

    searchEdit->setPlaceholderText(tr("search"));

    m_filelistWd->setObjectName("file_list_wd");
    m_filelist->setObjectName("list_file");
    searchEdit->setObjectName("file_search_edit");
    localMode->setObjectName("btn_mode");
    liveMode->setObjectName("btn_mode");

    auto layout0 = new QVBoxLayout;
    auto layoutR = new QGridLayout;
    m_filelistWd->setLayout(layout0);
    right->setLayout(layoutR);

    layout0->setSpacing(0);
    layout0->setMargin(0);
    layout0->addWidget(right);

    layoutR->setMargin(0);
    layoutR->setSpacing(0);
    layoutR->addWidget(searchEdit, 0, 0, 1, 2);
    layoutR->addWidget(m_filelist, 1, 0, 1, 2);
    layoutR->addWidget(localMode, 2, 0);
    layoutR->addWidget(liveMode, 2, 1);

    localMode->setCheckable(true);
    liveMode->setCheckable(true);
    auto modeGroup = new QButtonGroup(this);
    modeGroup->addButton(localMode, QPlayFileListModel::play_mode_local);
    modeGroup->addButton(liveMode, QPlayFileListModel::play_mode_live);
    localMode->setChecked(true);

    m_filelistWd->hide();
    connect(modeGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(selectMode(int)));
    connect(m_filelist, &QFileListView::select, [this](const QString& url)
    {
        emit play(url);
    });

    connect(m_filelist, &QFileListView::loadFile, this, &QToolWidgets::onLoadFile);
    connect(this, &QToolWidgets::play, m_filelist, &QFileListView::addLocalUrl);
    connect(this, &QToolWidgets::inputUrlFile, m_filelist, &QFileListView::inputUrlFile);

    connect(searchEdit, &QLineEdit::textChanged, m_filelist, &QFileListView::filter);

    return m_filelistWd;
}

void QToolWidgets::CreateMenu(QWidget *parent)
{
    auto menu = new QMenu(parent);
    auto actionAdjust = menu->addAction(tr("adjust"));
    auto topWindow = menu->addAction(tr("topWindow"));
    auto urlWindow = menu->addAction(tr("url"));
    actionAdjust->setCheckable(true);
    topWindow->setCheckable(true);

    menu->setObjectName("menu1");
    actionAdjust->setChecked(GET_CONFIG_DATA(Config::Data_Adjust).toBool());
    topWindow->setChecked(GET_CONFIG_DATA(Config::Data_TopWindow).toBool());
    connect(this, &QToolWidgets::showMenu, [menu]{ menu->popup(QCursor::pos());});
    connect(actionAdjust, &QAction::triggered, this, &QToolWidgets::viewAdjust);
    connect(topWindow, &QAction::triggered, this, &QToolWidgets::topWindow);
    connect(Config::instance(), &Config::loadConfig, [actionAdjust, topWindow]
    {
        actionAdjust->setChecked(GET_CONFIG_DATA(Config::Data_Adjust).toBool());
        topWindow->setChecked(GET_CONFIG_DATA(Config::Data_TopWindow).toBool());
    });

    connect(actionAdjust, &QAction::triggered, [](bool bCheck)
    {
       SET_CONFIG_DATA(bCheck, Config::Data_Adjust);
    });

    connect(topWindow, &QAction::triggered, [](bool bCheck)
    {
       SET_CONFIG_DATA(bCheck, Config::Data_TopWindow);
    });

//    connect(urlWindow, &QAction::triggered, [this]
//    {
//        if(m_bPlaying)
//        {
//            emit stop();
//        }

//        m_inputUrl->exec();
//        emit inputUrl();
//        auto sName = "rtmp://172.16.62.127/live/stream";
//        emit this->play(sName);
//        return;
//    });

    connect(urlWindow, &QAction::triggered, m_inputUrl, &QInputUrlWidget::showInit);
    connect(m_inputUrl, &QInputUrlWidget::inputUrl, [this](const QString& url)
    {
        if(m_bPlaying)
            emit stop();

        emit play(url);
    });
}

void QToolWidgets::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if(event->buttons() & Qt::RightButton)
        emit showMenu();
}
