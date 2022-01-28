#include "qpagewidget.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPushButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QIntValidator>
#ifndef STR
#define STR(x) QString::fromLocal8Bit(x)
#endif // STR

QPageWidget::QPageWidget(QWidget *parent)
    : QWidget(parent)
    , m_nCurrentPage(0)
    , m_nPageCount(0)
    , m_nStartPage(0)
    , m_perPage(10)
{
//    setObjectName("page_widget");
    auto label0 = new QLabel(tr("jump to"), this);
    label0->setObjectName("page_label");
    m_jump = new QLineEdit(this);
    m_jump->setObjectName("jump_edit");
    setNum(m_jump, 1, 1);
    m_jump->setAlignment(Qt::AlignCenter);
    m_jump->setContextMenuPolicy(Qt::NoContextMenu);
    connect(m_jump, SIGNAL(returnPressed()), this, SLOT(onGoPage()));
    auto label1 = new QLabel(tr("page"), this);
    label1->setObjectName("page_label");
    QPushButton* go = new QPushButton(tr("jump"), this);
    connect(go, SIGNAL(clicked()), this, SLOT(onGoPage()));
    go->setObjectName("jump_button");

    m_pgCount = new QLabel(this);
    m_pgCount->setObjectName("page_label");
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setAlignment(Qt::AlignRight);
    m_pglayout = new QHBoxLayout;
    setLayout(layout);

    QPushButton* prev = new QPushButton(STR("<"), this);
    QPushButton* next = new QPushButton(STR(">"), this);
    prev->setObjectName("jump_button");
    next->setObjectName("jump_button");
    connect(prev, SIGNAL(clicked()), this, SLOT(onPrev()));
    connect(next, SIGNAL(clicked()), this, SLOT(onNext()));
    m_group = new QButtonGroup(this);
    auto count = static_cast<int>(sizeof(m_index) / sizeof(m_index[0]));
    for (int n = 0; n < count; ++n)
    {
        m_index[n] = new QPushButton(QString("%1").arg(n + 1), this);
        m_index[n]->setObjectName("page_button");
        m_index[n]->setCheckable(true);
        m_group->addButton(m_index[n], n);
        if (n > 0)
            m_index[n]->hide();
        Addpage(n);
    }

    m_nMaxShowPage = sizeof(m_index) / sizeof(m_index[0]);
    connect(m_group, SIGNAL(buttonClicked(int)), this, SLOT(onPage(int)));
    m_pg = new QLabel(tr("Page 1 / 1") + ",", this);
    m_pg->setObjectName("page_label");
    m_index[0]->setChecked(true);

    layout->addStretch();
    layout->addWidget(prev);
    layout->addLayout(m_pglayout);
    layout->addWidget(next);
    layout->addWidget(m_pg);
    layout->addWidget(label0, 0, Qt::AlignRight);
    layout->addWidget(m_jump, 0, Qt::AlignRight);
    layout->addWidget(label1, 0, Qt::AlignRight);
    layout->addWidget(go, 0, Qt::AlignRight);
    layout->addWidget(m_pgCount, 0, Qt::AlignRight);
}

QPageWidget::~QPageWidget()
{
}

void QPageWidget::setNum(QLineEdit* edit, int min, int max)
{
    edit->setValidator(new QIntValidator(min, max, edit));
    edit->setAttribute(Qt::WA_InputMethodEnabled, false);
}

void QPageWidget::Addpage(int page)
{
    m_pglayout->addWidget(m_index[page],0, Qt::AlignCenter);
}

void QPageWidget::setTotalCount(int count)
{
    m_pgCount->setText(tr("current %1 uints").arg(0));
    resetPage(count, 0, 0, true);
}

void QPageWidget::setPageCount(int count)
{
    m_pgCount->setText(tr("current %1 uints").arg(count));
}

void QPageWidget::resetPage(int pageCount, int currentPage, int startPage, bool bReselect)
{
    if (bReselect)
    {
        m_jump->clear();
        m_nStartPage = startPage;
        m_nCurrentPage = currentPage;
        m_index[0]->setChecked(true);
    }

    m_nPageCount = pageCount;
    if (m_nPageCount <= 0) m_nPageCount = 1;
    setNum(m_jump, 1, m_nPageCount);
    for (int n = 0; n < m_nMaxShowPage; ++n)
    {
        m_index[n]->setText(QString("%1").arg(startPage + n + 1));
        if (m_nPageCount > startPage + n)
            m_index[n]->show();
        else
            m_index[n]->hide();
    }

    if (!m_nPageCount)m_nPageCount = 1;
    m_pg->setText(tr("Page %1 / %2").arg(currentPage + 1).arg(m_nPageCount) + ",");
    m_nCurrentPage = currentPage;
}

void QPageWidget::setPerPageCount(int ncount)
{
    m_perPage = ncount;
}

int QPageWidget::curentPage()
{
    return m_nCurrentPage + 1;
}

int QPageWidget::startPage()
{
    return m_nStartPage;
}

int QPageWidget::perPage()
{
    return m_perPage;
}

void QPageWidget::onPage(int nPage)
{
    m_nCurrentPage = m_nStartPage + nPage;
    if (!m_nPageCount)m_nPageCount = 1;
    m_pg->setText(tr("Page %1 / %2").arg(m_nCurrentPage + 1).arg(m_nPageCount) + ",");
    emit pageChanged(m_nCurrentPage + 1);
}

void QPageWidget::onPrev()
{
    if (m_nCurrentPage <= 0) return;
    --m_nCurrentPage;

    if (m_index[0]->isChecked())
    {
        m_nStartPage = m_nCurrentPage - m_nMaxShowPage + 1;
        if (m_nStartPage <= 0)
        {
            m_nStartPage = 0;
            m_index[m_nCurrentPage]->setChecked(true);
        }
        else
            m_index[m_nMaxShowPage - 1]->setChecked(true);
    }else
        m_index[m_nCurrentPage - m_nStartPage]->setChecked(true);

    resetPage(m_nPageCount, m_nCurrentPage, m_nStartPage);
    emit pageChanged(m_nCurrentPage + 1);
}

void QPageWidget::onNext()
{
    if (m_nCurrentPage >= m_nPageCount - 1) return;
    ++m_nCurrentPage;

    if (m_index[m_nMaxShowPage - 1]->isChecked())
    {
        m_nStartPage = m_nCurrentPage;
        if (m_nStartPage + m_nMaxShowPage > m_nPageCount)
        {
            m_nStartPage = m_nPageCount - m_nMaxShowPage;
            m_index[m_nCurrentPage - m_nStartPage]->setChecked(true);
        }
        else
            m_index[0]->setChecked(true);
    }else
        m_index[m_nCurrentPage - m_nStartPage]->setChecked(true);

    resetPage(m_nPageCount, m_nCurrentPage, m_nStartPage);
    emit pageChanged(m_nCurrentPage + 1);
}

void QPageWidget::onGoPage()
{
    int page = m_jump->text().toInt();
    if (!page) return;
    m_nCurrentPage = m_jump->text().toInt() - 1;
    m_nStartPage = m_nCurrentPage / m_nMaxShowPage * m_nMaxShowPage;
    if (m_nStartPage + m_nMaxShowPage > m_nPageCount)
        m_nStartPage = m_nPageCount - m_nMaxShowPage;
    if (m_nStartPage < 0) m_nStartPage = 0;
    m_index[m_nCurrentPage - m_nStartPage]->setChecked(true);
    resetPage(m_nPageCount, m_nCurrentPage, m_nStartPage);
    emit pageChanged(m_nCurrentPage + 1);
}
