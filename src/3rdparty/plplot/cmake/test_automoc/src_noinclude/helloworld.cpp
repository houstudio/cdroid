// Transcribed from <https://wiki.qt.io/Qt_for_Beginners>
// with additional modifications to #include a header
// that uses the Q_OBJECT macro so it must be processed
// by moc.
#include <QApplication>
#include <QPushButton>
#include "test_q_object.h"

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    QPushButton button( "Hello world!");
    button.show();
    return app.exec();
}
