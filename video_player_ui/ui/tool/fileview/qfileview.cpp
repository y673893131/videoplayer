#include "qfileview.h"
#include <QLineEdit>
#include <QPushButton>
#include <QButtonGroup>
#include <QBoxLayout>
#include <QApplication>
#include <QTime>
#include <QFile>
#include "playlist/qplayfilelistmodel.h"
#include "playlist/qfilelistview.h"
#include "ui/qtoolwidgets.h"
#include "ui/tool/title/qplaytitle.h"
#include "ui/tool/play_control/qplaycontrol.h"
#include "ui/tool/menu/qplaymenu.h"
#include "ui/tool/output/qoutputwidget.h"
#include "control/videocontrol.h"
#include "config/config.h"


#ifdef Q_OS_WIN
#include "ui/thumb/qwinthumbnail.h"
#endif

QFileView::QFileView(QWidget *parent)
    : QToolBase(parent)
    , m_bExceptionStop(false)
    , m_bHandleStop(false)
{
    initUi();
    initLayout();
}

QString QFileView::title(const QString &sUrl)
{
    auto model = qobject_cast<QPlayFileListModel*>(m_filelist->model());
    return model->title(sUrl);
}

void QFileView::initUi()
{
    hide();
    m_filelist = new QFileListView(this);
    m_searchEdit = new QLineEdit(this);
    auto localModel = new QPushButton(tr("localMode"), m_filelist);
    auto liveModel = new QPushButton(tr("liveMode"), m_filelist);

    setObjectName("file_list_wd");
    m_searchEdit->setObjectName("file_search_edit");
    m_searchEdit->setPlaceholderText(tr("search"));
    m_filelist->setObjectName("list_file");
    localModel->setObjectName("btn_file_mode");
    liveModel->setObjectName("btn_file_mode");

    m_group = new QButtonGroup(this);
    m_group->addButton(localModel, QPlayFileListModel::play_mode_local);
    m_group->addButton(liveModel, QPlayFileListModel::play_mode_live);
    localModel->setCheckable(true);
    liveModel->setCheckable(true);
    localModel->setChecked(true);
}

void QFileView::initLayout()
{
    auto layout = new QGridLayout(this);

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_searchEdit, 0, 0, 1, 2);
    layout->addWidget(m_filelist, 1, 0, 1, 2);

    layout->addWidget(m_group->button(QPlayFileListModel::play_mode_local), 2, 0);
    layout->addWidget(m_group->button(QPlayFileListModel::play_mode_live), 2, 1);
}

void QFileView::initConnect()
{
    auto model = qobject_cast<QPlayFileListModel*>(m_filelist->model());
    auto toolWidget = qobject_cast<QToolWidgets*>(m_parent);
    auto playControl = m_parent->findChild<QPlayControl*>();
    auto control = VIDEO_CONTROL;

    connect(m_searchEdit, &QLineEdit::textChanged, m_filelist, &QFileListView::filter);

    connect(m_group, SIGNAL(buttonClicked(int)), model, SLOT(setMode(int)));
    connect(this, &QFileView::play, model, &QPlayFileListModel::play);

    connect(m_filelist, &QFileListView::loadFile, toolWidget, &QToolWidgets::onLoadFile);
    connect(toolWidget, &QToolWidgets::load, this, &QFileView::onLoad);
    connect(toolWidget, &QToolWidgets::load, m_filelist, &QFileListView::addLocalUrl);
    connect(toolWidget, &QToolWidgets::inputUrlFile, m_filelist, &QFileListView::inputUrlFile);
#ifdef Q_OS_WIN
    connect(toolWidget, &QToolWidgets::thumb, this, &QFileView::onThumb);
    connect(toolWidget, &QToolWidgets::cmd, this, &QFileView::onCmd);
#endif
    connect(m_filelist, &QFileListView::select, this, &QFileView::onHandleStop);
    connect(m_filelist, &QFileListView::select, this, &QFileView::play);
    connect(this, &QFileView::play, control, &QVideoControl::onStart);

    connect(control, &QVideoControl::play, model, &QPlayFileListModel::play);
    connect(control, &QVideoControl::end, this, &QFileView::onEnd);
    connect(control, &QVideoControl::notExist, this, &QFileView::onEnd);
    connect(control, &QVideoControl::exceptionEnd, this, &QFileView::onExceptionEnd);

    connect(this, &QFileView::thumbPlayOrPause, playControl, &QPlayControl::onPlayOrPause);
    connect(this, &QFileView::thumbStop, control, &QVideoControl::onStoped);
}

void QFileView::onAutoShow()
{
    setVisible(!isVisible());
}

void QFileView::onAutoVisable(bool bHide)
{
    if(underMouse())
        QToolBase::onAutoVisable(bHide);
    else if(bHide)
    {
        setVisible(false);
    }
}

void QFileView::onLoad(const QStringList & list)
{
    onHandleStop();
    emit play(list.at(0));
}

void QFileView::onPrev()
{
    if(GET_CONFIG_DATA(Config::Data_PlayMode).toInt() == QPlayMenu::play_mode_random)
        playRandom();
    else
        playStep(-1);
}

void QFileView::onNext()
{
    if(GET_CONFIG_DATA(Config::Data_PlayMode).toInt() == QPlayMenu::play_mode_random)
        playRandom();
    else
        playStep(1);
}

void QFileView::onAutoNext()
{
    auto playMode = GET_CONFIG_DATA(Config::Data_PlayMode).toInt();
    switch (playMode) {
    case QPlayMenu::play_mode_loop:
        playStep(1);
        break;
    case QPlayMenu::play_mode_single:
        playStep(0);
        break;
    case QPlayMenu::play_mode_random:
        playRandom();
        break;
    default:
        break;
    }
}

void QFileView::onExceptionEnd()
{
    m_bExceptionStop = true;
}

void QFileView::onEnd()
{
    if(m_bHandleStop || m_bExceptionStop)
    {
        m_filelist->onEnd();
        m_bHandleStop = false;
        m_bExceptionStop = false;
        return;
    }

    onAutoNext();
}

void QFileView::onHandleStop()
{
    m_bHandleStop = true;
    QTimer::singleShot(300, this, [=]{ m_bHandleStop = false; });
}

#ifdef Q_OS_WIN
void QFileView::onThumb(int type)
{
    switch (type) {
    case thumb_prev:
        onHandleStop();
        onPrev();
        break;
    case thumb_play_or_pause:
        emit thumbPlayOrPause();
        break;
    case thumb_next:
        onHandleStop();
        onNext();
        break;
    }
}

void QFileView::onCmd(int type, const QString & args)
{
    switch (type) {
    case cmd_type_prev:
        onHandleStop();
        onPrev();
        break;
    case cmd_type_next:
        onHandleStop();
        onNext();
        break;
    case cmd_type_play:
    {
        auto model = qobject_cast<QPlayFileListModel*>(m_filelist->model());
        auto index = model->findIndex(args);
        if(index.isValid())
        {
            m_filelist->setCurrentIndex(index);
            emit m_filelist->select(args);
        }
        else
        {
            if(QFile::exists(args))
            {
                emit m_filelist->addLocalUrl(QStringList(args));
                auto index = model->findIndex(args);
                m_filelist->setCurrentIndex(index);
                emit m_filelist->select(args);
            }
            else
            {
                auto output = m_parent->findChild<QOutputWidget*>();
                output->onError(QString("%1 not exist.").arg(args));
            }
        }
    }break;
    case cmd_type_stop:
        onHandleStop();
        emit thumbStop();
        break;
    default:
        break;
    }

}
#endif

void QFileView::playStep(int nStep)
{
    auto index = m_filelist->currentIndex();
    auto m = m_filelist->model();
    if(!index.isValid())
    {
        if(nStep < 0)
            index = m->index(0, 0);
    }

    auto rows = m->rowCount();
    int row = index.row() + nStep;
    if(row < 0) row = rows - 1;
    else if(row >= rows) row = 0;
    index = m->index(row, 0);

    if(!index.isValid())
    {
        emit loadFile();
        return;
    }
    else
    {
        m_filelist->setCurrentIndex(index);
        m_filelist->update();
        auto url = index.data(QPlayFileListModel::role_url).toString();
        emit play(url);
    }
}

void QFileView::playRandom()
{
    auto model = qobject_cast<QPlayFileListModel*>(m_filelist->model());
    auto rowCount = model->rowCount();
    auto time = QTime::currentTime();
    auto sr = static_cast<uint>(time.msec());
    qsrand(sr);
    auto row = qrand() % rowCount;

    auto cur = m_filelist->currentIndex().row();
    row = row == cur ? 1 : row - cur;
    playStep(row);
}

