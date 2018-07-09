#include <QtTest>

// add necessary includes here
#include "../nrthreadpool.h"    /* @@FIXME: proper way to do this ??? */

#include <QUdpSocket>

class Test_ThreadAllocationPolicy : public QObject
{
    Q_OBJECT

    static const qint32 NoOfThreads = 3;
    NRThreadPool* m_nrtpPtr;

public:
    Test_ThreadAllocationPolicy();
    ~Test_ThreadAllocationPolicy();

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void test_allocateObjectToOutOfRangeThreadId();
    void test_minJobPolicy();

};

Test_ThreadAllocationPolicy::Test_ThreadAllocationPolicy()
{

}

Test_ThreadAllocationPolicy::~Test_ThreadAllocationPolicy()
{

}

void Test_ThreadAllocationPolicy::initTestCase()
{

}

void Test_ThreadAllocationPolicy::init()
{
    m_nrtpPtr = new NRThreadPool(NoOfThreads, "TestThreadPool", NULL);
}

void Test_ThreadAllocationPolicy::cleanup()
{
    if ( NULL != m_nrtpPtr )
    {
        delete m_nrtpPtr;
    }
}

void Test_ThreadAllocationPolicy::cleanupTestCase()
{

}

void Test_ThreadAllocationPolicy::test_allocateObjectToOutOfRangeThreadId()
{
    QSKIP("NOT IMPLEMENTED YET", SkipSingle);

    QUdpSocket* objectPtr = new QUdpSocket;
    m_nrtpPtr->runObject( objectPtr, NoOfThreads);

}

void Test_ThreadAllocationPolicy::test_minJobPolicy()
{
    m_nrtpPtr->setPolicy(NRThreadPool::MinimumJobsPolicy);
    /* allocate 5 objs on thread 0 */
    int ObjectsToAllocate = 5;
    for (int i = 0; i< ObjectsToAllocate; i++)
    {
        QUdpSocket* theObjectPtr = new QUdpSocket;
        m_nrtpPtr->runObject( theObjectPtr, 0 );
    }

    /* allocate 4 objs on thread 1 */
    ObjectsToAllocate = 4;
    for (int i = 0; i< ObjectsToAllocate; i++)
    {
        QUdpSocket* theObjectPtr = new QUdpSocket;
        m_nrtpPtr->runObject( theObjectPtr, 1 );
    }

    /* allocate the N+1 object using the policy; we expect the pool will choose thread 2 */
    QUdpSocket* theObjectPtr = NULL;

    theObjectPtr = new QUdpSocket;
    m_nrtpPtr->runObject( theObjectPtr );
    QCOMPARE( 2, m_nrtpPtr->findTidOfObject(theObjectPtr) );

    /* allocate the N+2 object using the policy; we expect the pool will choose thread 2 */
    theObjectPtr = new QUdpSocket;
    m_nrtpPtr->runObject( theObjectPtr );
    QCOMPARE( 2, m_nrtpPtr->findTidOfObject(theObjectPtr) );

    /* verify the thread allocation map */
    QMap<int, int> m = m_nrtpPtr->threadAllocationMap();
    QCOMPARE( 5, m[0] );
    QCOMPARE( 4, m[1] );
    QCOMPARE( 2, m[2] );
}

QTEST_MAIN(Test_ThreadAllocationPolicy)

#include "tst_test_threadallocationpolicy.moc"
