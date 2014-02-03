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

    if(data->data->power_state != XEN_VM_POWER_STATE_RUNNING)
        ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->vncTab));
    else
    {
        if(data->tunnel == nullptr)
        {
            for(unsigned int i = 0; i < data->data->consoles->size; i++)
            {
                if(!data->data->consoles->contents[i]->is_record)
                {
                    xen_console handle = data->data->consoles->contents[i]->u.handle;
                    if(!xen_console_get_record(data->host->session, &(data->data->consoles->contents[i]->u.record), handle))
                        continue;

                    data->data->consoles->contents[i]->is_record = true;
                }

                if(data->data->consoles->contents[i]->u.record->protocol == XEN_CONSOLE_PROTOCOL_RFB)
                {
                    data->tunnel = new XSConsoleVNCTunnel(data->data->consoles->contents[i]->u.record->location, data->host->username, data->host->password);
                    break;
                }
            }
        }

        if(data->tunnel == nullptr)
        {
            new QLabel(trUtf8("No consoles available."), ui->vncTab);
            return;
        }

        int port = data->tunnel->startListening();
        if(port == -1)
            new QLabel(trUtf8("Couldn't connect to VNC"), ui->vncTab);
        else
        {
            view = new VncView(ui->vncTab, QString("vnc://localhost:%1").arg(port));
            view->enableScaling(true);
            view->start();
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

void VMWindow::resizeEvent(QResizeEvent *)
{
    if(view)
    {
        QSize size = ui->vncTab->size();
        view->scaleResize(size.width(), size.height());
    }
}
