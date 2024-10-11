/******************************************************************************
 
    Copyright
    This code is strictly confidential and the receiver is obliged to use it 
    exclusively for his or her own purposes. No part of Viaccess code may be 
    reproduced or transmitted in any form or by any means, electronic or 
    mechanical, including photocopying, recording, or by any information storage 
    and retrieval system, without permission in writing from Viaccess. 
    The information in this code is subject to change without notice. Viaccess 
    does not warrant that this code is error free. If you find any problems 
    with this code or wish to make comments, please report them to Viaccess.
 
    Trademarks 
    Viaccess is a registered trademark of Viaccess ® in France and/or other 
    countries. All other product and company names mentioned herein are the 
    trademarks of their respective owners.
    Viaccess may hold patents, patent applications, trademarks, copyrights 
    or other intellectual property rights over the code hereafter. Unless 
    expressly specified otherwise in a Viaccess written license agreement, the
    delivery of this code does not imply the concession of any license over 
    these patents, trademarks, copyrights or other intellectual property.
 
******************************************************************************/

#ifndef _NGL_DMX_H_
#define _NGL_DMX_H_

#include "cdtypes.h"

BEGIN_DECLS
/**
 @ingroup std_drivers
 */

/**
 @defgroup demuxDriver DMX API
 @brief This section describes the interface for demux resources.

 Viaccess-Orca uses these resources to set filters for EMM and ECM monitoring.
 @{
*/

/**
 @defgroup demuxConst DMX Constants
 @brief The DMX driver uses a list of constants.
     
    In the `ngl_errors.h` header file:
    - @ref E_OK
    - @ref E_INVALID_PARA

    @{
*/

/** Maximum size of the filter */
#define MAX_FILTERS 64
#define MAX_FILTER_DEPTH 16
/** @} */

/**
 @defgroup demuxEnum DMX Type Enumerations
 @brief The DMX driver uses one enumeration.
 @{
*/

/**
    According to the provided notification mode, the STB must notify Viaccess-Orca about a matching 
    section only once (for one-shot notifications) or continuously (as long as the filter criteria 
    match the section and the filter is not stopped).
*/
typedef enum{
    DMX_TS,     /**< The STB must long as the filter criteria match*/
    DMX_SECTION,/*                         the section and the filter is not stopped. */
    DMX_PES    /**< The STB must  about a matching section only once. */
} DMX_TYPE;

/** @} */

/**
 @defgroup FILTER_NOTIFY DMX Callback Functions
 @brief The DMX driver uses one callback function.
 @{
*/

/**
    The STB calls this callback function type to notify Viaccess-Orca that a section matches the filter 
    criteria. The buffer that contains the section is specified in the `pBuffer` parameter. The STB 
    only notifies Viaccess-Orca about the sections that are error-free and pass the Cyclic Redundancy 
    Codes (CRC) check, if available.

    @param [in] dwVaFilterHandle    The filter handle that the STB is provided with when 
                                    the filter is allocated. Refer to @ref nglAllocateSectionFilter for details.
    @param [in] uiBufferLength      This value gives the length in bytes of the buffer that 
                                    contains the filtered section.
    @param [in] pBuffer             The address of the section buffer. The first byte must be the 
                                    table identifier of the section. It may be freed when this 
                                    function returns.
    @param [in] pUserData           Pointer to a STB driver user data which is set by the STB 
                                    manufacturer. The value pointed to by this parameter is not 
                                    used by the dmx driver. when data arrived the value is passed
                                    to FILTER_NOTIFY setted by @ref nglAllocateSectionFilter.
                                    a parameter of the @ref VA_DSCR_SetKeys function.
    
    @retval E_OK                  If the notification is taken into account.
    @retval E_INVALID_PARA   If the `dwVaFilterHandle` parameter does not refer to an 
                                    allocated filter.
    
    For related information refer to @ref nglAllocateSectionFilter @ref nglAllocateSectionFilter.
*/
typedef void (*FILTER_NOTIFY)(HANDLE dwVaFilterHandle,const BYTE *pBuffer ,UINT uiBufferLength ,void *pUserData );

DWORD nglDmxInit();
DWORD nglDmxUninit();
/** @} */

/**
 @defgroup manDemuxFunc DMX STB Manufacturer Functions
 @brief The following section details the functions that are coded by the STB manufacturer.
 @{ 
*/

/**
    This function allocates a filter to a PID.

    @param [in] dmx_id                        The value of this parameter is between 0 and `uiNbAcs`-1. 
                                               The `uiNbAcs` value is the number of ACS required by the 
                                               STB at initialization stage. For information on 
                                               initialization, refer to @ref ctrlAPI.
                                               @n The `dwAcsId` parameter indicates the ACS (demux) 
                                               instance the filter is allocated to.
    @param [in] dwVaFilterHandle               The Viaccess-Orca ACS uses this value to identify a filter 
                                               instance. It corresponds on a one to one basis to the 
                                               `dwStbFilterHandle` value returned by this function. 
                                               The STB manufacturer must store and keep the 
                                               `dwVaFilterHandle` value whilst the filter is allocated. 
                                               The STB uses this value to notify the Viaccess-Orca ACS that a 
                                               section matches that given filter criteria.
    @param [in] wPid                           This value gives the PID of the demux channel to which the 
                                               filter is allocated. Viaccess-Orca may allocate several filters 
                                               with the same PID.
    @param [in] filterCallback                  This is a pointer of callback function. The STB calls this 
                                               function to notify  that a section matches the 
                                               filter criteria. The buffer that contains the section is 
                                               specified in the `pBuffer` parameter. The STB only notifies 
                                               Viaccess-Orca about the sections that are error-free and pass the 
                                               Cyclic Redundancy Codes (CRC) check, if available. Note also 
                                               that for a matching section the STB calls this notification 
                                               function according to the specified notification mode (the 
                                               @ref tDMX_NotificationMode parameter passed in the function 
                                               @ref DMX_SetSectionFilterParameters): only once or continuously.
    
    @retval NGL_INVALID_HANDLE                 If the section filters allocation failed.
    @retval dwStbFilterHandle                  It must be unique. This means that two filters can never have the 
                                               same handle value even if they belong to two different ACS instances.
 
    The Viaccess-Orca ACS requires a @ref kVA_SETUP_NBMAX_SECTIONFILTERS` filter per ACS instance.
    
    For more information refer to @ref nglFreeSectionFilter.
*/
HANDLE nglAllocateSectionFilter(INT dmx_id,WORD  wPid, FILTER_NOTIFY filterCallback,void*userdata,DMX_TYPE dmxtp);

/**
    This function frees a previously allocated filter.

    @param [in] dwStbFilterHandle   STB manufacturer filter handle that is returned by the 
                                    @ref DMX_AllocateSectionFilter function. It uniquely 
                                    identifies the filter to be freed.
    
    @retval E_OK                  If the filter is successfully freed.
    @retval E_INVALID_PARA        If the `dwStbFilterHandle` value does not correspond to an 
                                    allocated filter.
    
    For related information, refer to: @ref nglAllocateSectionFilter and @ref nglStopSectionFilter.
*/
INT nglFreeSectionFilter( HANDLE dwStbFilterHandle );

/**
    This function sets the value and the mask of a given filter.

    @param [in] dwStbFilterHandle  STB filter handle that is returned by the 
                                   @ref nglAllocateSectionFilter function. It identifies the 
                                   filter to be set.
    @param [in] uiLength           Indicates the size of the filter value and filter mask in bytes. 
                                   This value ranges from 0 to @ref kDMX_MAX_FILTER_SIZE` (the 
                                   default maximum value is 8 bytes, unless otherwise specified 
                                   by Viaccess). If `uiLength` is set to 0, the filtering is only 
                                   carried out on the PID of the section.
    @param [in] pValue             This address contains the first byte of the filter value. If the 
                                   `uiLength` value is greater than 0, the `pValue` cannot be NULL. 
                                   If `uiLength` is set to 0, `pValue` does not have to be set.
    @param [in] pMask              This address contains the first byte of the filter mask. If the 
                                   `uiLength` value is greater than 0, `pMask` cannot be NULL. If 
                                   `uiLength` is set to 0, `pMask` does not have to be set.
    
    @retval E_OK                 If the filter is successfully set.
    @retval E_INVALID_PARA      If the `dwStbFilterHandle` value does not correspond to an 
                                   allocated filter or if the `uiLength`, `pValue` or `pMask` does 
                                   not match the conditions detailed above.
    
    @note The `nglSetSectionFilterParameters` function does not automatically start the filtering 
    process. If the filter is already running, the STB must handle this dynamic situation without 
    interruption.
    @n Viaccess-Orca allocates and frees the memory blocks pointed to by `pValue` and `pMask`.

    For related information refer to: @ref nglStartSectionFilter and @ref nglStopSectionFilter.
*/
INT nglSetSectionFilterParameters(HANDLE dwStbFilterHandle,BYTE *pMask, BYTE *pValue, UINT uiLength);

INT nglFilterSetCRC(HANDLE hfilter,BOOL enable);/*CRC default is enabled*/
/**
    This function starts a given filtering.

    @param [in] dwStbFilterHandle           The STB filter handle that is returned by the 
                                            @ref nglAllocateSectionFilter function. It 
                                            identifies the filter to be started.

    @retval E_OK                          If the filter is successfully started.
    @retval E_INVALID_PARA               If the `dwStbFilterHandle` does not correspond to an 
                                            allocated filter.
    @retval kVA_SET_FILTER_PARAMETERS_FIRST If the filter parameters have not been set since the 
                                            filter allocation. In this case, Viaccess-Orca must call the 
                                            @ref nglSetSectionFilterParameters function at least once.
    
    @note As soon as a filter is started, the STB must notify the Viaccess-Orca ACS about any section 
    that matches the filter criteria using the callback function type 
    @ref FILTER_NOTIFY. The STB calls this notification function according to the 
    specified `eNotificationMode` (in the function @ref nglSetSectionFilterParameters): only 
    once or continuously.
    @n If the filter is already started the function returns the @ref E_OK` value.

    For related information refer to: @ref nglStopSectionFilter, @ref  FILTER_NOTIFY.
    and @ref nglSetSectionFilterParameters.
*/
INT nglStartSectionFilter(HANDLE  dwStbFilterHandle);

/**
    This function stops a given filtering.

    @param [in] dwStbFilterHandle           The STB filter handle that is returned by the 
                                            @ref nglAllocateSectionFilter function. It 
                                            identifies the filter to be stopped.

    @retval E_OK                          If the filter is successfully stopped.
    @retval E_INVALID_PARAM           If the `dwStbFilterHandle` does not correspond to an 
                                            allocated filter.
    
    @note If the filter is already stopped, the function just returns the @ref E_OK` value.

    For related information refer to @ref nglStartSectionFilter.
*/
INT nglStopSectionFilter(HANDLE  dwStbFilterHandle);
INT nglGetFilterPid(HANDLE  dwStbFilterHandle);
INT nglSetSectionFilterOneshot(HANDLE dwStbFilterHandle,BOOL onshort);
/** @} */
/** @} */
END_DECLS

#endif /* _NGL_DMX_H_ */

