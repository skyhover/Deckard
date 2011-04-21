#!/bin/bash

echo "DECKARD DEBUG--A Tree-Based Code Clone-Related Bug Detection Tool. Version 1.2.2"
echo "Copyright (c) 2007-2011. University of California"
echo "Distributed under the three-clause BSD license."
echo

echo -n "==== Configuration checking..."
. `dirname $0`/../clonedetect/configure
errcode=$?
if [[ $errcode -eq 0 ]]; then
	echo "Done."
	echo
else
	exit $errcode
fi

TOOVERWRITE=
if [[ $# -ge 1 ]]; then
	case "$1" in
		clean )
			TOOVERWRITE="clean"
			;;
		clean_all )
			TOOVERWRITE="clean_all"
		       	;;
		overwrite )
			TOOVERWRITE="overwrite"
			;;
		* )
			echo "Usage: $0 [overwrite | clean | clean_all]"
			exit 1
			;;
	esac
fi


echo "==== Start clone-related bug detection ====" 
echo

SRCLANG=${FILE_PATTERN##*.}
case $SRCLANG in
	c | h | java | php )
		;;
	*)
		echo "Error: language '$SRCLANG' not supported."
		exit 1 ;;
esac

# the actual commands for bug finding:
find "$CLUSTER_DIR" -type f -name "post_cluster_vdb_*_*_allg_*_*" | while read cdb;
do
	basecdb=`basename "$cdb"`
	basecdb=${basecdb#post_}
	if [[ "$TOOVERWRITE" = "clean" || "$TOOVERWRITE" = "clean_all" ]];
	then
		echo "Removing bug report \"$CLUSTER_DIR/bug_$basecdb\" if any ..."
		rm -rf $CLUSTER_DIR/bug_$basecdb $CLUSTER_DIR/bug_$basecdb.html
	elif [[ "$TOOVERWRITE" != "overwrite" && ( -s "$CLUSTER_DIR/bug_$basecdb" || -s "$CLUSTER_DIR/bug_$basecdb.html" ) ]];
	then
		echo "Bug report \"$CLUSTER_DIR/bug_$basecdb[.html]\" exists. Skip it or set 'overwrite'."
	else
		( time "`dirname $0`/../bugdetect/bugfiltering" "$cdb" $SRCLANG > "${CLUSTER_DIR}/bug_$basecdb" ) 2> "${TIME_DIR}/bugfiltering_$basecdb"
		"$DECKARD_DIR/src/main/out2html" "${CLUSTER_DIR}/bug_$basecdb" > "${CLUSTER_DIR}/bug_${basecdb}.html"
		echo "For clone report \"$cdb\", its bug report is \"bug_$basecdb\";"
		echo "it's HTML format is \"${CLUSTER_DIR}/bug_$basecdb.html\"."
	fi
	echo
done

echo 
echo "==== All Bug Detection Done for the current 'config' file ===="
echo

