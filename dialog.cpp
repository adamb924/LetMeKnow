#include "dialog.h"
#include "ui_dialog.h"

#include <QMenu>
#include <QTimer>
#include <QProcess>
#include <QSound>

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

    mTrayIcon->show();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::createTrayIcon()
{
    mQuitAction = new QAction(tr("&Quit"), this);
    connect(mQuitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    mRestoreAction = new QAction(tr("&Restore"), this);
    connect(mRestoreAction, &QAction::triggered, this, &QWidget::showNormal);

    mTrayIconMenu = new QMenu(this);
    mTrayIconMenu->addAction(mRestoreAction);
    mTrayIconMenu->addAction(mQuitAction);

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
        break;
    case Dialog::NoInternet:
        icon = QIcon(":/images/red.png");
        break;
    case Dialog::Inactive:
    default:
        icon = QIcon(":/images/blue.png");
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

void Dialog::stop(bool success)
{
    if( success )
    {
        /// From: http://www.fromtexttospeech.com/
        QSound::play(":/sounds/theinternetisback.wav");
        setState( Dialog::Internet );
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
