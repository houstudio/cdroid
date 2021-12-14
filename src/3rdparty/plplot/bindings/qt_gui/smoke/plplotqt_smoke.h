#ifndef QT_SMOKE_H
#define QT_SMOKE_H

#include <smoke.h>

extern "C" SMOKE_EXPORT Smoke* plplotqt_Smoke;
extern "C" SMOKE_EXPORT void init_plplotqt_Smoke();
extern "C" SMOKE_EXPORT void delete_plplotqt_Smoke();

#ifndef QGLOBALSPACE_CLASS
#define QGLOBALSPACE_CLASS
class QGlobalSpace { };
#endif

#endif
