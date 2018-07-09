#ifndef UT_ROUNDROBIN_H
#define UT_ROUNDROBIN_H

#include <QObject>

class NRThreadPool;

class ut_RoundRobin : public QObject
{
    Q_OBJECT

    NRThreadPool* m_pool;

public:
    explicit ut_RoundRobin(QObject *parent = 0);

private slots:
    void cleanup();

    void doTest();

    void doTest_data();

};

#endif // UT_ROUNDROBIN_H
