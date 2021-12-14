// Maurice LeBrun
// 31-May-95
//
// Declarations for IP utility routines.
// Put here so as not to clutter up user codes.
//

#ifndef __TCPIP_H__
#define __TCPIP_H__

#include "pdf.h"
#include <tcl.h>

// Modified version of the "Tdp_PacketSend" command.

PLDLLIMPEXP_TCLTK int
pl_PacketSend( Tcl_Interp * interp, PLiodev * iodev, PDFstrm * pdfs );

// Modified version of the "Tdp_PacketReceive" command.

PLDLLIMPEXP_TCLTK int
pl_PacketReceive( Tcl_Interp * interp, PLiodev * iodev, PDFstrm * pdfs );

#endif  // __TCPIP_H__
