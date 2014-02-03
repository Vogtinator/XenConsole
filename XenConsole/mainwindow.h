#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QVector>
#include <QLabel>
#include <QProgressBar>

#include "xenserverapi.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    //Tab "new"
    void addServer();
    void protocolChanged(QString v);

    //Server tabs
    void editServer();
    void removeServer();
    void connectServer();

    //VM list
    void vmDoubleClicked(QModelIndex index);

    //For threads
    void xenError(QString msg, xen_session *session = nullptr);
    void showProgress(int percent, QString msg);

private:
    void disconnectHost(xshost_data &host);

    bool serverExists(QString name);

    void saveSettings();
    void loadSettings();
    void addTab(xshost_data &data);

    Ui::MainWindow *ui;
    QSettings settings;
    QHash<int, xshost_data> tab_data;
    QLabel status_label;
    QProgressBar progress_bar;
};

#endif // MAINWINDOW_H
