#!/bin/bash

# Set JOBS to suitable value. 16 is correct for hardware such as the
# Ryzen 7 1700 system with 8 cpu cores and twice that number of
# hardware threads.  But parallel builds are unreliable on Cygwin and
# MinGW-w64/MSYS2 so for those two platforms you should
# always set JOBS=1.
JOBS=16

# This script generates a tarball of source code and screenshots,
# 'htdocs_plot_examples.tgz', that will be part of the 'examples' web page,
#
# The original intention was to upload the generated tarball directly to the
# PLplot SourceForge website filesystem and unpack it there using ssh
# commands, but now that SF is no longer ssh accessible, the script
# has been generalized for any ssh-accessible site, which in turn can
# be used to update the SF website using rsync.

# This script must be run in the top plplot source directory as:
#
# WWW_USER=$USERNAME \
# WWW_GROUP=$GROUPNAME \
# WWW_HOST=$HOSTNAME \
# WWW_DIR=$WEBSITE_PREFIX \
# scripts/htdocs-gen_plot-examples.sh
#
# where $USERNAME, $GROUPNAME, $HOSTNAME, and $WEBSITE_PREFIX are the
# appropriate username, groupname, hostname, and website prefix directory
# of the ssh-accessible site.

# To avoid rebuild launch it like this:
# WWW_USER=$USERNAME \
# WWW_GROUP=$GROUPNAME \
# WWW_HOST=$HOSTNAME \
# WWW_DIR=$WEBSITE_PREFIX \
# build=false scripts/htdocs-gen_plot-examples.sh

# Sanity checks.
if [ -z "$WWW_USER" ] ; then
  echo "must specify non-empty WWW_USER environment variable"
  exit 1
fi

if [ -z "$WWW_GROUP" ] ; then
  echo "must specify non-empty WWW_GROUP environment variable"
  exit 1
fi

if [ -z "$WWW_HOST" ] ; then
  echo "must specify non-empty WWW_HOST environment variable"
  exit 1
fi

if [ -z "$WWW_DIR" ] ; then
  echo "must specify non-empty WWW_DIR environment variable"
  exit 1
fi
WWW_USER_AT_HOST=${WWW_USER}@$WWW_HOST

# cp should preserve time stamps to make rsync more efficient for
# website uploading to SourceForge.
CP='cp -p'

# a fresh make, with minimum bindings

build=${build:-true}

if [ "$build" = "true" ] ; then
    rm -rf htdocsgen/build_dir htdocsgen/install
    mkdir -p htdocsgen/build_dir htdocsgen/install
    cd htdocsgen/build_dir
    cmake -DCMAKE_INSTALL_PREFIX=`pwd`/../install \
	-DDEFAULT_NO_BINDINGS=ON \
    -DDEFAULT_NO_DEVICES=ON -DPLD_pngcairo=ON \
	../../
    make -j$JOBS
    make -j$JOBS install
    cd ../..
fi

EXDIR=htdocs/examples-data
pushd htdocsgen/install/share/plplot*/examples/c
export cexamples_dir=`pwd`
make -j$JOBS
popd

# hack, x20c needs Chloe in the current directory
$CP examples/Chloe.pgm .

for exe in 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 \
    23 24 25 26 27 28 29 30 31 32 33; do

    if [ $exe = "08" -o $exe = "16" -o $exe = "20" -o $exe = "30" ] ; then
      # The default cairo graphics AA looks good for these examples now
      # since Werner's filled_polygon change to the cairo driver so no need
      # to explicitly turn graphics AA off any more with
      # DRIVEROPT='-drvopt graphics_anti_aliasing=1'
	DRIVEROPT=
    elif [ $exe = "09" -o $exe = "21" ] ; then
      # Text clipping.
        DRIVEROPT='-drvopt text_clipping=1'
    elif [ $exe = "27" ] ; then
      # even-odd fill rule (a general option rather than a driver option)
      # generates more interesting results.
        DRIVEROPT='-eofill'
    else
      # Otherwise use default graphics AA which is full AA
        DRIVEROPT=
    fi
    echo Working on example ${exe} using DRIVEROPT of $DRIVEROPT

    # generate standard and preview size images
    if [ $exe = "14" ] ; then
      echo x14a.%n.png | \
      $cexamples_dir/x${exe}c -dev pngcairo -o x${exe}.%n.png \
          -fam -fflen 2 $DRIVEROPT -geometry 800x600
      echo prev-x14a.%n.png | \
      $cexamples_dir/x${exe}c -dev pngcairo -o prev-x${exe}.%n.png \
          -fam -fflen 2 $DRIVEROPT -geometry 200x150
      # First and second pages of master stream are already stored as first
      # and second pages of overall example.
      # Move first and second pages of slave stream to third and fourth
      # pages of overall example for easy web viewing.
      mv x14a.01.png x14.03.png
      mv x14a.02.png x14.04.png
      mv prev-x14a.01.png prev-x14.03.png
      mv prev-x14a.02.png prev-x14.04.png
    else
      $cexamples_dir/x${exe}c -dev pngcairo -o x${exe}.%n.png \
          -fam -fflen 2 $DRIVEROPT -geometry 800x600
      $cexamples_dir/x${exe}c -dev pngcairo -o prev-x${exe}.%n.png \
          -fam -fflen 2 $DRIVEROPT -geometry 200x150
    fi
    # move to www directory.
    echo populating www directory demo${exe}
    rm -rf $EXDIR/demo${exe}
    mkdir -p $EXDIR/demo${exe}
    # page number is two digits or more.
    mv *${exe}.*[0-9][0-9].png $EXDIR/demo${exe}
    for f in \
	examples/ada/xstandard${exe}a.adb \
	examples/ada/xtraditional${exe}a.adb \
        examples/c/x${exe}c.c \
	examples/c++/x${exe}.cc \
	examples/d/x${exe}d.d \
	examples/fortran/x${exe}f.f90 \
        examples/java/x${exe}.java \
        examples/lua/x${exe}.lua \
        examples/ocaml/x${exe}.ml \
        examples/octave/x${exe}c.m \
        examples/python/x${exe}.py \
        examples/tcl/x${exe}.tcl \
	; do
        if [ -f $f ] ; then
            $CP $f $EXDIR/demo${exe}
        else
            echo Example `basename $f` is not yet available \
                > $EXDIR/demo${exe}/`basename $f`
        fi
    done

    # rename executables, to avoid browsers trying to execute files
    # instead of showing them.
    (cd  $EXDIR/demo${exe};
	for j in \
	    *.adb \
            *.c \
	    *.cc \
	    *.d \
	    *.f90 \
	    *.java \
	    *.lua \
	    *.ml \
	    *.m \
	    *.py \
	    *.tcl \
	    ; do
	    mv $j $j.txt
	done
	)

done

# remove hack
rm -f Chloe.pgm

# create the tarball
TARBALL=htdocs-plot-demos.tgz
find htdocs -size 0 -exec rm {} \;
tar cvzf $TARBALL htdocs
rm -rf htdocs

# Transfer the tarball to $WWW_HOST and unpack it, such that the files will
# appear in the PLplot web site

echo "ssh $WWW_USER_AT_HOST mkdir -p $WWW_DIR"
ssh $WWW_USER_AT_HOST mkdir -p $WWW_DIR
echo "ssh $WWW_USER_AT_HOST rm -rf $WWW_DIR/$EXDIR"
ssh $WWW_USER_AT_HOST rm -rf $WWW_DIR/$EXDIR
echo "scp $TARBALL $WWW_USER_AT_HOST:$WWW_DIR"
scp $TARBALL $WWW_USER_AT_HOST:$WWW_DIR
echo "ssh $WWW_USER_AT_HOST chmod g=u $WWW_DIR/$TARBALL"
ssh $WWW_USER_AT_HOST chmod g=u $WWW_DIR/$TARBALL
echo "ssh $WWW_USER_AT_HOST tar -x -z -C $WWW_DIR -f $WWW_DIR/$TARBALL"
ssh $WWW_USER_AT_HOST tar -x -z -C $WWW_DIR -f $WWW_DIR/$TARBALL
echo "ssh $WWW_USER_AT_HOST chgrp -R $WWW_GROUP $WWW_DIR/$EXDIR"
ssh $WWW_USER_AT_HOST chgrp -R $WWW_GROUP $WWW_DIR/$EXDIR
echo "ssh $WWW_USER_AT_HOST chmod -R g=u $WWW_DIR/$EXDIR"
ssh $WWW_USER_AT_HOST chmod -R g=u $WWW_DIR/$EXDIR
echo "ssh $WWW_USER_AT_HOST rm -f $WWW_DIR/$TARBALL"
ssh $WWW_USER_AT_HOST rm -f $WWW_DIR/$TARBALL
