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


#ifndef __NGL_SMC_H__
#define __NGL_SMC_H__

#include "cdtypes.h"

BEGIN_DECLS

/**
 @ingroup std_drivers
 */

/**
 @defgroup scDriver NGL_SMC API
 @brief This section concentrates on how to reset a smartcard, as well as how to write to and read information from a smartcard.
 @{
 */

/**
 @defgroup scConst NGL_SMC Constants
 @brief The NGL_SMC constants

 The NGL_SMC driver uses a list of constants. These are in the va_errors.h header file:
 - @ref kVA_OK
 - @ref kVA_INVALID_PARAMETER
 - @ref kVA_RESOURCE_BUSY
 - @ref kVA_UNEXPECTED_CALLBACK
 - @ref kVA_NOT_IMPLEMENTED
 - @ref kVA_RESOURCE_ALREADY_LOCKED
 - @ref kVA_RESOURCE_NOT_LOCKED
 
 @{
 @}
 */


 
/**
 @defgroup scEnum NGL_SMC Enumerations
 @brief The NGL_SMC enumeration

 The NGL_SMC API driver contains the following enumeration:
 @{
 */

/** This enumeration specifies the smartcard slot state */
typedef enum
{
    eSMCCARD_EXTRACTED = 0, /**< The smartcard is extracted. */
    eSMCCARD_INSERTED,      /**< The smartcard is inserted but not reset. */
    eSMCCARD_READY          /**< The smartcard is inserted and reset. A command can be sent. */
} tNGL_SMC_State;

/** @} */


/**
 @defgroup manSCFunc NGL_SMC STB Manufacturer Functions
 @brief The NGL_SMC functions to be implemented by the STB manufacturer.

 @{
 The following section gives detailed information on the different NGL_SMC driver functions already listed above.@n
 All of these functions are coded by the STB manufacturer.@n
 The functions are defined in:@n
 `va_sc.h`
*/

/**
 This function resets a given smartcard.
 @param [in] dwScSlot Indicates the slot number of the smartcard. This value ranges from 0 to uiNbSmartCard--1.
 @param [in] uiPPSNegotiationValue This parameter is set with the PPS (Protocol Parameter Selection) negotiation
                                   value requested by the ACS.
                             <br> If this value is 0, or if the interface byte TA1 of the smartcard (described in the ISO/IEC 7816-3 document) is absent,
                             the PPS negotiation must not take place.
                             <br> On the other hand, if the ATR (Answer To Reset) specifies that the card permits negotiation,
							 the NGL_SMC driver proceeds with a PPS negotiation to request a change in the communication speed.
                             <br> The 4 MSB of `uiPPSNegotiationValue` indicates the factor F and the 4 LSB specifies the factor D,
							 similarly to the interface byte TA1.
                             <br> The STB manufacturer must fine tune the negotiated Fn value as close as possible to F (Fn < = F),
							 taking into account the limitations of the smartcard given by the ATR.
							 Likewise, the value Dn must be as close as possible to D (Dn < = D).
                             <br> Note that according to the ISO/IEC 7816-3 tables, it is the referenced values and
							 not the references that must be tuned.
							 For example, if the negotiation value D is 0x6 and the smartcard handles 0x1 and 0x8,
							 the negotiated value should be 0x8 (as the referenced value for 0x6 is greater than the referenced
							 value for 0x8).
 @retval kVA_OK                If the request is accepted.
 @retval kVA_INVALID_PARAMETER If the value of `dwScSlot` is out of range.
 @retval kVA_RESOURCE_BUSY     If the smartcard reader is already processing a reset command previously required by Viaccess-Orca.
 @retval kVA_INSERT_CARD_FIRST If no smartcard is present in the slot.

 @b Comments @n Once the reset process has been successfully carried out, the STB calls the Viaccess-Orca function @ref NGL_SMC_ResetDone.
      On the other hand, if an error occurs during the reset process, the STB calls @ref NGL_SMC_ResetFailed.
 @note The STB must call the @ref NGL_SMC_ResetFailed function when the PPS exchange between the driver and the smartcard fails.
 
 For related information, refer to @ref NGL_SMC_ResetDone and @ref NGL_SMC_ResetFailed.
*/
typedef void(*NGL_SMCRESET_NOTIFY)(DWORD slot,const BYTE*atr,INT atrlen,BYTE pps);
typedef void(*NGL_SMCSTATE_NOTIFY)(DWORD dwSlot,tNGL_SMC_State state,void*userdata);
typedef void(*NGL_SMCCOMMAND_NOTIFY)(DWORD slot,const BYTE*response,UINT size,void*userdata);
INT nglSmcReset(DWORD dwScSlot, UINT8 uiPPSNegotiationValue,NGL_SMCRESET_NOTIFY fn);
INT nglSmcRegisterNotify(DWORD dwScSlot,NGL_SMCSTATE_NOTIFY fn,void*userdata);

/**
 This function retrieves information on the state of a given smartcard.
 @param [in] dwScSlot Indicates the slot number of the smartcard. This value ranges from 0 to uiNbSmartCard--1.
 @param [in] pScState Points to the address of a @ref tNGL_SMC_State enumeration.
                      The memory is allocated by Viaccess-Orca, but the enumeration is updated by the STB.
 @retval kVA_OK                If the request is correctly accepted.
 @retval kVA_INVALID_PARAMETER the dwScSlot value is out of range or if pScState is NULL.

 For related information, refer to @ref tNGL_SMC_State
*/
INT nglSmcGetState(DWORD dwScSlot, tNGL_SMC_State *pScState);

/**
 This function sends a command to a given smartcard.
 @param [in] dwScSlot Indicates the slot number of the smartcard to be activated. This value runs from 0 to uiNbSmartCard--1.
 @param [in] uiCommandSize Specifies the size of the command and its associated data sent to the smartcard
 @param [in] pCommand Contains the command and associated data that is sent to the smartcard
 @retval kVA_OK If the request is accepted.
 @retval kVA_INVALID_PARAMETER If the dwScSlot value is out of range or if pCommand is NULL.
 @retval kVA_RESOURCE_BUSY     If the smartcard is already processing a Viaccess-Orca command.
 @retval kVA_INSERT_CARD_FIRST If no smartcard is inserted in the slot.
 @retval kVA_RESET_CARD_FIRST  If the smartcard is not reset.
 @b Comments @n If the command is accepted and is being processed any subsequent @ref NGL_SMC_SendCommand request is answered
       with a kVA_RESOURCE_BUSY and is ignored.
       @n Once the command is successfully processed, the STB calls the @ref NGL_SMC_CommandDone function.
	   However, if the command was not processed, the STB calls the @ref NGL_SMC_CommandFailed function.
       <br> As soon as the @ref NGL_SMC_CommandDone or the @ref NGL_SMC_CommandFailed function returns,
	   the smartcard can receive a new @ref NGL_SMC_SendCommand request.
       <br> If an error occurred (kVA_INVALID_PARAMETER or kVA_RESOURCE_BUSY), the STB ignores the send command request. 
	   In this case, the Viaccess-Orca ACS does not expect any @ref NGL_SMC_CommandDone or @ref NGL_SMC_CommandFailed response.
       <br> Viaccess-Orca allocates, fills and frees the memory block pointed by pCommand. 
	   If the return value is kVA_OK, Viaccess-Orca waits for one of the two callback functions 
	   @ref NGL_SMC_CommandDone or @ref NGL_SMC_CommandFailed before freeing the allocated memory. 
	   In all other cases the memory is freed immediately.

 For related Viaccess-Orca functions, refer to @ref NGL_SMC_CommandDone and @ref NGL_SMC_CommandFailed
*/
INT nglSmcRead(DWORD dwScSlot,BYTE *pData,UINT datasize);

INT nglSmcSendCommand(DWORD dwScSlot,BYTE*command,UINT cmdsize,NGL_SMCCOMMAND_NOTIFY fn,void*userdata);
INT nglSmcTransfer( DWORD dwScSlot,BYTE*command,UINT cmdsize,BYTE*response,UINT *responseSize);
/**
 This function activates a smartcard. It is used for ISO compatibility tests only.
 @param [in] dwScSlot Indicates the slot number of the smartcard to be deactivated. This value ranges from 0 to uiNbSmartCard--1.
 @retval kVA_OK                If the request is accepted.
 @retval kVA_INVALID_PARAMETER If dwScSlot is out of range.
 @retval kVA_NOT_IMPLEMENTED   If the request is not supported by the STB NGL_SMC driver.
 @retval kVA_INSERT_CARD_FIRST If no smartcard is inserted in the slot.
 @b Comments @n The activation process must follow the recommendations documented in ISO/IEC 7816-3.
 
 For related information, refer to @ref NGL_SMC_Deactivate
*/
INT nglSmcActivate(DWORD dwScSlot);


/**
 This function deactivates a smartcard. It is used for ISO compatibility tests only.
 @param [in] dwScSlot Indicates the slot number of the smartcard. This value ranges from 0 to uiNbSmartCard--1.
 @retval kVA_OK                If the request is accepted.
 @retval kVA_INVALID_PARAMETER If dwScSlot is out of range.
 @retval kVA_NOT_IMPLEMENTED   If the request is not supported by the STB NGL_SMC driver.
 @retval kVA_INSERT_CARD_FIRST If no smartcard is inserted in the slot.
 @b Comments @n The deactivation process must follow the recommendations documented in ISO/IEC 7816-3.

 For related information, refer to @ref NGL_SMC_Activate
*/
INT nglSmcDeactivate(DWORD dwScSlot);


/**
 This function locks a smartcard reader.
 @param [in] dwScSlot Indicates the slot number of the smartcard to be locked. This value ranges from 0 to uiNbSmartCard--1.
 @retval kVA_OK                      If the request is accepted.
 @retval kVA_INVALID_PARAMETER       If dwScSlot is out of range.
 @retval kVA_RESOURCE_ALREADY_LOCKED If the smartcard reader is already locked by Viaccess-Orca or by a concurrent software.
 @retval kVA_INSERT_CARD_FIRST       If no smartcard is inserted in the slot.
 @b Comments @n This function ensures that no other processes, like for example reset requests, gain access to the smartcard reader 
       while it is locked. This function is useful for processing a list of smartcard commands that must not be interrupted.

 For related information, refer to @ref NGL_SMC_Unlock
*/
INT nglSmcLock(DWORD dwScSlot);


/**
 This function unlocks a smartcard reader.
 @param [in] dwScSlot Indicates the slot number of the smartcard to be unlocked. This value ranges from 0 to uiNbSmartCard--1.
 @retval kVA_OK                  If the request is accepted.
 @retval kVA_INVALID_PARAMETER   If dwScSlot is out of range.
 @retval kVA_RESOURCE_NOT_LOCKED If the smartcard reader is already unlocked by Viaccess-Orca or by a concurrent software.
 @retval kVA_INSERT_CARD_FIRST   If no smartcard is inserted in the slot.
 
 For related information, refer to @ref NGL_SMC_Unlock
*/
INT nglSmcUnlock(DWORD dwScSlot);
	
/** @} */


/**
 @defgroup viaccessSCFunc NGL_SMC Viaccess-Orca Functions
 @brief The NGL_SMC functions to be implemented by Viaccess-Orca.

 @{
 The following section gives detailed information on the different NGL_SMC driver functions listed above.@n
 All of these functions are provided by Viaccess-Orca.@n
 The functions are defined in:@n
 `va_sc.h`
*/

/**
 The STB calls this function once a smartcard has been successfully reset, including an eventual PPS negotiation.
 @param [in] dwScSlot Indicates the slot number of the smartcard to be unlocked. This value ranges from 0 to uiNbSmartCard--1.
 @param [in] uiAtrSize Specifies the size of the ATR message sent by the smartcard. This value ranges between 2 and 33 bytes.
 @param [in] pAtr Buffer that contains the ATR information, including the initial character TS (see the ISO/IEC 7816-3 document).
 @param [in] uiPPSNegotiatedvalue Indicates the PPS negotiated value.
                                  This parameter specifies the communication speed set during the reset in the case of a PPS 
								  negotiation. uiPpsnegotiatedValue must be less than or equal to the uiPpsNegotiationValue 
								  given as a parameter of the @ref NGL_SMC_Reset function.
                                  The 4 MSB of uiPpsnegotiatedValue indicate the negotiated factor F and the 4 LSB indicate 
								  the negotiated factor D.
			@note This value is set to 0 only if uiPpsNegotiationValue is set to 0 in the @ref NGL_SMC_Reset function. 
			      Otherwise this value must reflect the actual communication speed.
 @retval kVA_OK                  If the callback is successful.
 @retval kVA_INVALID_PARAMETER   If any of the function parameters do not meet the above conditions.
 @retval kVA_UNEXPECTED_CALLBACK If Viaccess-Orca did not require a reset operation.

 @b Comments @n The pAtr argument points to a buffer that is allocated and filled by the STB. 
       This buffer can be freed as soon as the @ref NGL_SMC_ResetDone function returns.
       <br> For information on the related STB function, refer to @ref NGL_SMC_Reset.
       <br> After a call to @ref NGL_SMC_ResetDone and before it returns, 
	   Viaccess-Orca must be able to call the function that triggered this call without it returning @ref kVA_RESOURCE_BUSY. 
	   (This function is not called in the @ref NGL_SMC_ResetDone function.)
 
 For information on the related STB function, refer to @ref NGL_SMC_Reset
*/
INT NGL_SMC_ResetDone(DWORD dwScSlot, UINT8 uiAtrSize, BYTE *pAtr, UINT8 uiPPSNegotiatedvalue);


/**
 The STB calls this function when the reset of a smartcard fails.
 @param [in] dwScSlot Indicates the slot number of the smartcard to be unlocked. This value ranges from 0 to uiNbSmartCard--1.
 @retval kVA_OK                  If the callback is successful.
 @retval kVA_INVALID_PARAMETER   If dwScSlot is out of range.
 @retval kVA_UNEXPECTED_CALLBACK If Viaccess-Orca did not require a reset operation.

 @b Comments @n After a call to @ref NGL_SMC_ResetFailed and before it returns, Viaccess-Orca must be able to call the function that triggered 
       this call without it returning @ref kVA_RESOURCE_BUSY. (This function is not called in the @ref NGL_SMC_ResetFailed function.)

 For information on the related STB function, refer to @ref NGL_SMC_Reset
*/
INT NGL_SMC_ResetFailed(DWORD dwScSlot);


/**
 The STB calls this callback function when a smartcard is inserted in the smartcard slot.
 @param [in] dwScSlot Indicates the slot number of the smartcard to be unlocked. This value ranges from 0 to uiNbSmartCard--1. 
 @retval kVA_OK                  If the notification was successful.
 @retval kVA_INVALID_PARAMETER   If dwScSlot is out of range.
 @retval kVA_UNEXPECTED_CALLBACK If Viaccess-Orca did not require a reset operation.
 
 @b Comments @n After a call to the @ref NGL_SMC_CardInserted function and before it returns, Viaccess-Orca must be able to call 
       the @ref NGL_SMC_Reset function without it returning @ref kVA_INSERT_CARD_FIRST and the @ref NGL_SMC_GetState function 
	   must indicate that the smartcard is inserted. (@ref NGL_SMC_Reset and @ref NGL_SMC_GetState are not called in 
	   the @ref NGL_SMC_CardInserted function.)
 
 For related information, refer to: @ref NGL_SMC_CardExtracted, @ref NGL_SMC_Reset
*/
INT NGL_SMC_CardInserted(DWORD dwScSlot);

/**
 The STB calls this function when a smartcard is extracted.
 @param [in] dwScSlot Indicates the slot number of the smartcard to be unlocked. This value ranges from 0 to uiNbSmartCard--1. 
 @retval kVA_OK                If the notification was successful.
 @retval kVA_INVALID_PARAMETER If dwScSlot is out of range.
 
 @b Comments @n If a smartcard is extracted whilst a command is being processed, the STB calls the @ref NGL_SMC_CommandFailed function
       before calling the @ref NGL_SMC_CardExtracted function.
 
  For related information, refer to: @ref NGL_SMC_CardInserted
  
  After a call to the NGL_SMC_CardExtracted function and before it returns, Viaccess-Orca must be able to call 
  the @ref NGL_SMC_GetState function that must indicate that the card is extracted. 
  (@ref NGL_SMC_GetState is not called in the @ref NGL_SMC_CardExtracted function.)
*/
INT NGL_SMC_CardExtracted(DWORD dwScSlot);


/**
 This function is called to indicate that a given smartcard command has been successfully processed.
 @param [in] dwScSlot Indicates the slot number of the smartcard to be unlocked. This value ranges from 0 to uiNbSmartCard--1.
 @param [in] uiReturnSize Indicates the number of bytes returned by the smartcard.
 @param [in] pReturn Details the buffer that contains the bytes returned by the smartcard.
 @retval kVA_OK                  If the callback is successful.
 @retval kVA_INVALID_PARAMETER   If dwScSlot is out of range or if pReturn equals NULL.
 @retval kVA_UNEXPECTED_CALLBACK If Viaccess-Orca did not require a send command operation.
 
 @b Comments @n The memory block that pReturn points to is allocated and filled by the STB. It can be freed as soon as 
       the @ref NGL_SMC_CommandDone function returns.

 For information on the related STB function, refer to @ref NGL_SMC_SendCommand.

 After a call to @ref NGL_SMC_CommandDone and before it returns, Viaccess-Orca must be able to call the function that triggered 
       this call without it returning @ref kVA_RESOURCE_BUSY. (This function is not called in the @ref NGL_SMC_CommandDone function.)
*/
INT NGL_SMC_CommandDone(DWORD dwScSlot, UINT32 uiReturnSize, BYTE *pReturn);

/**
 This function is called to indicate that a smartcard command has failed due to a communication problem with
 the smartcard. Reasons for this can be either an extracted smartcard or a timeout occurrence during the
 processing of the smartcard command.
 @param [in] dwScSlot Indicates the slot number of the smartcard to be unlocked. This value ranges from 0 to uiNbSmartCard--1.  
 @retval kVA_OK                  If the callback is successful.
 @retval kVA_INVALID_PARAMETER   If dwScSlot is out of range.
 @retval kVA_UNEXPECTED_CALLBACK If Viaccess-Orca did not require a send command operation.
 
 For information on the related STB function, refer to @ref NGL_SMC_SendCommand.@n

 @b Comments @n After a call to @ref NGL_SMC_CommandFailed and before it returns, Viaccess-Orca must be able to call the function that triggered 
       this call without it returning @ref kVA_RESOURCE_BUSY. (This function is not called in the @ref NGL_SMC_CommandFailed function.)
*/
INT NGL_SMC_CommandFailed(DWORD dwScSlot);

/** @} */

/** @} */
END_DECLS

#endif /* _NGL_SMC_H_ */

