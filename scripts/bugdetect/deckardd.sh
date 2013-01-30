#!/bin/bash

#
# 
# Copyright (c) 2007-2013, University of California / Singapore Management University
#   Lingxiao Jiang         <lxjiang@ucdavis.edu> <lxjiang@smu.edu.sg>
#   Ghassan Misherghi      <ghassanm@ucdavis.edu>
#   Zhendong Su            <su@ucdavis.edu>
#   Stephane Glondu        <steph@glondu.net>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the University of California nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
#

echo "DECKARD DEBUG--A Tree-Based Code Clone-Related Bug Detection Tool. Version 1.3"
echo "Copyright (c) 2007-2013. University of California / Singapore Management University"
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

