#include <QMessageBox>

#include "xsloadhostthread.h"

void XSLoadHostThread::run()
{
    QMutexLocker(data->xen_lock);

    emit progressChanged(0, trUtf8("Connecting to %1").arg(data->name));

    data->session = xen_session_login_with_password(call_func, reinterpret_cast<void*>(data), data->username.toUtf8().data(), data->password.toUtf8().data(), xen_api_latest_version);

    if(data->session == nullptr)
        return error(trUtf8("Failed to connect to %1").arg(data->name), nullptr);

    data->connected = true;

    data->connect_button->setText(trUtf8("Disconnect"));

    emit progressChanged(10, trUtf8("Fetching Host"));

    xen_host host_handle = nullptr;

    if(!xen_session_get_this_host(data->session, &host_handle, data->session))
        return error(trUtf8("Failed to fetch host!"), data->session);

    emit progressChanged(15, trUtf8("Fetching Data"));

    if(!xen_host_get_record(data->session, &data->data, host_handle))
        return error(trUtf8("Failed to fetch data!"), data->session);

    emit progressChanged(20, trUtf8("Fetching VMs"));
    xen_vm_xen_vm_record_map *vms;
    if(!xen_vm_get_all_records(data->session, &vms))
        return error(trUtf8("Failed to fetch VMs!"), data->session);

    data->vm_list_widget->clear();

    data->vm_count = vms->size;
    data->vms = new xsvm_data*[vms->size];
    for(size_t i = 0; i < data->vm_count; i++)
    {
        xen_vm_record *vm = vms->contents[i].val;

        xsvm_data *vm_data = new xsvm_data;
        vm_data->data = vm;
        vm_data->host = data;
        vm_data->tunnel = nullptr;
        vm_data->window = nullptr;

        data->vms[i] = vm_data;

        if(vm->is_a_snapshot || vm->is_a_template)
            continue;

        QListWidgetItem *item = new QListWidgetItem(QString::fromUtf8(vm->name_label), data->vm_list_widget);
        item->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<void*>(vm_data)));
    }

    data->vm_list_widget->setDisabled(false);

    emit progressChanged(100, trUtf8("Connected to %1").arg(data->name));
}
