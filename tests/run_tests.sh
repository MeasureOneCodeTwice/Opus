#!/bin/bash
total_fails=0
testfiles=$(ls | grep -e ".*_test$")
for test in $testfiles
do
    ./$test 
    total_fails=$(($total_fails + $?)) 
done

exit $total_fails
