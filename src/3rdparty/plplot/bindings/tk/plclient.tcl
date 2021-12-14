#----------------------------------------------------------------------------
# PLPLOT TK/TCL graphics client procs
# Maurice LeBrun
# 14-Jan-1994
# IFS, University of Texas at Austin
#
# These are customizable (since autoloaded) initialization procs for the
# TK driver.  At present you should be very careful what you change,
# however, since this stuff is still evolving.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# plclient_init
#
# Initializes variables for plplot/TK/DP driver.
# These variables determine what plserver procs are invoked by the driver
# to do various jobs.  Specifically, they are:
#
# plserver_init_proc	proc to initialize plserver
# plw_create_proc	proc to create the plframe widget
# plw_start_proc	proc to manage the plframe widget
# plw_flash_proc	proc to handle end of page condition
# plw_end_proc		proc to detach the plframe widget from the client
#----------------------------------------------------------------------------

proc plclient_init {} {

    global plserver_init_proc plw_create_proc \
	plw_start_proc plw_end_proc \
	dp client_host client_port

# This initializes plserver

    set plserver_init_proc	plserver_init

# These govern the plframe and associated widgets

    set plw_create_proc		plw::create
    set plw_start_proc		plw::start
    set plw_end_proc 		plw::end

# Set up communications port

    if { $dp } {
	set client_host [host_id]
	set client_port [dp_MakeRPCServer]
    }
}

#----------------------------------------------------------------------------
# plclient_link_init
#
# Initializes link to plserver assuming plserver already exists.
# Basically this means just sending info on the communications channel.
#----------------------------------------------------------------------------

proc plclient_link_init {} {
    global dp server client

    if { $dp } {
	global client_host client_port

	dp_RDO $server set client_host $client_host
	dp_RDO $server set client_port $client_port

    } else {
	global client_name

	send [list $server] "after 1 {set client_name [list $client_name]}"
	send [list $server] "after 2 plserver_start"
    }

    wait_until {[info exists client]}
}

#----------------------------------------------------------------------------
# plclient_link_end
#
# Terminate link to plserver.
#
# Note: due to some apparent bogosity in Tcl 7.3 and/or Tcl-DP 3.2,
# Tcl_DeleteInterp() doesn't always provide all the necessary cleanup when
# exiting, and a filehandler event for the old interpreter is generated
# /after/ the old interpreter has been deleted.  When multiple interpreters
# are in use, this results in an attempt to use a hash table that has
# already been deleted, and a panic.  I tried to find the offending code
# while in the debugger but it looks like a tough one to crack.  Only seen
# when using multiple DP output streams (e.g. from x14c or pltcl).
#----------------------------------------------------------------------------

proc plclient_link_end {} {
    global dp server plserver_exiting plw_end_proc

    if { [info exists server] } then {

    # Tell server we are exiting.

	if { $dp } then {
	    dp_RPC $server set plclient_exiting 1
	} else {
	    send $server "set plclient_exiting 1"
	}

    # If the server isn't exiting, cause it to.

	if { ! [info exists plserver_exiting] } then {
	    global plwindow
	    if { $dp } then {
		dp_update
		dp_RPC $server after 1 $plw_end_proc $plwindow
	    } else {
		update
		send $server "after 1 plw::end $plwindow"
	    }
	    wait_until {[info exists plserver_exiting]}
	}

    # Clean up socket communications if using Tcl-DP.

	if { $dp } {
	    catch dp_CloseRPC $server
	    dp_update
	}
    }

# Destroy main window if there is one.

    if { ! $dp } then {
	destroy .
    }
}

#----------------------------------------------------------------------------
# plclient_dp_init
#
# Initializes dp data link.
# In addition, as this is the last client/server connection needed, I
# disable further connections.
#----------------------------------------------------------------------------

proc plclient_dp_init {} {
    global server server_host plwindow client data_port data_sock

    dp_RDO $server plw::dplink $plwindow $client

    wait_until {[info exists data_port]}
    set data_sock [lindex [dp_connect $server_host $data_port] 0]

    dp_Host -
}
