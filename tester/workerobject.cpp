#include "workerobject.h"

#include <QThread>
#include <QDebug>
#include <unistd.h>


WorkerObject::WorkerObject(int id, QObject *parent) :
    QObject(parent),
    m_id(id)
{
    bool b = (bool) connect (this, SIGNAL(startWork(int,int)), this, SLOT(work_private(int,int)));

    Q_ASSERT(b);
}


WorkerObject::~WorkerObject()
{
    qDebug() << "Destroyed worker " << m_id;
}

void
WorkerObject::work_private(int cycles, int sleepmsecs)
{
    for (int i=0; i < cycles; i++) {
        qDebug() << "This is Worker " << m_id << " running on thread " << QThread::currentThread() << "executin cycle " << i+1 << "/" << cycles;
        emit workedOneCycle(m_id);
        if (sleepmsecs) {
            qDebug() << "Worker " << m_id << " will now sleep for " << sleepmsecs;
            usleep(sleepmsecs * 1000);
        }
    }
    emit stoppedWork(m_id);
}


void
WorkerObject::work(int cycles, int sleepmsecs)
{
    emit startWork(cycles, sleepmsecs);
}
