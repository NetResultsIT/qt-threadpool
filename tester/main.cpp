#include <QCoreApplication>
#include <testobject.h>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TestObject to(3);

    return a.exec();
}
