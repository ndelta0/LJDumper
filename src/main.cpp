#include <QApplication>

#include "views/LjdMainWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    LjdMainWindow w;
    w.show();

    return QApplication::exec();
}