#include <sys/mman.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gfxdrm.h>
#include <cdtypes.h>
#include <cdlog.h>

GFXDrm::GFXDrm(const char*devnode) {
    int i,ret;
    uint64_t has_dumb;
    drmModeRes *res;
    drmModeConnector *conn;
    struct modeset_dev *dev;
    fd=open(devnode, O_RDWR | O_CLOEXEC);
    if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || !has_dumb) {
        LOGE("drm device  does not support dumb buffers");
    }
    modeRes = drmModeGetResources(fd);
}

GFXDrm::~GFXDrm() {
    /* free resources again */
    drmModeFreeResources(modeRes);
}

int GFXDrm::fetchModes(std::vector<GFXDrm::GFXMode>&modes) {
    for (int i = 0; i < modeRes->count_connectors; i++) {
        drmModeConnector*conn=drmModeGetConnector(fd, modeRes->connectors[i]);
        if((conn->connection != DRM_MODE_CONNECTED)||(conn->count_modes == 0)) {
            LOGD("ignoring unused connector %u",conn->connector_id);
            continue;
        }
        for(int j=0; j<conn->count_modes; j++) {
            GFXMode md;
            md.width=conn->modes[j].hdisplay;
            md.height=conn->modes[j].vdisplay;
            md.mode=conn->modes[j];
            find_crtc(conn,md);
            gfxModes.push_back(md);
        }
    }
    modes=gfxModes;
    return gfxModes.size();
}


int GFXDrm::find_crtc(drmModeConnector *conn,GFXDrm::GFXMode&dev) {
    drmModeEncoder *enc;
    unsigned int i, j;
    int32_t crtc;
    struct modeset_dev *iter;

    /* first try the currently conected encoder+crtc */
    if (conn->encoder_id)
        enc = drmModeGetEncoder(fd, conn->encoder_id);
    else
        enc = NULL;

    if (enc) {
        if (enc->crtc_id) {
            crtc = enc->crtc_id;
            for (auto m:gfxModes) {
                if (m.crtc == crtc) {
                    crtc = -1;
                    break;
                }
            }
            if (crtc >= 0) {
                drmModeFreeEncoder(enc);
                dev.crtc = crtc;
                return 0;
            }
        }
        drmModeFreeEncoder(enc);
    }
    /* If the connector is not currently bound to an encoder or if the
     * encoder+crtc is already used by another connector (actually unlikely
     * but lets be safe), iterate all other available encoders to find a
     * matching CRTC. */
    for (i = 0; i < conn->count_encoders; ++i) {
        enc = drmModeGetEncoder(fd, conn->encoders[i]);
        if (!enc) {
            LOGE("cannot retrieve encoder %u:%u (%d): %m",i, conn->encoders[i], errno);
            continue;
        }
        /* iterate all global CRTCs */
        for (j = 0; j < modeRes->count_crtcs; ++j) {
            /* check whether this CRTC works with the encoder */
            if (!(enc->possible_crtcs & (1 << j)))
                continue;
            /* check that no other device already uses this CRTC */
            crtc = modeRes->crtcs[j];
            for (auto m:gfxModes) {
                if (m.crtc == crtc) {
                    crtc = -1;
                    break;
                }
            }
            /* we have found a CRTC, so save it and return */
            if (crtc >= 0) {
                drmModeFreeEncoder(enc);
                dev.crtc = crtc;
                return 0;
            }
        }
        drmModeFreeEncoder(enc);
    }
    LOGD("cannot find suitable CRTC for connector %u",conn->connector_id);
    return -ENOENT;
}

int GFXDrm::create_fb(int width,int height,GFXDrm::GFXFB&fb) {
    struct drm_mode_create_dumb creq;
    struct drm_mode_destroy_dumb dreq;
    struct drm_mode_map_dumb mreq;
    int ret;

    /* create dumb buffer */
    memset(&creq, 0, sizeof(creq));
    creq.width = width;
    creq.height= height;
    creq.bpp = 32;
    ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
    if (ret < 0) {
        LOGE("cannot create dumb buffer (%d): %m",errno);
        return -errno;
    }
    fb.stride = creq.pitch;
    fb.size = creq.size;
    fb.handle = creq.handle;

    /* create framebuffer object for the dumb-buffer */
    ret = drmModeAddFB(fd, width, height, 24, 32, fb.stride,fb.handle, &fb.fb);
    if (ret) {
        LOGE("cannot create framebuffer (%d): %m",errno);
        ret = -errno;
        goto err_destroy;
    }
    /* prepare buffer for memory mapping */
    memset(&mreq, 0, sizeof(mreq));
    mreq.handle = fb.handle;
    ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
    if (ret) {
        LOGE("cannot map dumb buffer (%d): %m",errno);
        ret = -errno;
        goto err_fb;
    }
    /* perform actual memory mapping */
    fb.map = (uint8_t*)mmap(0, fb.size, PROT_READ | PROT_WRITE, MAP_SHARED,fd, mreq.offset);
    if (fb.map == MAP_FAILED) {
        LOGE("cannot mmap dumb buffer (%d): %m",errno);
        ret = -errno;
        goto err_fb;
    }
    /* clear the framebuffer to 0 */
    memset(fb.map, 0, fb.size);
    return 0;

err_fb:
    drmModeRmFB(fd, fb.fb);
err_destroy:
    memset(&dreq, 0, sizeof(dreq));
    dreq.handle = fb.handle;
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
    return ret;
}
