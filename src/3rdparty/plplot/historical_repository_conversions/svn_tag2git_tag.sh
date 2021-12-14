git branch |
# Remove spaces at beginning of line:
sed s/..// |
# Only get 'tag' branches:
grep ^tag-- |
# Strip down to just the tag name:
sed s/tag--// |
while read tagname; do
  git tag -a "$tagname" -m "Tag imported from SVN." "tag--$tagname" >/dev/null 2>/dev/null && echo "tagged: $tagname"
  git branch -D "tag--$tagname" >/dev/null 2>/dev/null && echo "deleted branch: tag--$tagname"
done
