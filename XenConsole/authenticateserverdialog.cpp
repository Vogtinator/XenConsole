#include "authenticateserverdialog.h"
#include "ui_authenticateserverdialog.h"
#include "mainwindow.h"

AuthenticateServerDialog::AuthenticateServerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthenticateServerDialog)
{
    ui->setupUi(this);
}

AuthenticateServerDialog::~AuthenticateServerDialog()
{
    delete ui;
}

void AuthenticateServerDialog::setName(QString name)
{
    setWindowTitle(tr("Authentication for %1").arg(name));
}

void AuthenticateServerDialog::accept()
{
    this->close();
    emit connectionReady(ui->usernameEdit->text(), ui->passwordEdit->text());
}
