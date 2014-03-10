#!/bin/bash

# get filename for report
REPORT_FILE=$1
OUTPUT_FILE=`basename $1 .report`.output
if [ `uname -s | cut -b-7` = "MINGW32" ]; then
  EXPECTED_RESULT=`echo "$2" | cut -d, -f 2`
else
  EXPECTED_RESULT=`echo "$2" | cut -d, -f 1`
fi
TARGET=$3
if [ ! "$4" = "--" ]; then
  echo "error: argument error, \$4 has to be '--'!"
  exit 2
fi
shift 4

# run simulavr, this uses all other arguments
echo "$*"
if [ ${GNUTIME} = yes ]; then
  (/usr/bin/time -p $*) > ${REPORT_FILE}.stdout 2> ${REPORT_FILE}.stderr
  RESULT=$?
else
  # bash builtin time command!
  (time -p $*) > ${REPORT_FILE}.stdout 2> ${REPORT_FILE}.stderr
  RESULT=$?
fi

# write report
echo "${TARGET} `tail -n 2 ${REPORT_FILE}.stderr | head -1 | cut "-d " -f 2`" > ${REPORT_FILE}

# write output
echo "stdout:" > $OUTPUT_FILE
cat ${REPORT_FILE}.stdout >> $OUTPUT_FILE
echo "" >> $OUTPUT_FILE
echo "stderr:" >> $OUTPUT_FILE
cat ${REPORT_FILE}.stderr >> $OUTPUT_FILE
echo "" >> $OUTPUT_FILE

# give back exit code
RES=0
if [ ! "$RESULT" = "$EXPECTED_RESULT" ]; then
  echo ""
  echo "error: return code from simulavr = $RESULT, expected = $EXPECTED_RESULT!"
  echo ""
  cat $OUTPUT_FILE
  RES=1
fi
rm -f ${REPORT_FILE}.stdout ${REPORT_FILE}.stderr
exit $RES

# EOF
