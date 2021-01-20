#ifndef QPAGEWIDGET_H
#define QPAGEWIDGET_H

#include <QWidget>
class QPushButton;
class QHBoxLayout;
class QButtonGroup;
class QLabel;
class QLineEdit;
class QPageWidget : public QWidget
{
    Q_OBJECT

public:
    QPageWidget(QWidget *parent);
    ~QPageWidget();

    void resetPage(int pageCount, int currentPage=0, int startPage=0,bool bReselect=false);
    void setPerPageCount(int);
    int curentPage();
    int startPage();
    int perPage();
signals:
    void pageChanged(int);
public slots:
    void setTotalCount(int count);
    void setPageCount(int count);
private slots:
    void onPage(int);
    void onPrev();
    void onNext();
    void onGoPage();
private:
    void Addpage(int page);
    void setNum(QLineEdit* edit, int min, int max);
private:
    QPushButton* m_index[5];
    QHBoxLayout* m_pglayout;
    QButtonGroup* m_group;
    QLabel* m_pg;
    QLabel* m_pgCount;
    QLineEdit* m_jump;
    int m_nCurrentPage, m_nPageCount, m_nStartPage,m_nMaxShowPage,m_perPage;
};

#endif // QPAGEWIDGET_H
