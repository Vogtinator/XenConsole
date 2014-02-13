#ifndef VMWINDOW_H
#define VMWINDOW_H

#include <QMainWindow>

/* Fix "macro argument mismatch." */
#ifndef Q_MOC_RUN
#include "krdc_VncView/vncview.h"
#include "xsconsolevnctunnel.h"
#include "xenserverapi.h"
#endif

namespace Ui {
class VMWindow;
}

class VMWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    VMWindow(xsvm_data *data, QWidget *parent = 0);
    ~VMWindow();

public slots:
    void vncError(QString title, QString msg);
    
private:
    Ui::VMWindow *ui;
    xsvm_data *data;
    VncView *view;
};

#endif // VMWINDOW_H
