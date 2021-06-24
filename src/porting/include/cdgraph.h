#ifndef __NGL_GRAPH_H__
#define __NGL_GRAPH_H__
#include <cdtypes.h>

/**
 @ingroup std_drivers
 */

/**
 @defgroup graphDriver GRAPH API
 @brief This section describes the interface for Graph interface.

 @{
*/


BEGIN_DECLS
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

typedef struct _GFXRect{
    INT x;
    INT y;
    UINT w;
    UINT h;
}GFXRect;
/**}*/

/**
 @defgroup graphfunctions Graph Funtions
 @brief .
 @{ */


/*This function init the graph dirver .

/**
    @retval E_OK                            If init success.
    @retval E_ERROR                         If init failed.

    For more information refer to @ref nglCreateSurface.*/

DWORD GFXInit();
/**This function get the graph resolution 
    @param [out]width                         The value return screen width in pixels.
    @param [out]height                        The value return screen height in pixels.
    @retval E_OK                            
    @retval E_ERROR
     For more information refer to @ref nglCreateSurface and @ref nglGetSurfaceInfo.
*/
DWORD GFXGetScreenSize(UINT*width,UINT*height);

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

DWORD GFXCreateSurface(HANDLE*surface,UINT width,UINT height,INT format,BOOL hwsurface);

/**This function create an OSD Surface which we can used to draw sth.
    @param [in]surface                       The surface handle which is created by @ref nglCreateSurface.
    @param [out]width                         The value return screen width in pixels.
    @param [out]height                        The value return screen height in pixels.
    @param [out]format                        The value used to return surface pixel's format @ref NGLPIXELFORMAT
    @retval E_OK
    @retval E_ERROR
    For more information refer to @ref nglCreateSurface and @ref nglGetSurfaceInfo.
*/

DWORD GFXGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format);
DWORD GFXLockSurface(HANDLE surface,void**buffer,UINT*pitch);
DWORD GFXUnlockSurface(HANDLE surface);
DWORD GFXSurfaceSetOpacity(HANDLE surface,BYTE alpha);
/**Thie function fill the surface area with color 
  @param [in]surface         
  @param [in]rect            if rect is NULL fill whole surface area
  @param [in]color           color with format as A8R8G8B8
  @retval E_OK
  @retval E_ERROR
*/
DWORD GFXFillRect(HANDLE dstsurface,const GFXRect*rect,UINT color);

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
DWORD GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect*srcrect);
DWORD GFXFlip(HANDLE dstsurface);

/**This functionDestroy the surface
    @param [in]dstsurface                     The dest surface we want to destroied
    @retval E_OK
    @retval E_ERROR
    For more information refer to @ref nglCreateSurface
*/
DWORD GFXDestroySurface(HANDLE surface);

/**}*///raphfunctions

END_DECLS
#endif

