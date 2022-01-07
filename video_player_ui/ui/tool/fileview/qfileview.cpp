#include "qfileview.h"
#include <QLineEdit>
#include <QPushButton>
#include <QButtonGroup>
#include <QBoxLayout>
#include <QApplication>
#include "playlist/qplayfilelistmodel.h"
#include "playlist/qfilelistview.h"
#include "ui/qtoolwidgets.h"
#include "ui/tool/title/qplaytitle.h"
#include "control/videocontrol.h"
#include "config/config.h"

QFileView::QFileView(QWidget *parent)
    : QToolBase(parent)
    , m_bHandleStop(false)
{
    initUi();
    initLayout();
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
    localModel->setObjectName("btn_mode");
    liveModel->setObjectName("btn_mode");

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
    auto control = VIDEO_CONTROL;

    connect(m_searchEdit, &QLineEdit::textChanged, m_filelist, &QFileListView::filter);

    connect(m_group, SIGNAL(buttonClicked(int)), model, SLOT(setMode(int)));
    connect(this, &QFileView::play, model, &QPlayFileListModel::play);

    connect(m_filelist, &QFileListView::loadFile, toolWidget, &QToolWidgets::onLoadFile);
    connect(toolWidget, &QToolWidgets::load, m_filelist, &QFileListView::addLocalUrl);
    connect(toolWidget, &QToolWidgets::load, this, &QFileView::play);
    connect(toolWidget, &QToolWidgets::inputUrlFile, m_filelist, &QFileListView::inputUrlFile);

    connect(m_filelist, &QFileListView::select, this, &QFileView::onHandleStop);
    connect(m_filelist, &QFileListView::select, this, &QFileView::play);
    connect(this, &QFileView::play, control, &QVideoControl::onStart);

    connect(control, &QVideoControl::play, model, &QPlayFileListModel::play);
    connect(control, &QVideoControl::end, this, &QFileView::onEnd);
}

void QFileView::onAutoShow()
{
    setVisible(!isVisible());
}

void QFileView::onAutoVisable(bool bHide)
{
    if(bHide)
    {
        setVisible(false);
    }
}

void QFileView::onPrev()
{
    playStep(-1);
}

void QFileView::onNext()
{
    playStep(1);
}

void QFileView::onEnd()
{
    auto bLoopPlay = GET_CONFIG_DATA(Config::Data_LoopPlay).toBool();
    if(!m_bHandleStop && bLoopPlay)
    {
        onNext();
    }
    else
    {
        m_filelist->onEnd();
        m_bHandleStop = false;
    }
}

void QFileView::onHandleStop()
{
    m_bHandleStop = true;
}

void QFileView::playStep(int nStep)
{
    auto index = m_filelist->currentIndex();
    if(!index.isValid())
        index = m_filelist->model()->index(0, 0);
    else
    {
        auto m = m_filelist->model();
        auto rows = m->rowCount();
        int row = index.row() + nStep;
        if(row < 0) row = rows - 1;
        else if(row >= rows) row = 0;

        index = m->index(row, 0);
    }
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

