#ifndef QFILEVIEW_H
#define QFILEVIEW_H

#include "../base/qtoolbase.h"

class QFileListView;
class QLineEdit;
class QButtonGroup;
class QFileView : public QToolBase
{
    Q_OBJECT
public:
    explicit QFileView(QWidget *parent = nullptr);

signals:
    void loadFile();
    void play(const QString&);
public slots:
    void onAutoShow();
    void onAutoVisable(bool) override;
    void onPrev();
    void onNext();
    void onEnd();
    void onHandleStop();
private:
    void playStep(int nStep);

private:
    void initUi();
    void initLayout();
    void initConnect() override;

private:
    QFileListView* m_filelist;
    QLineEdit* m_searchEdit;
    QButtonGroup* m_group;
    bool m_bHandleStop;
};

#endif // QFILEVIEW_H
