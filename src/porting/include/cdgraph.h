#ifndef __CDROID_GRAPH_H__
#define __CDROID_GRAPH_H__
#include <stdbool.h>
#include <stdint.h>
/**
 @ingroup std_drivers
 */

/**
 @defgroup graphDriver GRAPH API
 @brief This section describes the interface for Graph interface.

 @{
*/
#ifdef __cplusplus
extern "C" {
#endif
typedef void* GFXHANDLE;
/**
 @defgroup graphFormat PIXEL Format
 @brief This enum list the surface format .
 @{ */
typedef enum {
    GPF_UNKNOWN,
    GPF_ARGB4444,
    GPF_ARGB1555,
    GPF_RGB565,
    GPF_ARGB,
    GPF_ABGR,
    GPF_RGB32
}GFXPIXELFORMAT;
/** @} */
/**
 @defgroup graphStruct Structs
 @brief .
 @{ */
typedef struct _GFXPoint {
    int32_t x;
    int32_t y;
}GFXPoint;

typedef struct _GFXRect {
    int32_t x;
    int32_t y;
    uint32_t w;
    uint32_t h;
}GFXRect;

typedef struct _GFXCursorImage{
    uint32_t width;
    uint32_t height;
    uint32_t hotX;
    uint32_t hotY;
    uint32_t delay;
    uint32_t*pixels;
}GFXCursorImage;
/**}*/

/**
 @defgroup graphfunctions Graph Funtions
 @brief .
 @{ */


 /*This function init the graph dirver .*/

 /**
     @retval E_OK                            If init success.
     @retval E_ERROR                         If init failed.

     For more information refer to @ref nglCreateSurface.*/

int32_t GFXInit();
/**This function get the graph resolution
 *  @param [in]dispid displayid ,count as 0,1,2...
    @param [out]width                         The value return screen width in pixels.
    @param [out]height                        The value return screen height in pixels.
    @retval E_OK
    @retval E_ERROR
     For more information refer to @ref nglCreateSurface and @ref nglGetSurfaceInfo.
*/
/**GFXGetDisplayCount get display count>=1 */
int32_t GFXGetDisplayCount();

/**GFXGetDisplaySize return phisical display  size in no rotation*/
int32_t GFXGetDisplaySize(int dispid, uint32_t* width, uint32_t* height);
/**This function create an OSD Surface which we can used to draw sth.
    @param [out]surface                      The value used to return surface handle.
    @param [in]width                         The value give the surface width in pixels.
    @param [in]height                        The value give the surface height in pixels.
    @param [in]format                        surface format @ref NGLPIXELFORMAT
    @param [in]hwsurface
    @retval E_OK
    @retval E_ERROR
    For more information refer to @ref nglCreateSurface and @ref nglGetSurfaceInfo.
*/

int32_t GFXCreateSurface(int dispid, GFXHANDLE* surface, uint32_t width, uint32_t height, int32_t format, bool hwsurface);

/**This function create an OSD Surface which we can used to draw sth.
    @param [in]surface                       The surface handle which is created by @ref nglCreateSurface.
    @param [out]width                         The value return screen width in pixels.
    @param [out]height                        The value return screen height in pixels.
    @param [out]format                        The value used to return surface pixel's format @ref NGLPIXELFORMAT
    @retval E_OK
    @retval E_ERROR
    For more information refer to @ref nglCreateSurface and @ref nglGetSurfaceInfo.
*/

int32_t GFXGetSurfaceInfo(GFXHANDLE surface, uint32_t* width, uint32_t* height, int32_t* format);
int32_t GFXLockSurface(GFXHANDLE surface, void** buffer, uint32_t* pitch);
int32_t GFXUnlockSurface(GFXHANDLE surface);
int32_t GFXSurfaceSetOpacity(GFXHANDLE surface, uint8_t alpha);
/**Thie function fill the surface area with color
  @param [in]surface
  @param [in]rect            if rect is NULL fill whole surface area
  @param [in]color           color with format as A8R8G8B8
  @retval E_OK
  @retval E_ERROR
*/
int32_t GFXFillRect(GFXHANDLE dstsurface, const GFXRect* rect, uint32_t color);

/**This function Blit source surface to dest surface .
    @param [in]dstsurface                     The dest surface which used to blit to.
    @param [in]dx                             The position x which source surface blit to
    @param [in]dy                             The position y which source surface blit to
    @param [in]srcsurface                     The source surface
    @param [in]srcrect                        The rectange(area) in source surface we want to blit
    @retval E_OK
    @retval E_ERROR
    For more information refer to @ref nglCreateSurface .
*/
int32_t GFXBlit(GFXHANDLE dstsurface, int dx, int dy, GFXHANDLE srcsurface, const GFXRect* srcrect);
int32_t GFXBatchBlit(GFXHANDLE dstsurface, const GFXPoint* dest_point, GFXHANDLE srcsurface, const GFXRect* srcrects);
int32_t GFXFlip(GFXHANDLE dstsurface);

/**This functionDestroy the surface
    @param [in]dstsurface                     The dest surface we want to destroied
    @retval E_OK
    @retval E_ERROR
    For more information refer to @ref nglCreateSurface
*/
int32_t GFXDestroySurface(GFXHANDLE surface);

GFXHANDLE GFXCreateCursor(const GFXCursorImage*);
/*
 * cursorHandle Cursor to attached to Display
 * cursorHandle =NULL(0) for Cursor dettach
 * NULL cursorHandle also used for hide cursor
 * */
void GFXAttachCursor(GFXHANDLE cursorHandle);
void GFXMoveCursor(int32_t xPos,int32_t yPos);
void GFXDestroyCursor(GFXHANDLE);

/**}*///raphfunctions

#ifdef __cplusplus
}
#endif
#endif

