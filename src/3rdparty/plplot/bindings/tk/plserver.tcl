#----------------------------------------------------------------------------
# PLPLOT TK/TCL graphics renderer menu procs
# Maurice LeBrun
# 26-Apr-1993
# IFS, University of Texas at Austin
#
# This file includes the main initialization, cleanup, and communication
# procs.  Those dealing with the plframe widget and "megawidget" are kept
# elsewhere.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# plserver_init
# Create the main window, menus, etc.
#----------------------------------------------------------------------------

proc plserver_init {} {
global file_menu_on  plserver_init_done

if { ! [info exists plserver_init_done] } {
set plserver_init_done 1

# Set up toplevel

    plstdwin .

# Create the window for the menu bar

# No File/Help menu! save plot area! Set in plconfig.tcl
if $file_menu_on then {
    frame .menu -relief raised -borderwidth 1
    pack append . .menu {top fillx}

#------------
# File menu
#------------

    menubutton .menu.file -text "File" -menu .menu.file.m -underline 0
    menu .menu.file.m

#    .menu.file.m add command \
#	-label "Open" \
#	-command {null_command "Open"} \
#	-underline 0

    .menu.file.m add command \
	-label "About..." \
	-command about \
	-underline 0

#    if { ! [ info exists child ] } then {
#	.menu.file.m add command \
#	    -label "Launch client..." \
#	    -command {null_command "Launch client..."} \
#	    -underline 0
#    }

    .menu.file.m add separator

    .menu.file.m add command \
	-label "Exit" \
	-command exit \
	-underline 0

    bind . <Control-x> exit

    pack append .menu .menu.file {left}

#--------------
# Debug menu
#--------------

    if {[info exists debug_on]} then {
	menubutton .menu.debug -text "Debug" -menu .menu.debug.m -underline 0
	menu .menu.debug.m

	.menu.debug.m add command \
	    -label "Execute TCL command" \
	    -command evalCmd \
	    -underline 0

	pack append .menu .menu.debug {left}
    }

#--------------
# Help menu
#--------------

    menubutton .menu.help -text "Help" -menu .menu.help.m -underline 0
    menu .menu.help.m

    .menu.help.m add command \
	-label "On Tcl/TK..." \
	-command help_tcltk \
	-underline 3

    .menu.help.m add command \
	-label "On GUI..." \
	-command help_gui \
	-underline 3

    .menu.help.m add command \
	-label "On Keys..." \
	-command help_keys \
	-underline 3

    pack append .menu .menu.help {right}


    bind . <Control-x> exit

# Set up for keyboard-based menu traversal

    tk_menuBar .menu .menu.file .menu.help
    tk_bindForTraversal . .menu

    global tk_version
    if {$tk_version < 4.0} then {
	focus default .
    }
}
}
# Set up initial link to client.

    plserver_link_init
}

#----------------------------------------------------------------------------
# client_cmd
#
# Send string to client.  Does it in the background and catches errors
# (if client is busy it won't respond).
#
# The first "after 1" ensures the command is issued in the server's
# background, so that we always continue processing events.  This is
# important for handshaking and good for performance.
#
# The second "after 1" (or the dp_RDO if Tcl-DP rpc is used) ensures that
# the client process doesn't try to send a reply.  Also good for
# performance but mainly to prevent problems if the server exits before
# the client can reply (so there is nothing to reply to).
#----------------------------------------------------------------------------

proc client_cmd {msg} {
    global dp client

    if { $dp } then {
	after 1 catch [list "dp_RDO [list $client] $msg"]
    } else {
#	after 1 catch [list "send [list $client] after 1 $msg"]
	after 1 catch [list "send -async [list $client] $msg"]
    }
}

#----------------------------------------------------------------------------
# plserver_link_init
#
# Set up link between client and server interpreters.
#----------------------------------------------------------------------------

proc plserver_link_init {} {
    global dp client

    if { $dp } then {
	global client_host client_port server_host server_port

	if { ! [ info exists client_host ] } then {
	    set client_host localhost
	}

	set server_host [host_id]
	set server_port [dp_MakeRPCServer]
	set client [dp_MakeRPCClient $client_host $client_port]

	dp_RDO [list $client] set server_host $server_host
	dp_RDO [list $client] set server_port $server_port
	dp_RDO [list $client] set client [list $client]

    } else {
	global client_name server_name

	set server_name [winfo name .]
	set client $client_name

	send $client "set server_name [list $server_name]"
	send $client "set client [list $client]"
    }
}

#----------------------------------------------------------------------------
# plserver_link_end
#
# Terminate link between client and server interpreters.
# Operates on the 1 client / 1 plserver principle for now.
#----------------------------------------------------------------------------

proc plserver_link_end {} {
    global dp client plclient_exiting

    if { [info exists client] } then {

    # Tell client we are exiting.

	if { $dp } then {
	    dp_RPC [list $client] set plserver_exiting 1
	} else {
	    send $client "set plserver_exiting 1"
	}

    # If the client isn't exiting, cause it to.

	if { ! [info exists plclient_exiting] } then {
	    if { $dp } then {
		dp_RPC [list $client] dp_after 1 abort
	    } else {
		send $client "after 1 abort"
	    }
	    wait_until {[info exists plclient_exiting]}
	}

    # Clean up socket communications if using Tcl-DP.

	if { $dp } then {
	    catch dp_CloseRPC [list $client]
	}
    }
}

#----------------------------------------------------------------------------
# plserver_start
#
# Startup proc when client is run independently.
#----------------------------------------------------------------------------

proc plserver_start {{use_dp 0}} {
    global dp client

    set dp $use_dp

    plserver_init

    if { $dp } then {
	global client_host client_port server_host server_port

	set server_host [host_id]
	set server_port [dp_MakeRPCServer]

	puts stderr "Please start client with flags: "
	puts stderr "  -server_host $server_host -server_port $server_port"

	wait_until {[info exists client_port]}
	set client "[dp_MakeRPCClient $client_host $client_port]"
	dp_RDO [list $client] set client [list $client]

    } else {
	global client_name server_name

	#puts stderr "Please start client with flags: "
	#puts stderr "  -server_name $server_name"
	tkwait variable client_name

	set client $client_name
	send $client "set client [list $client]"
    }
}
