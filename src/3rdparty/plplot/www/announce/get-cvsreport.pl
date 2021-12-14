#!/usr/bin/perl -w

# Copyright (C) 2003  Rafael Laboissiere
# This script is in the public domain
#
# Run it like this: get-cvsreport.pl -a -f 2003-04-21 > report

undef $/;
open (CVSREPORT, "cvsreport " . join (" ", @ARGV) . "|");
$delim = "Commit from";

print reverse map { "$delim$_" } split (/$delim/, <CVSREPORT> . "\n\n" );

