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
#include <QPainter>
#include <QStackedWidget>
#include <QTimer>
#include <QTime>
#include <QToolTip>
#include "playlist/qplayfilelistmodel.h"
#include "playlist/qfilelistview.h"
#include "progress/qprogressslider.h"
#include "config/config.h"
#include "ui/qinputurlwidget.h"
#include "platform/platform/qdouyuwidget.h"
#include "platform/platform/qliveplatformmanager.h"
#include "control/videocontrol.h"
#ifdef Q_OS_WIN
#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#endif

QToolWidgets::QToolWidgets(QWidget *parent)
    : QWidget(parent)
    , m_bPlaying(false)
    , m_bLocalFile(false)
    , m_index(0)
    , m_playMode(QPlayFileListModel::play_mode_local)
    , m_totalSeconds(0)
{
    init(parent);
}

void QToolWidgets::init(QWidget *parent)
{
    initStyle();
    initUi(parent);
    qApp->installNativeEventFilter(this);
}

void QToolWidgets::initStyle()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window /*| Qt::SubWindow*/);
    setAttribute(Qt::WA_TranslucentBackground);
#ifdef Q_OS_WIN
    auto hwnd = reinterpret_cast<HWND>(this->winId());
    setAreo(hwnd);
    setShadow(hwnd);
#endif
}

void QToolWidgets::initUi(QWidget *parent)
{
    setObjectName("toolWd");
    m_inputUrl = new QInputUrlWidget(this);
    CreateMenu(parent);
    auto widget = new QWidget(this);
    CreateSubTitle(widget);
    auto title = CreateTitle(widget);
    auto center = CreateCenterToolbar(widget);
    auto process = CreateProcessbar(widget);
    auto toolbar = CreateToolbar(widget);
    auto leftWd = CreateLeftlist(widget);
    auto filelist = CreateFilelist(widget);

    auto autoHidetimer = new QTimer(this);
    autoHidetimer->setSingleShot(true);
    autoHidetimer->setInterval(3000);

    auto layoutThis = new QVBoxLayout(this);
    auto layoutWd = new QVBoxLayout(widget);
    auto layoutMid = new QHBoxLayout;
    auto layoutMidCenter = new QVBoxLayout;

    layoutThis->setMargin(0);
    layoutThis->addWidget(widget);

    layoutWd->setMargin(0);
    layoutWd->setSpacing(0);
    layoutWd->addWidget(title);
    layoutWd->addLayout(layoutMid);
    layoutWd->addLayout(process);
    layoutWd->addWidget(toolbar);

    layoutMid->setContentsMargins(5, 0, 0, 0);
    layoutMid->addWidget(leftWd);
    layoutMid->addStretch();
    layoutMid->addLayout(layoutMidCenter);
    layoutMid->addStretch();
    layoutMid->addWidget(filelist);

    layoutMidCenter->addStretch();
    layoutMidCenter->addLayout(center);
    layoutMidCenter->addStretch();

    initSize();

    connect(this, &QToolWidgets::start, [autoHidetimer, this](int index)
    {
        autoHidetimer->start();
        m_index = index;
    });

    connect(this, &QToolWidgets::hideOrShow, autoHidetimer, &QTimer::stop);
    connect(autoHidetimer, &QTimer::timeout, [this]
    {
        if(m_bPlaying)
            emit hideOrShow(true);
    });

    connect(this,&QToolWidgets::selectMode, this, &QToolWidgets::onSelectMode);
    connect(this,&QToolWidgets::setTotalSeconds, [this]{m_bPlaying = true; });
    connect(this,&QToolWidgets::stop, [this]{m_bPlaying = false; });
    connect(this,&QToolWidgets::mouseMove, [autoHidetimer, title, toolbar, this]
    {
        emit hideOrShow(false);
        autoHidetimer->start();
        auto pos = QCursor::pos();
        pos = mapFromGlobal(pos);
        auto y0 = title->pos().y() + title->height();
        auto y1 = toolbar->pos().y();
        if(pos.x() < 25 && pos.y() > y0 && pos.y() < y1 && m_douyu->isHidden())
            m_livePlatformWd->setVisible(true);
    });
}

void QToolWidgets::initSize()
{
    CALC_WIDGET_WIDTH(m_livePlatformWd, 200.0f / 1920);
    CALC_WIDGET_WIDTH(m_filelistWd, 200.0f / 1920);
    CALC_WIDGET_WIDTH(m_title, 100.0f / 1920);
    CALC_WIDGET_HEIGHT(m_titleWd, 60.0f / 1920);
    CALC_WIDGET_HEIGHT(m_toolWd, 150.0f / 1920);
    CALC_WIDGET_HEIGHT(m_subtitleWd, 150.0f / 1920);
}

int QToolWidgets::index()
{
    return m_index;
}

void QToolWidgets::setExists(bool bExists)
{
    m_bLocalFile = bExists;
}

void QToolWidgets::onLoadFile()
{
    auto path = GET_CONFIG_DATA(Config::Data_Path).toString();
    if(path.isEmpty())
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QFileDialog dialog(
        this,
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

void QToolWidgets::onLeftPress()
{
#ifdef Q_OS_WIN
    if(isFullScreen())
        return;
    if(::ReleaseCapture()){
        ::SendMessage(HWND(this->winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
    }
#endif
}

void QToolWidgets::onMax()
{
    auto widget =/* parentWidget()*/this;
    if(!widget)
        widget = this;

    if(widget->isMaximized() || widget->isFullScreen())
    {
        widget->showNormal();
        setMaximumSize(qApp->desktop()->availableGeometry().size());
    }
    else
    {
        setMaximumSize(qApp->desktop()->availableGeometry().size());
        widget->showMaximized();
    }
}

void QToolWidgets::onFull()
{
    auto widget =/* parentWidget()*/this;
    if(!widget)
        widget = this;
    if(widget->isMaximized() || widget->isFullScreen())
    {
        widget->showNormal();
        setMaximumSize(qApp->desktop()->availableGeometry().size());
    }
    else
    {
//        setMaximumSize(qApp->desktop()->screenGeometry().size());
        widget->showFullScreen();
    }
}

void QToolWidgets::onSubtitle(const QString &str, int /*index*/)
{
    enum enum_sub_title
    {
        Sub_Titl_Time_Begin = 1,
        Sub_Titl_Time_End,
        Sub_Title_Type,
        Sub_Title_Content = 9
    };

    auto list = str.split(',');
    if(list.size() <= Sub_Title_Content)
    {
        return;
    }

    auto tmBeg = UTIL->getMs(list[Sub_Titl_Time_Begin]);
    auto tmEnd = UTIL->getMs(list[Sub_Titl_Time_End]);

    if(m_subtitle.tmBeg != tmBeg || m_subtitle.tmEnd != tmEnd)
    {
        m_subtitle.tmBeg = tmBeg;
        m_subtitle.tmEnd = tmEnd;
        m_subtitle.titls.clear();
    }

    QString content;
    for (int n = Sub_Title_Content; n < list.size(); ++n)
    {
        content.append(list[Sub_Title_Content]);
        content.append(",");
    }

    if(list.size() > Sub_Title_Content)
    {
        content.remove(content.length() - 1, 1);
    }

    if(content.lastIndexOf("\r\n") >= 0)
    {
        content = content.remove(content.length() - 2, 2);
    }

    m_subtitle.titls[list[Sub_Title_Type]] = content;
    m_subtitles[0]->setText(m_subtitle.titls.begin()->second);
    if(m_subtitle.titls.size() == 2)
    {
        m_subtitles[1]->setText(m_subtitle.titls.rbegin()->second);
    }
    else
    {
        m_subtitles[1]->clear();
    }
}

void QToolWidgets::onStreamInfo(const QStringList &list, int nChannel, int nDefault)
{
    auto actions = m_channelActions[nChannel];
    auto menu = m_channelMenus[nChannel];
    menu->clear();
    for(auto ac : actions->actions())
        actions->removeAction(ac);
    if(nDefault < 0)
        return;
    int n = 0;
    for(auto it : list)
    {
        auto ac = menu->addAction(it);
        ac->setCheckable(true);
        actions->addAction(ac);
        if(n == nDefault)
            ac->setChecked(true);
        ++n;
    }

    menu->addActions(actions->actions());
}

QWidget *QToolWidgets::CreateTitle(QWidget * parent)
{
    auto widget = new QWidget(parent);
    m_titleWd = widget;
    m_title = new QLabel(qApp->applicationName(), widget);
    m_min = new QPushButton(widget);
    m_max = new QPushButton(widget);
    m_close = new QPushButton(widget);

    widget->setObjectName("wd_title");
    m_title->setObjectName("label_title");
    m_min->setObjectName("btn_min");
    m_max->setObjectName("btn_max");
    m_close->setObjectName("btn_close");

    m_min->setToolTip(tr("minimize"));
    m_max->setToolTip(tr("maximize"));
    m_close->setToolTip(tr("close"));

    auto layout = new QHBoxLayout(widget);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(m_title);
    layout->addStretch();
    layout->addWidget(m_min);
    layout->addWidget(m_max);
    layout->addWidget(m_close);

    connect(m_min, &QPushButton::clicked, [this]
    {
        parentWidget()->showMinimized();
    });

    connect(m_max, &QPushButton::clicked, this, &QToolWidgets::onMax);
    connect(parentWidget(), SIGNAL(leftDoubleClicked()), this, SLOT(onFull()));
    connect(m_close, &QPushButton::clicked, this, &QToolWidgets::exit);
    connect(this, &QToolWidgets::hideOrShow, [widget, this](bool bHide)
    {
        m_title->setHidden(bHide);
        m_min->setHidden(bHide);
        m_max->setHidden(bHide);
        m_close->setHidden(bHide);
        widget->setHidden(bHide);
        if(bHide)
        {
            m_livePlatformWd->setHidden(bHide);
            if(cursor().shape() != Qt::BlankCursor)
                setCursor(Qt::BlankCursor);
        }
        else
        {
            if(cursor().shape() == Qt::BlankCursor)
                setCursor(Qt::ArrowCursor);
        }
    });

    connect(this, &QToolWidgets::play, [this](const QString& sUrl)
    {
        auto sName = sUrl.mid(sUrl.lastIndexOf('/') + 1);
        m_title->setToolTip(sName);
        auto font = m_title->fontMetrics();
        auto sText = font.elidedText(sName, Qt::ElideRight, m_title->width());
        m_title->setText(sText);

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
    m_process = new QProgressSlider(Qt::Orientation::Horizontal, parent, this);
    m_process->setObjectName("slider_pro");
    m_process->setValue(0);
    m_process->setDisabled(true);
    m_process->hide();
    auto layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_process);

    connect(this, &QToolWidgets::start, [this]{
        m_process->setEnabled(true);
    });

    connect(this, &QToolWidgets::stop, this, [this]{
        m_process->setValue(0);
        m_process->setDisabled(true);
        m_process->setVisible(false);
    }, Qt::QueuedConnection);

    connect(this,&QToolWidgets::setTotalSeconds, [this]{
        m_process->setEnabled(true);
        m_process->setVisible(false);
    });

    connect(this, &QToolWidgets::hideOrShow, [this](bool bHide){
        m_process->setVisible(m_bLocalFile && !bHide);
        if(!(m_bLocalFile && !bHide))
        {
            QTimer::singleShot(0, [this]{m_process->hide();});
        }
    });

    connect(this, &QToolWidgets::setTotalSeconds, [this](int nSeconds)
    {
        m_process->setRange(0, nSeconds);
    });
    connect(this, &QToolWidgets::setPosSeconds, [this](int nSeconds)
    {
        m_process->setPos(nSeconds);
    });

    connect(m_process, &QProgressSlider::gotoPos, this, &QToolWidgets::setSeekPos);
    connect(m_process, &QProgressSlider::getPreview, this, &QToolWidgets::getPreview);
    connect(this, &QToolWidgets::_preview, m_process, &QProgressSlider::onPreview);

    return layout;
}

QWidget *QToolWidgets::CreateToolbar(QWidget *parent)
{
    auto widget = new QWidget(parent);
    m_toolWd = widget;
    auto stop = new QPushButton(widget);
    auto prev = new QPushButton(widget);
    auto play = new QPushButton(widget);
    auto next = new QPushButton(widget);
    auto volMute = new QPushButton(widget);
    auto volNum = new QSlider(Qt::Orientation::Horizontal, widget);
    auto fileList = new QPushButton(widget);
    auto time = new QLabel(widget);
    time->setAlignment(Qt::AlignCenter);
    auto framRate = new QLabel(widget);

    widget->setObjectName("wd_toolbar");
    stop->setObjectName("btn_stop");
    prev->setObjectName("btn_prev");
    play->setObjectName("btn_pause");
    next->setObjectName("btn_next");
    volMute->setObjectName("btn_volume");
    volMute->setIcon(/*parent->*/style()->standardIcon(QStyle::SP_MediaVolume));
    volNum->setObjectName("slider_voice");
    fileList->setObjectName("file_list");
    fileList->setToolTip(tr("play list"));
    time->setObjectName("label_frame_time");
    framRate->setObjectName("label_frame_rate");

    auto space = CALC_WIDGET_WIDTH(nullptr, 10.0f / 1920);
    auto margin = CALC_WIDGET_WIDTH(nullptr, 15.0f / 1920);
    auto btnSize = CALC_WIDGET_SIZE(nullptr, 50.0f / 1920, 50.0f / 1080);
    stop->setFixedSize(btnSize / 5 * 2);
    prev->setFixedSize(btnSize);
    play->setFixedSize(btnSize);
    next->setFixedSize(btnSize);
    volMute->setFixedSize(btnSize / 2);
    CALC_WIDGET_WIDTH(volNum, 100.0f / 1920);
    fileList->setFixedSize(btnSize / 5 * 3);

    auto layout = new QHBoxLayout(widget);
    layout->setSpacing(space);
    layout->setMargin(margin);
    layout->addWidget(stop);
    layout->addWidget(prev);
    layout->addWidget(play);
    layout->addWidget(next);
    layout->addWidget(volMute);
    layout->addWidget(volNum);
    layout->addSpacing(10);
    layout->addWidget(time);
    layout->addStretch();
    layout->addWidget(framRate);
    layout->addSpacing(10);
    layout->addWidget(fileList);

    volNum->setRange(0, 100);
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

    connect(this, &QToolWidgets::frameRate, [framRate](int frameCount)
    {
        framRate->setText(QString("%1: %2/s").arg(tr("video")).arg(frameCount));
    });

    connect(this, &QToolWidgets::stop, this, [framRate]{ framRate->clear(); });
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

    auto funcUpdateTime = [this, time](int nSeconds)
    {
        if(m_totalSeconds > 100000000)
        {
            time->clear();
        }
        else
        {
            time->setText(QString("%1 / %2").arg(QTime::fromMSecsSinceStartOfDay(nSeconds).toString("HH:mm:ss"))
                          .arg(QTime::fromMSecsSinceStartOfDay(m_totalSeconds).toString("HH:mm:ss")));
        }
    };

    connect(this, &QToolWidgets::setTotalSeconds, [this, funcUpdateTime](int totals)
    {
        m_totalSeconds = totals;
        funcUpdateTime(0);
    });

    connect(this, &QToolWidgets::stop, this, [this, time]
    {
        m_totalSeconds = 0;
        time->clear();
    });

    connect(this, &QToolWidgets::setPosSeconds, funcUpdateTime);

    return widget;
}

QWidget *QToolWidgets::CreateLeftlist(QWidget *parent)
{
    m_livePlatformWd = new QWidget(parent);
    m_livePlatformWd->setObjectName("live_platform");
    auto close = new QPushButton(m_livePlatformWd);
    close->setObjectName("btn_close");
    m_platformManager = new QLivePlatformManager(m_livePlatformWd);
    auto group = m_platformManager->group();

    auto layout = new QVBoxLayout(m_livePlatformWd);
    auto layoutBtn = new QVBoxLayout;
    layout->addWidget(close, 0, Qt::AlignRight | Qt::AlignTop);
    layout->addStretch();
    layout->addLayout(layoutBtn);
    layout->addStretch();

    layoutBtn->setAlignment(Qt::AlignCenter);
    for(auto it : group->buttons())
        layoutBtn->addWidget(it);

    m_livePlatformWd->hide();
    connect(close, &QPushButton::clicked, m_livePlatformWd, &QWidget::hide);
    connect(group, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), m_livePlatformWd, &QWidget::hide);

    m_douyu = new QDouyuWidget(parent);
    connect(m_douyu, &QDouyuWidget::play, this, &QToolWidgets::play);
    connect(group, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), m_douyu, &QDouyuWidget::showIndex);

    return m_livePlatformWd;
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

    auto layout0 = new QVBoxLayout(m_filelistWd);
    auto layoutR = new QGridLayout(right);

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

QWidget *QToolWidgets::CreateSubTitle(QWidget *parent)
{
    auto widget = new QWidget(parent);
    m_subtitleWd = widget;
    auto ch = new QLabel(widget);
    auto other = new QLabel(widget);
    m_subtitles.push_back(ch);
    m_subtitles.push_back(other);
    ch->setAlignment(Qt::AlignCenter);
    other->setAlignment(Qt::AlignCenter);

    widget->setObjectName("sub_title_wd");
    ch->setObjectName("sub_title_ch");
    other->setObjectName("sub_title_other");

    auto layout = new QVBoxLayout(widget);
    layout->setAlignment(Qt::AlignCenter);
    layout->setMargin(0);
    layout->addWidget(ch);
    layout->addWidget(other);
    layout->addSpacing(CALC_WIDGET_HEIGHT(nullptr, 20.0f / 1920));
    connect(this, &QToolWidgets::setPosSeconds, this, [this](int pos)
    {
        bool bVisible = (m_subtitle.tmBeg < pos && m_subtitle.tmEnd > pos);
        for (auto it : m_subtitles)
        {
            it->setVisible(bVisible);
        }
    });

    return widget;
}

void QToolWidgets::CreateMenu(QWidget *parent)
{
    auto menu = new QMenu(parent);
    auto actionAdjust = menu->addAction(tr("adjust"));
    auto topWindow = menu->addAction(tr("topWindow"));
    auto urlWindow = menu->addAction(tr("url"));

    auto menuRender = new QMenu(tr("render"), parent);
    auto group = new QActionGroup(parent);
    auto actionOpengl = group->addAction(tr("opengl"));
    actionOpengl->setCheckable(true);
    auto actionDX11 = group->addAction(tr("dx11"));
    actionDX11->setCheckable(true);
    menuRender->addActions(group->actions());

    auto menuChannel = new QMenu(tr("channel"), parent);
    auto menuChannelVideo = new QMenu(tr("video"), parent);
    auto menuChannelAudio = new QMenu(tr("audio"), parent);
    auto menuChannelSubtitle = new QMenu(tr("subtitle"), parent);

    m_channelMenus.push_back(menuChannelVideo);
    m_channelMenus.push_back(menuChannelAudio);
    m_channelMenus.push_back(menuChannelSubtitle);
    for (int n = 0; n < channel_max; ++n) {
        auto channelGroup = new QActionGroup(parent);
        m_channelActions.push_back(channelGroup);
        connect(channelGroup, &QActionGroup::triggered, [this, n, channelGroup](QAction *ac)
        {
            auto index = channelGroup->actions().indexOf(ac);
            emit activeChannel(n, index);
        });
    }

    menuChannel->addMenu(menuChannelVideo);
    menuChannel->addMenu(menuChannelAudio);
    menuChannel->addMenu(menuChannelSubtitle);

    menu->addSeparator();
    menu->addMenu(menuRender);
    menu->addMenu(menuChannel);

    actionAdjust->setCheckable(true);
    topWindow->setCheckable(true);

    menu->setObjectName("menu1");
    menuRender->setObjectName("menu1");
    menuChannel->setObjectName("menu1");
    menuChannelVideo->setObjectName("menu1");
    menuChannelAudio->setObjectName("menu1");
    menuChannelSubtitle->setObjectName("menu1");
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

    connect(urlWindow, &QAction::triggered, m_inputUrl, &QInputUrlWidget::showInit);
    connect(m_inputUrl, &QInputUrlWidget::inputUrl, [this](const QString& url)
    {
        if(m_bPlaying)
            emit stop();

        emit play(url);
    });

    connect(group, &QActionGroup::triggered, [group, this](QAction *action)
    {
        for (auto ac : group->actions())
        {
            if(!ac->text().contains(m_curRender) && action == ac)
            {
                ac->setText(ac->text() + tr("(restart valid)"));
            }
            else
            {
                ac->setText(ac->text().replace(tr("(restart valid)"), ""));
            }
        }

        action->setChecked(true);
        SET_CONFIG_DATA(action->text().replace(tr("(restart valid)"), ""), Config::Data_Render);
    });

    connect(Config::instance(), &Config::loadConfig, [this, group]
    {
        m_curRender = GET_CONFIG_DATA(Config::Data_Render).toString();
        for (auto ac : group->actions())
        {
            if(ac->text().contains(m_curRender))
            {
                ac->setChecked(true);
                break;
            }
        }
    });
}

void QToolWidgets::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if(event->buttons() & Qt::RightButton)
        emit showMenu();
    if(event->button() == Qt::LeftButton)
    {
#ifdef Q_OS_WIN
        onLeftPress();
#endif
    }
}

void QToolWidgets::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);
    if(event->button() == Qt::LeftButton)
    {
        onFull();
    }
}

void QToolWidgets::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    emit _move(event->pos());
}

bool QToolWidgets::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
#ifdef Q_OS_WIN
    auto msg = reinterpret_cast<MSG*>(message);
//    auto widget = QWidget::find((int)msg->hwnd);
//    if(msg->hwnd == reinterpret_cast<HWND>(m_inputUrl->winId()))
//        qDebug() << __FUNCTION__ << widget << msg->message;
    if(msg->hwnd != reinterpret_cast<HWND>(this->winId()))
    {
        // resize box
        // 因为设置了WA_TranslucentBackground，不能捕获到鼠标的消息（用于缩放），需要转发给自己
        // 如果不设置WA_TranslucentBackground，背景无法穿透，不能看到视频界面
        if((isActiveWindow() || (parentWidget() && parentWidget()->isActiveWindow())) && (msg->message == WM_NCHITTEST || msg->message == WM_NCLBUTTONDOWN))
        {
            msg->hwnd = reinterpret_cast<HWND>(winId());
        }
        else
        {
            return false;
        }
    }

    if(_nativeEvent(eventType, message, result, this))
        return true;
#endif
    return false;
}

void QToolWidgets::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    UTIL->setWindowEllispeFrame(this, 0, 0);
    emit _resize(event->size());
    m_subtitleWd->setFixedWidth(width());
    m_subtitleWd->move(0, height() - m_subtitleWd->height());
}

void QToolWidgets::keyPressEvent(QKeyEvent *event)
{
    qDebug() << __FUNCTION__ << event->key();
    switch (event->key()) {
    case Qt::Key_Escape:
        if(isFullScreen())
        {
            showNormal();
        }
        break;
    case Qt::Key_Left:
    {
        auto value = m_process->value();
        value -= 5000;
        if(value >= 0)
        {
            m_process->setValue(value);
            emit m_process->gotoPos(value);
        }
    }break;
    case Qt::Key_Right:
    {
        auto value = m_process->value();
        value += 5000;
        if(value < m_process->maximum())
        {
            m_process->setValue(value);
            emit m_process->gotoPos(value);
        }
    }break;
    case Qt::Key_Up:
        break;
    case Qt::Key_Down:
        break;
    default:
        break;
    }
}
