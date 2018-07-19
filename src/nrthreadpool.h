#ifndef NRTHREADPOOL_H
#define NRTHREADPOOL_H

#include <QObject>
#include <QThread>
#include <QMap>
#include <QMutex>
#include <QVector>
#include <QMap>
#include <QDebug>

#ifdef ENABLE_TPOOL_DBG
    #define TPDBG qDebug()
#else
    #define TPDBG if (true) {} else qDebug()
#endif


#ifndef PTR2QSTRING
#define PTR2QSTRING(x) QString("0x%1").arg((quintptr)x, 16, 16, QChar('0'))
#endif


typedef QMap<int,int> TPoolAllocationMap;


class RunningQThreadInfo
{
    QList<QObject*> m_assignedObjList;
    int m_tid;

public:
    explicit RunningQThreadInfo(int tid = -1)
        :m_tid(tid) { /* EMPTY */ }
    int numAssignedObjects() const  { return m_assignedObjList.count(); }
    void addObject(QObject*o)       { m_assignedObjList.append(o);      }
    void removeObject(QObject*o)    { m_assignedObjList.removeAll(o);   }
    int associatedThread() const    { return m_tid; }
    bool hasObject(const QObject *o) const { return m_assignedObjList.contains(const_cast<QObject*>(o)); }
    QString getInfoString() const;
};

class NRThreadPool : public QObject
{
    Q_OBJECT

public:
    enum ThreadAssignmentPolicy { RoundRobinPolicy, MinimumJobsPolicy };

private:
    QVector<QThread* > m_v;
    int m_lastUsedThread;
    int m_numOfThreads;
    ThreadAssignmentPolicy m_threadUsagePolicy;
    QMap<int, RunningQThreadInfo> m_threadUsageCountMap;
    QMutex mux;
    QString m_poolName;

    int findThread2Use();
    void increaseThreadObjects(int tid, QObject*);
    void decreaseThreadObjects(int tid, QObject *);
    void renameThreads(const QString &name);

private slots:
    void handleObjectDestruction(QObject *i_op);

public:
    explicit NRThreadPool(int numberOfThreads2Spawn=0, const QString &poolname="NrTPool", QObject *parent = 0);
    virtual ~NRThreadPool();
    void setPolicy(ThreadAssignmentPolicy tap) { m_threadUsagePolicy = tap; }
    int runObject(QObject *o, int preferred_tid=-1);
    int threadsInPool() const { return m_numOfThreads; }
    void setPoolName(const QString &name);
    QString getPoolName() { return m_poolName; }
    QString getPoolStatus();
    int findTidOfObject(const QObject *);

    /*!
     * \brief threadAllocationMap
     *  returns a map with the thread id -> no of running objects
     * \returns the map with the thread id to running object mapping
     */
    TPoolAllocationMap threadAllocationMap();

signals:
    void sigThreadPoolStatusChanged(const TPoolAllocationMap &threadAllocMap);

public slots:

};

#include <QMetaType>
Q_DECLARE_METATYPE(TPoolAllocationMap)

#endif // NRTHREADPOOL_H
