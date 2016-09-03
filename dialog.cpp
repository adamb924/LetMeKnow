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
    mTrayIconMenu = new QMenu(this);

    QAction *letMeKnow = new QAction(tr("&Let me know when the internet is back"), this);
    // http://stackoverflow.com/questions/26034241/make-a-qmenu-label-bold-without-affecting-its-children
    QFont boldFont = mTrayIconMenu->menuAction()->font();
    boldFont.setBold(true);
    letMeKnow->setFont(boldFont);
    connect(letMeKnow, &QAction::triggered, this, &QWidget::showNormal);

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
