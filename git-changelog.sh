#!/bin/sh
# use it as: ./git-changelog.sh --since="2009-06-28" > ChangeLog.new
git log --date=short --pretty="format:%cd%x09%an <%ae>%n%n%x09* %s%n" $1 $2 $3 $4
# EOF
