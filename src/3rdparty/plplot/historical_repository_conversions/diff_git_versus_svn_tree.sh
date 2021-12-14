#!/bin/bash
# compare various revisions of svn trunk with corresponding git master
# branch commits, latest revisions of svn branches with corresponding
# git branches, and latest revisions of svn tags with corresponding
# git tags.

# Tailor these values (and only these values) in order to get this
# script to work for other situations.

# The testing of the master branch follows only first parent commits,
# i.e., it corresponds to the history of svn trunk commits that were
# converted to git master.  Normally, testing all these commits takes
# too long so the count_start and count_sample parameters below are
# used to sample these commits in chronological order.  The commits
# are counted with the count variable starting at 0 and are tested
# only when count-count_start >= 0 and mod(count-count_start,
# count_sample) == 0.
count_start=0
count_sample=1

# Relative or absolute path of complete (including branches, tags, and
# trunk) working directory checked out from the local subversion
# repository.
svn_working_dir=plplot_complete_local_svn

# Relative or absolute path of cloned (non-bare) local git repository.
# Later change this to the cloned SF git repository to check that repository
# as well.
git_working_dir=plplot_local_clone.git

# These temporary directories get repeatedly removed and reused.
svn_export_dir=svn_temporary_export
git_copy_dir=git_temporary_copy

# End of tailored values.

function compare_tree {
    # $1 is the subversion tree,
    # $2 is the revision of that tree,
    # $3 is the git tree, and
    # $4 is the git identity of the git commit we want to compare
    # with the subversion revision.
    echo "Compare trees given by revision = $2 of $1 with git ID = $4 of $3"

# Sanity checking.
    if [ ! -d $1 ] ; then
	echo "
This script is only designed to work when $1 (an svn working directory
corresponding to a checked out svn repository) exists as a relative or
absolute directory.  This condition has been violated.
"
	exit 1
    fi

    if [ ! -d $3 ] ; then
	echo "
This script is only designed to work when $3 (a git repository for
timephem with a working directory) exists as a relative or absolute
directory.  This condition has been violated.
"
	exit 1
    fi

    # Start comparison of two trees.
    svn update --quiet --revision=$2 $1
    rm -rf $svn_export_dir
    svn export --quiet --revision=$2 $1 $svn_export_dir
    rm -rf $git_copy_dir
    git --work-tree=$3 --git-dir=$3/.git checkout --quiet $4
    # For checkout-index, trailing slash for $prefix is important for some
    # reason according to the man page.  Furthermore, it turns out
    # that --prefix must be an absolute directory.
    case "$git_copy_dir" in
	/*)
	    # absolute $git_copy_dir
	    git --work-tree=$3 --git-dir=$3/.git checkout-index -a --prefix=$git_copy_dir/
	    ;;
	*)
	    # Must convert relative $git_copy_dir to absolute with $(pwd)/
	    git --work-tree=$3 --git-dir=$3/.git checkout-index -a --prefix=$(pwd)/$git_copy_dir/
	    ;;
    esac

    # Loop through all files with appropriate keywords.
    svn propget svn:keywords --recursive $1 |
    sed -e 's? - .*$??' -e "s?$1?$svn_export_dir?" |

    while read FILE; do
	sed -e 's?\$Author:[^$]*\$?$Author$?' -e 's?\$Date:[^$]*\$?$Date$?' -e 's?\$Id:[^$]*\$?$Id$?' -e 's?\$Revision:[^$]*\$?$Revision$?' < "$FILE" >| /tmp/temporary_file
        # This overwrites $FILE with edited result, but doesn't matter because
        # it is exported tree which is overwritten each time in any case.
	\mv /tmp/temporary_file "$FILE"

	# sed process git version of this FILE since some svn clients store
        # e.g., $Id:$ in repository rather than the more usual $Id$.  The
        # edited result is in $git_copy_dir which is a throwaway directory
        # in any case.
	FILE=${FILE/$svn_export_dir/$git_copy_dir}
	sed -e 's?\$Author:[^$]*\$?$Author$?' -e 's?\$Date:[^$]*\$?$Date$?' -e 's?\$Id:[^$]*\$?$Id$?' -e 's?\$Revision:[^$]*\$?$Revision$?' < "$FILE" >| /tmp/temporary_file
        # This overwrites $FILE with edited result, but doesn't matter because
        # it is exported tree which is overwritten each time in any case.
	\mv /tmp/temporary_file "$FILE"
    done
    diff -Naur $svn_export_dir $git_copy_dir
}

# compare svn trunk HEAD with corresponding git commit.
compare_tree $svn_working_dir/trunk HEAD $git_working_dir master

# compare the tips of various svn branches with corresponding git branch.

# Generate list of git branches corresponding to subversion branches, and
# compare those trees.
git --work-tree=$git_working_dir --git-dir=$git_working_dir/.git checkout --quiet master
git --work-tree=$git_working_dir --git-dir=$git_working_dir/.git branch --remotes |
sed -e "s?  origin/??" |
grep -v master |
while read branch; do
    compare_tree $svn_working_dir/branches/$branch HEAD $git_working_dir $branch
done

# compare the tips of various svn tags with corresponding git tag.

# Generate list of git tags corresponding to subversion tags, and
# compare those trees.
git --work-tree=$git_working_dir --git-dir=$git_working_dir/.git checkout --quiet master
git --work-tree=$git_working_dir --git-dir=$git_working_dir/.git tag|
grep -v backup |
while read tag; do
    compare_tree $svn_working_dir/tags/${tag/tilde/\~} HEAD $git_working_dir $tag
done

# compare various svn trunk revisions with corresponding git commit on the
# master branch.

# Initialize count for loop below.
count=$((0))

# Generate list of git commit ID's versus svn revision number from a
# git log of the master branch.
git --work-tree=$git_working_dir --git-dir=$git_working_dir/.git checkout --quiet master
# N.B. --first-parent means we follow just master branch with no merges from other branches.  For some "gitty" reason --no-merges does not do this.
# --reverse puts this log into chronological order.
git --work-tree=$git_working_dir --git-dir=$git_working_dir/.git log --name-status --first-parent --reverse master|
grep -E '(^commit | revision=)' |
sed -e 's?    svn path=/trunk/; ??' -e 's?^commit ??' -e 's?revision=??' |
while read commitId; do
    read revision
    mod_count=$((($count-$count_start)%$count_sample))
    if [ $mod_count == 0 ] ; then
	compare_tree $svn_working_dir/trunk $revision $git_working_dir $commitId
    fi
    count=$((($count+1)))
done
