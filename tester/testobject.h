#ifndef TESTOBJECT_H
#define TESTOBJECT_H

#include <QObject>
#include <nrthreadpool.h>
#include <workerobject.h>

class TestObject : public QObject
{
    Q_OBJECT
    NRThreadPool tpool;
    QVector<WorkerObject*> v;
public:
    explicit TestObject(int numobj2test, QObject *parent = 0);

signals:

public slots:
    void handleWorkerStop(int i);

};

#endif // TESTOBJECT_H
