#include "ut_MinimumJobs.h"

#include <QTest>
#include <QSignalSpy>
#include <nrthreadpool.h>

ut_MinimumJobs::ut_MinimumJobs(QObject *parent) : QObject(parent)
{

}

void ut_MinimumJobs::cleanup()
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

void ut_MinimumJobs::likeRobin()
{
    QFETCH(int, threadNo);
    QFETCH(int, workerNo);

    m_pool = new NRThreadPool(threadNo);
    m_pool->setPolicy(NRThreadPool::MinimumJobsPolicy);

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

void ut_MinimumJobs::doTest()
{
    QFETCH(int, threadNo);
    QFETCH(int, workerToDelete);

    m_pool = new NRThreadPool(threadNo);
    m_pool->setPolicy(NRThreadPool::MinimumJobsPolicy);

    QList<QObject*> objectList;
    for (int i = 0; i < threadNo; i++)
    {
        QObject *obj = new QObject();
        objectList << obj;
        m_pool->runObject(obj);

        QCOMPARE( m_pool->findTidOfObject(obj), i % threadNo );
    }

    // Delete worker
    {
        QObject* obj = objectList[workerToDelete];
        QSignalSpy deleteSpy(obj, SIGNAL(destroyed(QObject*)));

        obj->deleteLater();
        while (deleteSpy.count() != 1)
            QTest::qWait(10);

        QTest::qWait(10);
        qDebug() << "Object" << workerToDelete << "deleted";

        obj = new QObject();
        objectList[workerToDelete] = obj;
        m_pool->runObject(obj);

        QCOMPARE( m_pool->findTidOfObject(obj), workerToDelete );
    }
}

void ut_MinimumJobs::accumulate()
{
    int threadNo = 5;
    int workerNo = 20;

    m_pool = new NRThreadPool(threadNo);
    m_pool->setPolicy(NRThreadPool::MinimumJobsPolicy);

    QList<QObject*> blockedObjectList;
    QList<QObject*> objectList;

    for (int i = 0; i < workerNo; i++)
    {
        QObject* obj = new QObject();
        if (i % threadNo == 0)
            blockedObjectList << obj;
        else
            objectList << obj;

        m_pool->runObject(obj);
        QCOMPARE( m_pool->findTidOfObject(obj), i % threadNo );
    }

    foreach (QObject* obj, objectList)
    {
        obj->deleteLater();
    }

    QTest::qWait(100);

    int size = objectList.size();
    objectList.clear();

    for (int i = 0; i < size; i++)
    {
        QObject* obj = new QObject();
        objectList << obj;

        m_pool->runObject(obj);

        QVERIFY( m_pool->findTidOfObject(obj) != 0 ); // The first thread is full
    }
}

void ut_MinimumJobs::likeRobin_data()
{
    QTest::addColumn<int> ("threadNo");
    QTest::addColumn<int> ("workerNo");

    QTest::newRow("1 th - 1 wk") << 1 << 1;
    QTest::newRow("1 th - 5 wk") << 1 << 5;
    QTest::newRow("5 th - 1 wk") << 5 << 1;
    QTest::newRow("5 th - 5 wk") << 5 << 5;
}

void ut_MinimumJobs::doTest_data()
{
    QTest::addColumn<int> ("threadNo");
    QTest::addColumn<int> ("workerToDelete");

    QTest::newRow("1 th") << 1 << 0;
    QTest::newRow("5 th") << 5 << 3;
}
