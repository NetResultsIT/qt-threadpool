#include "ut_RoundRobin.h"

#include <QSignalSpy>
#include <QTest>
#include <nrthreadpool.h>

ut_RoundRobin::ut_RoundRobin(QObject *parent)
    : QObject(parent)
    , m_pool(NULL)
{
}

void ut_RoundRobin::cleanup()
{
    if (m_pool)
    {
        QSignalSpy deleteSpy(m_pool, SIGNAL(destroyed(QObject*)));

        m_pool->deleteLater();
        m_pool = NULL;

        while (deleteSpy.count() != 1)
        {
            QTest::qWait(10);
        }
    }
}

void ut_RoundRobin::doTest()
{
    QFETCH(int, threadNo);
    QFETCH(int, workerNo);

    m_pool = new NRThreadPool(threadNo);
    m_pool->setPolicy(NRThreadPool::RoundRobinPolicy);

    QCOMPARE(m_pool->threadsInPool(), threadNo);

    QList<QObject*> objectList;
    for (int i = 0; i < workerNo; i++)
    {
        QObject *o = new QObject();

        objectList << o;
        m_pool->runObject(o);
    }

    for (int i = 0; i < workerNo; i++)
    {
        QObject *obj = objectList[i];
        int tid = m_pool->findTidOfObject(obj);

        QCOMPARE( tid, i % threadNo );
    }

    //qDebug() << m_pool->getPoolStatus();

    foreach (QObject *obj, objectList)
    {
        QSignalSpy deleteSpy(obj, SIGNAL(destroyed(QObject*)));
        obj->deleteLater();

        while (deleteSpy.count() != 1)
        {
            QTest::qWait(10);
        }
    }

    //qDebug() << m_pool->getPoolStatus();
}

void ut_RoundRobin::doTest_data()
{
    QTest::addColumn<int> ("threadNo");
    QTest::addColumn<int> ("workerNo");

    QTest::newRow("1 th - 1 wk") << 1 << 1;
    QTest::newRow("1 th - 5 wk") << 1 << 5;
    QTest::newRow("5 th - 1 wk") << 5 << 1;
    QTest::newRow("5 th - 5 wk") << 5 << 5;
}
