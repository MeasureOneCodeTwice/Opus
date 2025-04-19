#!/bin/bash
testfiles=$(ls | grep -e ".*_test$")

RED="\033[31m"
GREEN="\033[32m"
RESET="\033[39m"

#check for test failures
total_fails=0
for test in $testfiles
do
    printf "\n\n----- RUNNING $test -----\n\n"
    ./$test 
    total_fails=$(($total_fails + $?)) 
done

#run tests again, but this time check for memory leaks
bytes_lost=0
byte_loss_regex='in use at exit: ([0-9]+) bytes in [0-9]* blocks.'
printf "\n\n"
for test in $testfiles
do
    printf "\n----- CHECKING $test FOR MEMORY LEAKS -----\n\n"

    valgrind --leak-check=full -s ./$test 1>/dev/null 2>temp
    valgrind_output=$(cat temp)
    rm temp

    [[ $valgrind_output =~ $byte_loss_regex ]]
    if ((${BASH_REMATCH[1]} == 0)) 
    then
        printf "< $GREEN no memory leaked! $RESET > \n"
    else
        printf "\t< $RED MEMORY LEAK DETECTED! $RESET > \n"
        printf "  $RED ${BASH_REMATCH[0]} $RESET\n"
        echo "$valgrind_output"
    fi
    bytes_lost=$(($bytes_lost + ${BASH_REMATCH[1]}))

done

exit $(($total_fails + $bytes_lost))
