#!/bin/sh
# backup http://www.miscdebris.net/plplot_wiki to a tarball.

# Directory where results are temporarily stored.  This directory is
# destroyed both at the start and at the end.  Also the tarball
# $DIR.tar.gz is overwritten.  So be careful what you
# specify here.
DIR=plplot_wiki

rm -rf $DIR
# Need to create the directory first if logging to a file in it.
mkdir  $DIR

# Recurse up to 5 levels deep, don't get any html above
# plplot_wiki, get the files necessary to display all pages,
# Do not download *.tar.gz files (because these plplot tarballs are
# huge and dated),
# convert links to the locally downloaded version, windows==> escape a long
# list of characters (including question mark) from file names,
# start local hierarchy at $DIR, and keep a log of the transaction in
# $DIR/plplot_wiki_backup.log
wget --recursive --level=5 --no-parent \
--page-requisites --convert-links \
--restrict-file-names=windows \
--reject='*.tar.gz' \
--no-host-directories --cut-dirs=1 \
--directory-prefix=$DIR \
-o $DIR/plplot_wiki_backup.log \
http://www.miscdebris.net/plplot_wiki
# Create tarball and remove directory tree result
tar zcf $DIR.tar.gz $DIR
rm -rf $DIR
