#ifndef MEDIAQUEUE_H
#define MEDIAQUEUE_H

#include <QQueue>
#include <QMutex>

#define TRY_LOCK_TIMEOUT 1000*10 //milliseconds

template <class T>
class MediaQueue : public QQueue<T>
{
public:
    inline MediaQueue(){
    }

    inline MediaQueue(const MediaQueue<T>& queue){
        QList<T>::QList(queue);
    }

    inline MediaQueue<T> operator=(const MediaQueue<T>& queue){
        *this = queue;
        return *this;
    }

    inline void enqueue(const T &t){
//        if(mutex.tryLock(TRY_LOCK_TIMEOUT)){
//            QList<T>::append(t);
//            mutex.unlock();
//        }
        mutex.lock();
        QList<T>::append(t);
        mutex.unlock();
    }

    inline T dequeue(){
        T ret;
//        if(mutex.tryLock(TRY_LOCK_TIMEOUT)){
//            ret = QList<T>::takeFirst();
//            mutex.unlock();
//        }
        mutex.lock();
        if(!QList<T>::isEmpty()){
            ret = QList<T>::takeFirst();
        }
        mutex.unlock();
        return ret;
    }

    void clear(){
        mutex.lock();
        QList<T>::clear();
        mutex.unlock();
    }

private:
    QMutex mutex;
};

#endif // MEDIAQUEUE_H
