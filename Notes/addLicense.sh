#!/bin/bash

if [ $# -ne 1 ]; then
	echo "Usage: $0 <a file containing a list of filenames and types>"
	exit 127
fi

LICENSE="`dirname $0`/../LICENSE"
DELIMC='//'
DELIMCST='/*'
DELIMCEND='*/'
DELIMSH='#'

# file format:
# <file type>:<filename>

while read line; do
	t=${line%%:*}
	f=${line#*:}
	case $t in
		c | cc | cpp | h | hpp | java )  # small cases use '//'
			"`dirname $0`/addLicenseOneFile.py" "$LICENSE" "$f" "$DELIMC"
			;;
		[cC] | [cC][cC] | [cC][pP][pP] | [hH] | [hH][pP][pP] | [jJ][aA][vV][aA] )  # other cases use '/*' and '*/'
			"`dirname $0`/addLicenseOneFile.py" "$LICENSE" "$f" "$DELIMCST" "$DELIMCEND"
			;;
		[sS][hH] | [pP][yY] | * )
			# need special handling of the first line starting with #!
			"`dirname $0`/addLicenseOneShell.py" "$LICENSE" "$f" "$DELIMSH"
			;;
	esac
done < "$1"

