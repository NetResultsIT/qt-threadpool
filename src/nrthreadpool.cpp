#include "nrthreadpool.h"
#include <QDebug>
#include <QStringList>
#include <limits>


BEGIN_NRTHREADPOOL_NAMESPACE


static QString generateThreadName(const QString &name, int i)
{
    return QString("%1 T_%2").arg(name).arg(i);
}

/* *********************** *
 * RUNNINGTHREADINFO STUFF *
 * *********************** */
//Note: VS < 2105 does not have snprintf!
#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf(buf,len, format,...) _snprintf_s(buf, len,len, format, __VA_ARGS__)
#endif


QString
RunningQThreadInfo::getInfoString() const {
    QString s = "TID / ObjNum / ObjList: " + QString::number(m_tid) + " / " + QString::number(m_assignedObjList.count());
    foreach (QObject *op, m_assignedObjList) {
        s += " / " + PTR2QSTRING(op);
    }

    return s;
}



/* ******************* *
 * NRThreadPool STUFF  *
 * ******************* */
/*!
 * \brief NRThreadPool::NRThreadPool Constructor a ThreadPool class, this class simplies running QObjects on a group of threads
 * \param numberOfThreads2Spawn (optional) number of threads to be spawned
 * \param poolname (optional) a string that will be assigned to threads (for gdb display)
 * \param parent (optional) the pointer to the parent QObject
 */
NRThreadPool::NRThreadPool(int numberOfThreads2Spawn, const QString &poolname, QObject *parent) :
    QObject(parent),
    m_lastUsedThread(-1),
    m_threadUsagePolicy(RoundRobinPolicy),
    m_poolName(poolname)
{
    this->setObjectName(m_poolName);
    m_numOfThreads = (numberOfThreads2Spawn == 0) ? QThread::idealThreadCount() : numberOfThreads2Spawn;
    TPDBG << "NRThreadPool is spawning" << m_numOfThreads << "threads";
    for (int i = 0; i < m_numOfThreads; i++) {
        QThread *t = new QThread(this);
        t->setObjectName(generateThreadName(m_poolName, i));
        TPDBG << "Spawned Thread" << i << t;
        m_v.append( t );
        m_threadUsageCountMap.insert(i, RunningQThreadInfo(i));

        connect(t, SIGNAL(started()), this, SLOT(handleThreadStarted()));
        connect(t, SIGNAL(finished()), this, SLOT(handleThreadFinished()));
    }
}



NRThreadPool::~NRThreadPool()
{
    foreach(QThread *t, m_v) {
        t->exit();
        t->wait();
    }
}



/*!
 * \brief NRThreadPool::renameThreads Changes the name of threads displayed by GDB
 * \param name the new Thread's name
 * \note This method does not affect already running threads
 */
void
NRThreadPool::renameThreads(const QString &name)
{
    for (int i = 0; i < m_numOfThreads; i++) {
        m_v[i]->setObjectName(generateThreadName(name, i));
    }
}



//this will take place only if threads are restarted (not done yet)
void
NRThreadPool::setPoolName(const QString &name)
{
    m_poolName = name;
    this->setObjectName(m_poolName);
    this->renameThreads(m_poolName);
}



/*!
 * \brief NRThreadPool::increaseThreadObjects Records the fact that a QObject is now being run on a QThread
 * \param tid the Thread id we're adding the new object to
 * \param o the pointer of the QObject being added to the thread
 */
void
NRThreadPool::increaseThreadObjects(int tid, QObject *o)
{
    mux.lock();
    m_threadUsageCountMap[tid].addObject(o);
    mux.unlock();

    // notify the new status
    emit sigThreadAllocationChanged(threadAllocationMap());
}


/*!
 * \brief NRThreadPool::decreaseThreadObjects
 * \param tid
 * \param o
 */
void
NRThreadPool::decreaseThreadObjects(int tid, QObject *o)
{
    mux.lock();
    if ( !m_threadUsageCountMap.contains(tid) )
    {
        TPDBG << "Thread " << tid << " not found in the map, aborting decrease operation";
        mux.unlock();
        return;
    }
    m_threadUsageCountMap[tid].removeObject(o);
    Q_ASSERT(m_threadUsageCountMap[tid].numAssignedObjects() >= 0);
    if (0 == m_threadUsageCountMap[tid].numAssignedObjects()) {
        TPDBG << "Count of objects on thread " << tid << "is 0, stopping it...";
        m_v[tid]->quit();
    }
    mux.unlock();

    // notify the new status
    emit sigThreadAllocationChanged(threadAllocationMap());
}



/*!
 * \brief NRThreadPool::findTidOfObject finds the id of the thread where an object is running
 * \param o The object pointer we're trying to find
 * \return the id of the Thread where the passed Object is running or -1 if that is not found
 */
int
NRThreadPool::findTidOfObject(const QObject *o)
{
    mux.lock();
    foreach (RunningQThreadInfo rti, m_threadUsageCountMap.values()) {
        if (rti.hasObject(o)) {
            mux.unlock();
            return rti.associatedThread();
        }
    }
    mux.unlock();
    return -1;
}


/*!
 * \brief NRThreadPool::handleObjectDestruction When an Object that was being run on the pool is destroyed we remove its reference and,
 * eventually, stop the thread (if no more objects are being run on it)
 * \param i_op the pointer of the QObject that has been destoryed
 */
void
NRThreadPool::handleObjectDestruction(QObject *i_op)
{
    TPDBG << "Object " << PTR2QSTRING(i_op) << "was destroyed... we should remove it from our threadpool";
    int tid = findTidOfObject(i_op);
    TPDBG << "Object was running on thread" << tid << "removing it...";
    this->decreaseThreadObjects(tid, i_op);
    TPDBG << this->getPoolStatus();
}


/*!
 * \brief NRThreadPool::handleThreadStarted Log when a thread is started
 */
void
NRThreadPool::handleThreadStarted()
{
    QThread *t = dynamic_cast<QThread*>(sender());
    if (!t)
    {
        TPDBG << "Got NULL sender threadpool's thread!";
        return;
    }
    TPDBG << "Threadpool's thread" << t->objectName() << "has been started";
}

/*!
 * \brief NRThreadPool::handleThreadFinished Log when a thread is finished
 */
void
NRThreadPool::handleThreadFinished()
{
    QThread *t = dynamic_cast<QThread*>(sender());
    if (!t)
    {
        TPDBG << "Got NULL sender threadpool's thread!";
        return;
    }
    TPDBG << "Threadpool's thread" << t->objectName() << "has been stopped";
}



/*!
 * \brief NRThreadPool::runObject runs the specified QObject on a QThread from the pool
 * \param o the pointer to the object that is being moved and run on a new thread
 * \param preferred_tid (optional) the id of a thread where we'd prefer the object to run (i.e. to have two objects run on the same thread)
 * \return
 */
int
NRThreadPool::runObject(QObject *o, int preferred_tid)
{
    /*
     *  @@FIXME: there is no check that the preferred_tid is not out of range
     *  if an out-of-range value is passed an abort is generated by the QASSERT in the qvector
     */
    int tid;
    if (preferred_tid == -1) {
        tid = findThread2Use();
    }
    else {
        tid = preferred_tid;
    }
    TPDBG << "Object " << o << "will be moved to thread" << tid << m_v[tid];
    o->moveToThread( m_v[tid] );
    increaseThreadObjects(tid, o);

    if (!m_v[tid]->isRunning()) {
        TPDBG << "Thread " << tid << " was not started yet... starting now";
        m_v[tid]->start();
    }

    connect(o, SIGNAL(destroyed(QObject*)), this, SLOT(handleObjectDestruction(QObject*)));

    /*
     *  @@FIXME: might be changed to return the thread id the object has been allocated to
     *  !! WARNING !! this might break the code using it; change to be carefully evaluated
     */
    return 0;
}


/*!
 * \brief NRThreadPool::findThread2Use Chooses, based on the desired policy, on which thread an Object should be run
 * \return the id of thread where the object should be run
 * \note currently the Two policies being used are: RoundRobin (the default) and MinimumJobs that chooses
 * the thread with the minimum number of assigned objects
 */
int
NRThreadPool::findThread2Use()
{
    int retval = -1;

    switch(m_threadUsagePolicy) {
        case MinimumJobsPolicy:
        {
            mux.lock();
                int max = std::numeric_limits<int>::max();
                retval = (++m_lastUsedThread) % m_numOfThreads;

                QList<RunningQThreadInfo> l = m_threadUsageCountMap.values();
                foreach (RunningQThreadInfo rti, l) {
                    if (rti.numAssignedObjects() < max) {
                        max = rti.numAssignedObjects();
                        retval = rti.associatedThread();
                    }
                }
            mux.unlock();
        }
        break;
        case RoundRobinPolicy:
        default:
            mux.lock();
                retval = (++m_lastUsedThread) % m_numOfThreads;
            mux.unlock();
    }

    m_lastUsedThread = retval;
    return retval;
}



/*!
 * \brief NRThreadPool::getPoolStatus is an utility functions that returns a string with info about the ppol and its threads
 * \return The string containing the status of the pool
 */
QString
NRThreadPool::getPoolStatus()
{
    QString s;
    QStringList sl;
    mux.lock();
    foreach (RunningQThreadInfo rti, m_threadUsageCountMap.values()) {
        sl << rti.getInfoString();
        sl << ( (m_v[rti.associatedThread()]->isRunning()) ? "(RUNNING)" : "(STOPPED)" );
    }
    mux.unlock();
    return sl.join(" -- ");
}

TPoolAllocationMap
NRThreadPool::threadAllocationMap()
{
    //m_threadUsageCountMap
    QMap<int, int> m;
    foreach (RunningQThreadInfo info, m_threadUsageCountMap)
    {
        m.insert(info.associatedThread(), info.numAssignedObjects());
    }
    return m;
}

END_NRTHREADPOOL_NAMESPACE
