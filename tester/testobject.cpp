#include "testobject.h"

#include <QVector>
#include <QDebug>

#include <unistd.h>

TestObject::TestObject(int numobj2test, QObject *parent) :
    QObject(parent)
{

    for (int i=0; i<numobj2test; i++) {
        qDebug() << "Creating Worker Object " << i;
        WorkerObject *wo = new WorkerObject(i);
        wo->setObjectName("WObj" + QString::number(i));
        connect(wo, SIGNAL(stoppedWork(int)), this, SLOT(handleWorkerStop(int)));
        v.append( wo );
    }

    tpool.setPoolName("PoolTester");


    qDebug() << tpool.getPoolStatus();

    foreach(WorkerObject* wo, v) {
        tpool.runObject(wo);
        wo->work(10, 250);
    }

    for (int i=0; i<10; i++) {
        qDebug() << "Reading pool status...";
        usleep(200000);
        qDebug() << tpool.getPoolStatus();
    }
}


void
TestObject::handleWorkerStop(int i)
{
    qDebug() << "worker" << i << "reported stopped working";
    qDebug() << "examining v to delete the correct worker" << v;
    foreach(WorkerObject *wop, v) {
        if (wop) {
            qDebug() << wop;
            if(wop->getId() == i) {
                wop->deleteLater();
                v[i] = NULL;
                return;
            }
        }
    }
}
