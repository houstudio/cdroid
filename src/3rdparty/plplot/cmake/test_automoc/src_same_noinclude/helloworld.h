#ifndef TEST_Q_OBJECT_H
#define TEST_Q_OBJECT_H
// Vacuous class to test whether automoc works
// correctly.

#include <QObject>
class MyClass : public QObject
{
    Q_OBJECT

public:
    MyClass(QObject *parent = 0) {;}
    ~MyClass(){;}

};
#endif
