#include "dialog.h"
#include "ui_dialog.h"

#include <QMenu>
#include <QTimer>
#include <QProcess>
#include <QSound>
#include <QSettings>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{    
    ui->setupUi(this);

    createTrayIcon();

    mTimer = new QTimer(this);
    mTimer->setSingleShot(false);
    connect( mTimer, SIGNAL(timeout()), this, SLOT(internetTest()) );

    connect( ui->startButton, SIGNAL(clicked(bool)), this, SLOT(start()) );
    connect( ui->stopButton, SIGNAL(clicked(bool)), this, SLOT(stop()) );

    setState(Dialog::Inactive);

    QSettings settings("adambaker.org", "LetMeKnow");
    ui->siteEdit->setText( settings.value("siteEdit","google.com").toString() );
    ui->intervalBox->setValue( settings.value("intervalBox", 10).toInt() );
    ui->soundBox->setChecked( settings.value("soundBox", true).toBool() );
    ui->notificationBox->setChecked( settings.value("notificationBox", true).toBool() );

    mTrayIcon->show();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    QSettings settings("adambaker.org", "LetMeKnow");
    settings.setValue("siteEdit", ui->siteEdit->text() );
    settings.setValue("intervalBox", ui->intervalBox->text() );
    settings.setValue("soundBox",ui->soundBox->isChecked() );
    settings.setValue("notificationBox",ui->notificationBox->isChecked() );
}

void Dialog::createTrayIcon()
{
    mTrayIconMenu = new QMenu(this);

    QAction *letMeKnow = new QAction(tr("&Let me know when the internet is back"), this);
    // http://stackoverflow.com/questions/26034241/make-a-qmenu-label-bold-without-affecting-its-children
    QFont boldFont = mTrayIconMenu->menuAction()->font();
    boldFont.setBold(true);
    letMeKnow->setFont(boldFont);
    connect(letMeKnow, SIGNAL(triggered(bool)), this, SLOT(start(bool)) );

    QAction *restoreAction = new QAction(tr("&Settings..."), this);
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    QAction *quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    mTrayIconMenu->addAction(letMeKnow);
    mTrayIconMenu->addAction(restoreAction);
    mTrayIconMenu->addAction(quitAction);

    mTrayIcon = new QSystemTrayIcon(this);
    mTrayIcon->setContextMenu(mTrayIconMenu);

    connect( mTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(start(QSystemTrayIcon::ActivationReason)) );
}

void Dialog::setState(Dialog::State state)
{
    mState = state;

    QIcon icon;

    switch(mState)
    {
    case Dialog::Internet:
        icon = QIcon(":/images/green.png");
        mTrayIcon->setToolTip(tr("There is internet."));
        break;
    case Dialog::NoInternet:
        icon = QIcon(":/images/red.png");
        mTrayIcon->setToolTip(tr("There is no internet."));
        break;
    case Dialog::Inactive:
    default:
        icon = QIcon(":/images/blue.png");
        mTrayIcon->setToolTip(tr("We haven't checked for internet recently."));
        break;
    }

    mTrayIcon->setIcon( icon );
    setWindowIcon(icon);

}

void Dialog::start(QSystemTrayIcon::ActivationReason reason)
{
    if( reason == QSystemTrayIcon::DoubleClick )
    {
        setState( Dialog::NoInternet );
        mTimer->start( ui->intervalBox->value() * 1000 );

        ui->startButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
    }
}

void Dialog::start(bool checked)
{
    Q_UNUSED(checked);
    start(QSystemTrayIcon::DoubleClick);
}

void Dialog::stop(bool success)
{
    if( success )
    {
        setState( Dialog::Internet );
        if( ui->soundBox->isChecked() )
        {
            /// Sound from: http://www.fromtexttospeech.com/
            QSound::play(":/sounds/theinternetisback.wav");
        }
        if( ui->notificationBox->isChecked() )
        {
            mTrayIcon->showMessage(tr("Internet Available"), tr("We were able to get ahold of %1, so at least there is something.").arg(ui->siteEdit->text()) );
        }
    }
    else
    {
        setState( Dialog::Inactive );
    }
    mTimer->stop();

    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
}

void Dialog::internetTest()
{
    /// From: http://stackoverflow.com/questions/22935103/get-the-ping-from-a-remote-target-with-qt-windows-linux

    QStringList parameters;

    #if defined(WIN32)
       parameters << "-n" << "1";
    #else
       parameters << "-c 1";
    #endif

    parameters << ui->siteEdit->text();

    int exitCode = QProcess::execute("ping", parameters );
    if (exitCode==0) {
        stop(true);
    }
}
