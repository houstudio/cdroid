#!/bin/bash

# Backup the PLplot wiki at SF in JSON form (which is the
# only form offered by the Allura API)

# Store all output files in /tmp/plplot_wiki_backup
# After this script is run you will likely want to
# cp /tmp/plplot_wiki_backup/wiki_backup.tar.gz to a
# more permanent location
mkdir -p /tmp/plplot_wiki_backup
cd /tmp/plplot_wiki_backup

# SF wiki page prefix name used by the Allura API
WIKI_PAGE_NAME=https://sourceforge.net/rest/p/plplot/wiki

# Get list of Wiki pages.
curl \
--verbose \
--cookie-jar curl_cookies1.txt \
${WIKI_PAGE_NAME} \
1>| backup_sf_wiki1.stdout \
2>| backup_sf_wiki1.stderr

# Create a blank-delimited list of Wiki page names.
page_list=$(cat backup_sf_wiki1.stdout)
# Strip off leading string
page_list=${page_list##*\{\"pages\": \[}
# Strip off trailing string
page_list=${page_list%%\]\}*}

# Strip off all quotes.  This assumes each element of the list
# includes no blanks which I believe is always going to be the
# case (since if you did try to create a page with a blank in
# the name, it would likely insert %20 for that blank).
page_list=${page_list//\"/}

# Drop commas from list.  The final result should be a blank-delimited
# list
page_list=${page_list//,/}

rm -rf wiki_backup
mkdir wiki_backup

for PAGE in ${page_list} ; do
    curl --silent --max-redirs 0 ${WIKI_PAGE_NAME}/$PAGE >|wiki_backup/$PAGE
    curl_rc=$?
    if [ "$curl_rc" != 0 ] ; then
	echo "Curl failed for <${WIKI_PAGE_NAME}/$PAGE>.... exiting"
	exit 1
    fi
done

# Create tarball of results
tar zcf wiki_backup.tar.gz wiki_backup