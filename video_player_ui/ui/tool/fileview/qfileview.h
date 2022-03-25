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

    QString title(const QString& sUrl);
signals:
    void loadFile();
    void play(const QString&);
    void thumbPlayOrPause();
    void thumbStop();
public slots:
    void onAutoShow();
    void onAutoVisable(bool) override;
    void onLoad(const QStringList&);
    void onPrev();
    void onNext();
    void onAutoNext();
    void onExceptionEnd();
    void onEnd();
    void onHandleStop();
#ifdef Q_OS_WIN
    void onThumb(int);
    void onCmd(int, const QString&);
#endif
private:
    void playStep(int nStep);
    void playRandom();

private:
    void initUi();
    void initLayout();
    void initConnect() override;

private:
    QFileListView* m_filelist;
    QLineEdit* m_searchEdit;
    QButtonGroup* m_group;
    bool m_bExceptionStop;
    bool m_bHandleStop;
};

#endif // QFILEVIEW_H
