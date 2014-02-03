#ifndef VMWINDOW_H
#define VMWINDOW_H

#include <QMainWindow>
#include "krdc_VncView/vncview.h"
#include "xenserverapi.h"
#include "xsconsolevnctunnel.h"

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
    void resizeEvent(QResizeEvent *);
    
private:
    Ui::VMWindow *ui;
    xsvm_data *data;
    VncView *view;
};

#endif // VMWINDOW_H
