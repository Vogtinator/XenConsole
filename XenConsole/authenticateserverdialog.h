#ifndef AUTHENTICATESERVERDIALOG_H
#define AUTHENTICATESERVERDIALOG_H

#include <QDialog>

namespace Ui {
class AuthenticateServerDialog;
}

class AuthenticateServerDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AuthenticateServerDialog(QWidget *parent = 0);
    ~AuthenticateServerDialog();

    void setName(QString name);
signals:
    void connectionReady(QString username, QString password);

public slots:
    void accept();
    
private:
    Ui::AuthenticateServerDialog *ui;
};

#endif // AUTHENTICATESERVERDIALOG_H
