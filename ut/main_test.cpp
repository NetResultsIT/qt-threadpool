
#include "ut_RoundRobin.h"
#include "ut_MinimumJobs.h"

#include <QCoreApplication>
#include <QTest>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    ut_RoundRobin test1;
    ut_MinimumJobs test2;

    QTest::qExec(&test1, app.arguments());
    QTest::qExec(&test2, app.arguments());

    return 0;
}
