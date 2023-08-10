#include <cdtypes.h>
#include <ngl_pvr.h>
#include <cdlog.h>
#include <ngl_timer.h>

static unsigned char*pvr_buffer=NULL;
typedef struct {
    char *path;
} NGLPVR;

DWORD nglPvrInit() {
    return 0;
}


DWORD nglPvrRecordOpen(const char*record_path,const NGLPVR_RECORD_PARAM*param) {
    return 0;
}

DWORD nglPvrRecordPause(DWORD handler) {
    return E_OK;
}

DWORD nglPvrRecordResume(DWORD handler) {
    return E_OK;
}

DWORD nglPvrRecordClose(DWORD handler) {
    return E_OK;
}

void nglGetPvrPath(DWORD handler,char*path) {
}

///////////////////////////////PVR PLAYER////////////////////////////

DWORD nglPvrPlayerOpen(const char*pvrpath) {
    return 0;
}

DWORD nglPvrPlayerPlay(DWORD handle) {
    return E_OK;
}

DWORD nglPvrPlayerStop(DWORD handle) {
    return E_OK;
}

DWORD nglPvrPlayerPause(DWORD handle) {
    return E_OK;
}

DWORD nglPvrPlayerClose(DWORD handle) {
    return E_OK;
}

