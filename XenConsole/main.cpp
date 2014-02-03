#include <QApplication>
#include <QTextCodec>
#include <QDebug>
#include "xenserverapi.h"

extern "C" {
    #include <curl/curl.h>
    #include <libxml/parser.h>
}

#include "mainwindow.h"

QDataStream &operator<<(QDataStream &out, const xshost_data &obj)
{
    out << obj.name << obj.url;

    if(obj.save_auth)
        out << obj.username << obj.password;
    else
        out << "" << "";

    return out;
}

QDataStream &operator>>(QDataStream &in, xshost_data &obj)
{
    in >> obj.name >> obj.url >> obj.username >> obj.password;

    if(obj.username.isEmpty())
        obj.save_auth = false;
    else
        obj.save_auth = true;

    return in;
}

int main(int argc, char *argv[])
{
    xmlInitParser();
    xmlKeepBlanksDefault(0);
    xen_init();
    curl_global_init(CURL_GLOBAL_ALL);

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
        QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

    qRegisterMetaType<xshost_data>();
    qRegisterMetaTypeStreamOperators<xshost_data>();

    QApplication a(argc, argv);
    QCoreApplication::setOrganizationDomain("Vogtinator");
    QCoreApplication::setApplicationName("XenConsole");

    MainWindow w;
    w.show();
    
    int ret = a.exec();

    curl_global_cleanup();
    xen_fini();
    xmlCleanupParser();

    return ret;
}
