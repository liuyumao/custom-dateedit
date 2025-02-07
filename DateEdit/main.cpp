#include <QApplication>
#include <QFile>
#include "Widget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile qssFile(":/res/style.qss");
    qssFile.open(QFile::ReadOnly);
    qApp->setStyleSheet(qssFile.readAll());
    qssFile.close();

    Widget w;
    w.show();
    return a.exec();
}
