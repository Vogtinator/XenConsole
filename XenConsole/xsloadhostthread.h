#ifndef XSLOADHOSTTHREAD_H
#define XSLOADHOSTTHREAD_H

#include <functional>

#include <QThread>

#include "xenserverapi.h"

/* Qt 5 has a bug which makes it impossible to use std::function directly */
struct function_wrapper {
    std::function<void()> f;
};

Q_DECLARE_METATYPE(function_wrapper)

class XSLoadHostThread : public QThread
{
    Q_OBJECT
public:
    XSLoadHostThread(xshost_data* data)
        : data(data) {}

    void run();

signals:
    void error(QString msg, xen_session *session);
    void progressChanged(int percent, QString msg);
    void iWantToDoSomething(function_wrapper f);

private:
    void doInGUIThread(std::function<void()> f);

    xshost_data *data;
};

#endif // XSLOADHOSTTHREAD_H
