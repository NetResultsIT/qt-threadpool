#ifndef UT_MINIMUMJOBS_H
#define UT_MINIMUMJOBS_H

#include <QObject>

class NRThreadPool;

class ut_MinimumJobs : public QObject
{
    Q_OBJECT

    NRThreadPool* m_pool;

public:
    explicit ut_MinimumJobs(QObject *parent = nullptr);

private slots:
    void cleanup();

    void likeRobin();
    void doTest();
    void accumulate();

    void likeRobin_data();
    void doTest_data();
};

#endif // UT_MINIMUMJOBS_H
