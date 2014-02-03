#ifndef ADDSERVERDIALOG_H
#define ADDSERVERDIALOG_H

#include <QDialog>

class MainWindow;

namespace Ui {
class AddServerDialog;
}

class AddServerDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AddServerDialog(MainWindow *mw);
    ~AddServerDialog();

public slots:
    void accepted();
    void rejected();

signals:
    void addedServer(QString name, QString url, QString username, QString password);
    
private:
    Ui::AddServerDialog *ui;
    MainWindow *parent;
};

#endif // ADDSERVERDIALOG_H
