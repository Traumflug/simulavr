#!/bin/bash

# get filename for report
REPORT_FILE=$1
OUTPUT_FILE=`basename $1 .report`.output
EXPECTED_RESULT=$2
TARGET=$3
if [ ! "$4" = "--" ]; then
  echo "error: argument error, \$4 has to be '--'!"
  exit 2
fi
shift 4

# run simulavr, this uses all other arguments
echo "$*"
/usr/bin/time -f "$TARGET %U" -o ${REPORT_FILE}.time $* > ${REPORT_FILE}.stdout 2> ${REPORT_FILE}.stderr
RESULT=$?

# write report
tail -n 1 ${REPORT_FILE}.time > ${REPORT_FILE}
echo "" >> ${REPORT_FILE}.stderr
echo "time:" >> ${REPORT_FILE}.stderr
cat ${REPORT_FILE}.time >> ${REPORT_FILE}.stderr

# write output
echo "stdout:" > $OUTPUT_FILE
cat ${REPORT_FILE}.stdout >> $OUTPUT_FILE
echo "" >> $OUTPUT_FILE
echo "stderr:" >> $OUTPUT_FILE
cat ${REPORT_FILE}.stderr >> $OUTPUT_FILE
echo "" >> $OUTPUT_FILE

# give back exit code
RES=0
if [ ! $RESULT = $EXPECTED_RESULT ]; then
  echo ""
  echo "error: return code from simulavr = $RESULT, expected = $EXPECTED_RESULT!"
  echo ""
  cat $OUTPUT_FILE
  RES=1
fi
rm -f ${REPORT_FILE}.stdout ${REPORT_FILE}.stderr ${REPORT_FILE}.time
exit $RES

# EOF
