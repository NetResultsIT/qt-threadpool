#ifndef WORKEROBJECT_H
#define WORKEROBJECT_H

#include <QObject>

class WorkerObject : public QObject
{
    Q_OBJECT

    int m_id;

private slots:
    void work_private(int cycles, int sleepmsecs=0);

public:
    explicit WorkerObject(int id, QObject *parent = 0);
    ~WorkerObject();
    int getId() const { return m_id; }

signals:
    void startWork(int,int);
    void stoppedWork(int);
    void workedOneCycle(int);

public slots:
    void work(int cycles, int sleepmsecs=0);

};

#endif // WORKEROBJECT_H
