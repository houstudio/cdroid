#ifndef __GFX_DRM_H__
#define __GFX_DRM_H__
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <vector>

class GFXDrm {
public:
    typedef struct {
        uint32_t width;
        uint32_t height;
        uint32_t crtc;
        drmModeModeInfo mode;
    } GFXMode;
    typedef struct {
        uint32_t width;
        uint32_t height;
        uint32_t size;
        uint32_t stride;
        uint32_t handle;
        uint32_t fb_id;
        uint8_t* map;
    } GFXFB;
private:
    int fd;
    uint32_t crtc_id;/*identifier for display/hdmi/display port ...*/
    uint32_t conn_id;
    drmModeRes*modeRes;
    drmModeConnector*modeConnector;
    std::vector<GFXMode>gfxModes;
private:
    int find_crtc(drmModeConnector *conn,GFXDrm::GFXMode&dev);
public:
    GFXDrm(const char*dev);
    ~GFXDrm();
    int fetchModes(std::vector<GFXMode>&);
    int create_fb(int width,int height,GFXDrm::GFXFB&fb);
};
#endif
