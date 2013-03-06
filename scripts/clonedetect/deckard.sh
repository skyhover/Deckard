#!/bin/bash

echo "DECKARD--A Tree-Based Code Clone Detection Toolkit."
if [[ -f "`dirname $0`/../../README" ]]; then
   grep Version "`dirname $0`/../../README"
else
   echo " * Version Unknown. Missing README."
fi
echo "Copyright (c) 2007-2013. University of California / Singapore Management University"
echo "Distributed under the three-clause BSD license."
echo

echo -n "==== Configuration checking..."
. `dirname $0`/configure
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
			"`dirname $0`/vdbgen" clean
			"`dirname $0`/vertical-param-batch" clean
			exit $?
			;;
		clean_all )
			"`dirname $0`/vdbgen" clean_all
			"`dirname $0`/vertical-param-batch" clean_all
			exit $?
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



echo "==== Start clone detection ====" 
echo

SRCLANG=${FILE_PATTERN##*.}
case $SRCLANG in
	c | h | java | php )
		;;
	*)
		echo "Error: language '$SRCLANG' not supported."
		exit 1 ;;
esac

echo -n "Vector generation..."
"`dirname $0`/vdbgen" $TOOVERWRITE
errcode=$?
if [[ $errcode -ne 0 ]]; then
	echo "Error: problem in vec generator step. Stop and check logs in $TIME_DIR/"
	exit $errcode
else
	echo "Vector generation done. Logs in $TIME_DIR/vgen_*"
	echo "Vector files in $VECTOR_DIR/vdb_*"
	echo
fi

echo "Vector clustering and filtering..."
"`dirname $0`/vertical-param-batch" $TOOVERWRITE
errcode=$?
if [[ $errcode -ne 0 ]]; then
	echo "Error: problem in vec clustering step. Stop and check logs in $TIME_DIR/"
	exit $errcode
fi

echo "Clone detection done. Logs in $TIME_DIR/*"
echo "Clone reports in $CLUSTER_DIR/post_cluster_*"
echo

# Bug Finding:
echo "In addition, potential clone-related bugs may be produced by running the scripts:"
echo "(Be careful about the file overwriting and the choice for programming language)"
echo 
echo "(0) search clone reports and find out suspicious ones for all config combinations: "
echo "(Note that it may be slow if there are many clone groups)"
echo "    \"$DECKARD_DIR/scripts/bugdetect/deckardd.sh\""
echo 
echo "Or, run \"bugfilter\" for each clone cluster file, but need to export DECKARD_DIR first:"
echo "    export DECKARD_DIR=$DECKARD_DIR"
echo "(1) search clone reports and find out suspicious ones: "
echo "    \"$DECKARD_DIR/scripts/bugdetect/bugfiltering\" \"${CLUSTER_DIR}/post_<filename>\" $SRCLANG > \"${CLUSTER_DIR}/bug_<filename>\" 2> \"${TIME_DIR}/bugfiltering_<filename>\""
echo "(2) transform the bug reports to html for easier investigation:"
echo "    \"$DECKARD_DIR/src/main/out2html\" \"${CLUSTER_DIR}/bug_<filename>\" > \"${CLUSTER_DIR}/bug_<filename>.html\""
echo
## # the actual commands for bug finding:
## find "$CLUSTER_DIR" -type f -name "post_cluster_vdb_*_*_allg_*_*" | while read cdb;
## do
## 	basecdb=`basename "$cdb"`
## 	basecdb=${basecdb#post_}
## 	( time "`dirname $0`/../bugdetect/bugfiltering" "$cdb" $SRCLANG > "${CLUSTER_DIR}/bug_$basecdb" ) 2> "${TIME_DIR}/bugfiltering_$basecdb"
## 	"$DECKARD_DIR/src/main/out2html" "${CLUSTER_DIR}/bug_$basecdb" > "${CLUSTER_DIR}/bug_${basecdb}.html"
## done

echo 
echo "==== All Done for the current 'config' file ===="
echo

