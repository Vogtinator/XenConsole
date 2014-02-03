#ifndef XSLOADHOSTTHREAD_H
#define XSLOADHOSTTHREAD_H

#include <QThread>
#include "xenserverapi.h"

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

private:
    xshost_data *data;
};

#endif // XSLOADHOSTTHREAD_H
