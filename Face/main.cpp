#include "faceidentify.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FaceIdentify w;
    w.show();
    return a.exec();
}
