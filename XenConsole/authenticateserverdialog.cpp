#include "authenticateserverdialog.h"
#include "ui_authenticateserverdialog.h"
#include "mainwindow.h"

AuthenticateServerDialog::AuthenticateServerDialog(QString name, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthenticateServerDialog)
{
    ui->setupUi(this);

    setWindowTitle(tr("Authentication for %1").arg(name));

    ui->passwordEdit->setFocus();
}

AuthenticateServerDialog::~AuthenticateServerDialog()
{
    delete ui;
}

std::pair<QString, QString> AuthenticateServerDialog::getAuth()
{
    return std::make_pair(ui->usernameEdit->text(), ui->passwordEdit->text());
}

bool AuthenticateServerDialog::accepted()
{
    return is_accepted;
}

void AuthenticateServerDialog::accept()
{
    this->close();

    is_accepted = true;
    emit connectionReady(ui->usernameEdit->text(), ui->passwordEdit->text());
}
