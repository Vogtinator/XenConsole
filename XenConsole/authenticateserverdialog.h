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
    explicit AuthenticateServerDialog(QString name, QWidget *parent = 0);
    ~AuthenticateServerDialog();

    std::pair<QString,QString> getAuth();
    bool accepted();

signals:
    void connectionReady(QString username, QString password);

public slots:
    void accept();
    
private:
    Ui::AuthenticateServerDialog *ui;
    bool is_accepted = false;
};

#endif // AUTHENTICATESERVERDIALOG_H
