#ifndef QFILELISTMENU_H
#define QFILELISTMENU_H

#include "qplaymenubase.h"

class QFileListMenu : public QPlayMenuBase
{
    Q_OBJECT
public:
    enum action
    {
        action_open_dir,
        action_load_file,
        action_clean_file_lsit,
        action_delete
    };

public:
    explicit QFileListMenu(QWidget *parent = nullptr);
};

#endif // QFILELISTMENU_H
