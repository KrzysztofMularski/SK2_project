#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // DPI support
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps); //HiDPI pixmaps
    qputenv("QT_SCALE_FACTOR", "1");
    QApplication a(argc, argv);
    QFile styleSheetFile("../qt_app/Diffnes.qss");
    styleSheetFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(styleSheetFile.readAll());
    a.setStyleSheet(styleSheet);
    MainWindow w;
    w.setWindowTitle("Mnożenie macierzy");
    w.setWindowIcon(QIcon(":matrix.png"));
    w.show();
    return a.exec();
}
