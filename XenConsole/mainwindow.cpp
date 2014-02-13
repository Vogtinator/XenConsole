#include <utility>

#include <QLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QProgressBar>
#include <QDebug>
#include <QMdiSubWindow>
#include <QInputDialog>

#include "authenticateserverdialog.h"
#include "xenserverapi.h"
#include "krdc_VncView/vncview.h"
#include "vmwindow.h"
#include "xsloadhostthread.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

/* See below */
Q_DECLARE_METATYPE(QVector<int>)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    status_label.setHidden(true);
    progress_bar.setHidden(true);

    progress_bar.setMaximum(100);
    progress_bar.setMinimum(0);

    ui->statusBar->addPermanentWidget(&status_label, 3);
    ui->statusBar->addPermanentWidget(&progress_bar, 2);

    qRegisterMetaType<function_wrapper>();

    /* Qt5 complains about it not being registered. */
    qRegisterMetaType<QVector<int> >();

    connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(addServer()));
    connect(ui->protocol, SIGNAL(currentIndexChanged(QString)), this, SLOT(protocolChanged(QString)));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));

    loadSettings();
}

MainWindow::~MainWindow()
{
    for(auto &&d : tab_data)
        if(d.connected)
            xen_session_logout(d.session);

    delete ui;
}

void MainWindow::addServer()
{
    if(ui->nameEdit->text().isEmpty())
    {
        QMessageBox::critical(this, trUtf8("Error"), trUtf8("Name cannot be empty!"));

        ui->nameEdit->setFocus();
        return;
    }

    if(ui->portEdit->text().isEmpty())
    {
        QMessageBox::critical(this, trUtf8("Error"), trUtf8("Port number cannot be empty!"));

        ui->portEdit->setFocus();
        return;
    }

    if(ui->hostnameEdit->text().isEmpty())
    {
        QMessageBox::critical(this, trUtf8("Error"), trUtf8("Hostname cannot be empty!"));

        ui->hostnameEdit->setFocus();
        return;
    }

    xshost_data data;
    data.xen_lock = new QMutex;
    data.name = ui->nameEdit->text();
    data.url = QString("%1://%2:%3").arg(ui->protocol->currentText()).arg(ui->hostnameEdit->text()).arg(ui->portEdit->text());
    if(ui->saveAuthCheck->isChecked())
    {
        if(ui->usernameEdit->text().isEmpty())
        {
            QMessageBox::critical(this, trUtf8("Error"), trUtf8("Username cannot be empty!"));

            ui->usernameEdit->setFocus();
            return;
        }

        if(ui->passwordEdit->text().isEmpty())
        {
            QMessageBox::information(this, trUtf8("Useful tips"), trUtf8("Did you know that empty passwords are insecure?"));

            ui->passwordEdit->setFocus();
            return;
        }

        data.username = ui->usernameEdit->text();
        data.password = ui->passwordEdit->text();
    }
    else
        data.username = QString();

    data.connected = false;
    data.data = nullptr;

    addTab(data);

    saveSettings();
}

void MainWindow::saveSettings()
{
    QVariantList list;
    for(auto &&d : tab_data)
        list << QVariant::fromValue(d);

    settings.setValue("Hosts", QVariant(list));
}

void MainWindow::loadSettings()
{
    QVariantList list = settings.value("Hosts", QVariant(QVariantList{})).toList();
    for(auto &&d : list)
    {
        xshost_data data = d.value<xshost_data>();
        data.connected = false;
        data.data = nullptr;

        addTab(data);
    }
}

void MainWindow::addTab(xshost_data &data)
{
    QVBoxLayout *layout = new QVBoxLayout;
    QHBoxLayout *layout2 = new QHBoxLayout;
    QPushButton *edit_button = new QPushButton(trUtf8("Edit"));
    QPushButton *delete_button = new QPushButton(trUtf8("Delete"));

    connect(edit_button, SIGNAL(clicked()), this, SLOT(editServer()));
    connect(delete_button, SIGNAL(clicked()), this, SLOT(removeServer()));

    layout2->addWidget(edit_button);
    layout2->addWidget(delete_button);
    layout->addLayout(layout2);

    data.vm_list_widget = new QListWidget;
    data.vm_list_widget->setDisabled(true);
    connect(data.vm_list_widget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(vmDoubleClicked(QModelIndex)));
    layout->addWidget(data.vm_list_widget);

    data.connect_button  = new QPushButton(trUtf8("Connect"));
    connect(data.connect_button, SIGNAL(clicked()), this, SLOT(connectServer()));
    layout->addWidget(data.connect_button);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    int id = ui->tabWidget->addTab(widget, data.name);

    tab_data[id] = data;
}

void MainWindow::protocolChanged(QString v)
{
    if(v.compare("https", Qt::CaseInsensitive) == 0)
        ui->portEdit->setText("443");
    else
        ui->portEdit->setText("80");
}

void MainWindow::editServer()
{
    //TODO
}

void MainWindow::removeServer()
{
    int tab = ui->tabWidget->currentIndex();
    ui->tabWidget->removeTab(tab);

    xshost_data &host = tab_data[tab];

    if(host.connected)
        disconnectHost(host);

    tab_data.remove(tab);

    saveSettings();
}

void MainWindow::connectServer()
{
    xshost_data &host = tab_data[ui->tabWidget->currentIndex()];

    if(host.connected)
    {
        disconnectHost(host);

        showProgress(100, trUtf8("Disconnected."));
        return;
    }

    if(host.username.isEmpty())
    {
        AuthenticateServerDialog dialog(host.name, this);

        dialog.show();
        dialog.exec();

        if(!dialog.accepted())
            return;

        std::tie(host.username, host.password) = dialog.getAuth();
    }

    XSLoadHostThread *thread = new XSLoadHostThread(&host);

    connect(thread, SIGNAL(progressChanged(int,QString)), this, SLOT(showProgress(int,QString)), Qt::QueuedConnection);
    connect(thread, SIGNAL(error(QString,xen_session*)), this, SLOT(xenError(QString,xen_session*)), Qt::QueuedConnection);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(iWantToDoSomething(function_wrapper)), this, SLOT(doInThisThread(function_wrapper)), Qt::QueuedConnection);

    thread->start();
}

void MainWindow::xenError(QString msg, xen_session *session)
{
    if(session && session->error_description_count > 0)
        msg += "\nCause: " + QString::fromUtf8(*session->error_description);

    QMessageBox::critical(this, trUtf8("Failure"), msg);
}

void MainWindow::showProgress(int percent, QString msg)
{
    status_label.setText(msg);
    progress_bar.setValue(percent);

    status_label.setHidden(false);
    progress_bar.setHidden(false);
}

void MainWindow::vmDoubleClicked(QModelIndex index)
{
    xsvm_data *vm = reinterpret_cast<xsvm_data*>(index.data(Qt::UserRole).value<void*>());

    if(vm->window)
    {
        ui->mdiArea->setActiveSubWindow(vm->window);
        return;
    }

    vm->window = ui->mdiArea->addSubWindow(new VMWindow(vm, nullptr));
    vm->window->resize(350, 350);
    vm->window->show();
}

void MainWindow::disconnectHost(xshost_data &host)
{
    for(size_t i = 0; i < host.vm_count; i++)
    {
        xsvm_data *vm = host.vms[i];
        if(vm->window)
        {
            delete vm->window;
            vm->window = nullptr;
        }

        if(vm->tunnel)
        {
            delete vm->tunnel;
            vm->tunnel = nullptr;
        }

        xen_vm_record_free(vm->data);
        vm->data = nullptr;
    }

    delete[] host.vms;
    host.vms = nullptr;

    xen_host_record_free(host.data);
    host.data = nullptr;

    xen_session_logout(host.session);
    host.session = nullptr;

    host.connected = false;
    host.connect_button->setText(trUtf8("Connect"));
    host.vm_list_widget->setDisabled(true);

    xen_vm_xen_vm_record_map_free(host.vm_map);
    host.vm_map = nullptr;
}

void MainWindow::doInThisThread(function_wrapper f)
{
    f.f();

    QThread *thread = qobject_cast<QThread*>(QObject::sender());
    thread->quit();
}
