#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

#include <QSystemTrayIcon>

namespace Ui {
class Dialog;
}

class QAction;
class QMenu;
class QTimer;

class Dialog : public QDialog
{
    Q_OBJECT

public:
    enum State { Inactive, NoInternet, Internet };

    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    void createTrayIcon();
    void setState( Dialog::State state );

private slots:
    void start(QSystemTrayIcon::ActivationReason reason = QSystemTrayIcon::DoubleClick);
    void stop(bool success = false);
    void internetTest();

private:
    Ui::Dialog *ui;

    State mState;

    QTimer *mTimer;
    QAction *mQuitAction, *mRestoreAction;
    QSystemTrayIcon *mTrayIcon;
    QMenu *mTrayIconMenu;
};

#endif // DIALOG_H
