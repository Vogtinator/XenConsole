#include "krdc_VncView/vncview.h"

#include "vmwindow.h"
#include "ui_vmwindow.h"

VMWindow::VMWindow(xsvm_data *data, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VMWindow),
    data(data),
    view(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    setWindowTitle(data->data->name_label);
    ui->nameLabel->setText(data->data->name_label);

    if(data->data->power_state == XEN_VM_POWER_STATE_RUNNING)
    {
        if(data->tunnel == nullptr)
        {
            struct xen_console_record_opt_set *consoles = data->data->consoles;
            for(unsigned int i = 0; i < consoles->size; i++)
            {
                if(!consoles->contents[i]->is_record)
                {
                    xen_console handle = consoles->contents[i]->u.handle;
                    if(!xen_console_get_record(data->host->session, &(consoles->contents[i]->u.record), handle))
                        continue;

                    consoles->contents[i]->is_record = true;
                }

                if(consoles->contents[i]->u.record->protocol == XEN_CONSOLE_PROTOCOL_RFB)
                {
                    data->tunnel = new XSConsoleVNCTunnel(consoles->contents[i]->u.record->location, data->host->username, data->host->password);
                    break;
                }
            }
        }

        if(data->tunnel == nullptr)
        {
            ui->tabWidget->addTab(new QLabel(trUtf8("VNC not available"), ui->tabWidget), trUtf8("VNC"));
            return;
        }

        int port = data->tunnel->startListening();
        if(port == -1)
            ui->tabWidget->addTab(new QLabel(trUtf8("Couldn't connect to VNC"), ui->tabWidget), trUtf8("VNC"));
        else
        {            
            QWidget *widget = new QWidget(ui->tabWidget);
            QVBoxLayout *layout = new QVBoxLayout(widget);

            view = new VncView(widget, QString("vnc://localhost:%1").arg(port));
            view->enableScaling(true);
            view->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

            layout->addWidget(view);
            widget->setLayout(layout);

            ui->tabWidget->addTab(widget, trUtf8("VNC"));

            connect(view, SIGNAL(errorMessage(QString,QString)), this, SLOT(vncError(QString,QString)));
            view->start(RemoteView::Unknown);
            view->update();
        }
    }
}

VMWindow::~VMWindow()
{
    delete view;
    delete data->tunnel;
    data->tunnel = nullptr;
    delete ui;

    data->window = nullptr;
}

void VMWindow::vncError(QString title, QString msg)
{
    Q_UNUSED(title);

    QWidget *widget = qobject_cast<QWidget*>(view->parent());
    QLayout *layout = widget->layout();
    layout->removeWidget(view);
    layout->addWidget(new QLabel(msg));

}
