#ifndef XENSERVERAPI_H
#define XENSERVERAPI_H

#include <QString>
#include <QMetaType>
#include <QListWidget>
#include <QPushButton>
#include <QMdiSubWindow>
#include <QMutex>

extern "C" {
    #include <xen/api/xen_all.h>
    #include <curl/curl.h>
    #include <libxml/parser.h>
}

struct xsvm_data;
class XSConsoleVNCTunnel;

struct xshost_data {
    QString name;
    QString url;
    QString username;
    QString password;
    bool save_auth;
    bool connected;
    QMutex *xen_lock;
    xen_session *session;
    xen_host_record *data;
    size_t vm_count;
    xsvm_data **vms;
    xen_vm_xen_vm_record_map *vm_map;
    QListWidget *vm_list_widget;
    QPushButton *connect_button;
};

struct xsvm_data {
    xshost_data *host;
    xen_vm_record *data;
    XSConsoleVNCTunnel *tunnel;
    QMdiSubWindow *window;
};

QDataStream &operator<<(QDataStream &out, const xshost_data &obj);
QDataStream &operator>>(QDataStream &in, xshost_data &obj);

Q_DECLARE_METATYPE(xshost_data)

int call_func(const void *data, size_t len, void *user_handle, void *result_handle, xen_result_func result_func);

#endif // XENSERVERAPI_H
