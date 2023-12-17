#!/usr/bin/env bash

# function to clean up files and make executables
remake () {
    #echo -e "\nCleaning old files and making executables"
    make -s clean
    make -s >/dev/null 2>&1
}


echo -e "To remove colour from tests, set COLOUR to 1 in sh file\n"
COLOUR=0
if [[ COLOUR -eq 0 ]]; then
    ORANGE='\033[0;33m'
    GREEN='\033[0;32m'
    RED='\033[0;31m'
    NC='\033[0m'
else
    ORANGE='\033[0m'
    GREEN='\033[0m'
    RED='\033[0m'
    NC='\033[0m'
fi

LIB_BOOKS=100
NUM_THREADS=6
SCORE=0

echo -e "\nTesting :: Compilation"
remake
if [[ $? -eq 0 ]]; then
    echo -e "  ${GREEN}Test One Passed${NC}"
    SCORE=$(($SCORE+3))
else
    echo -e "  ${RED}Failed${NC}"
    exit 1
fi

count=()

# Initialize the array with 6 elements
for ((i = 0; i < $NUM_THREADS; i++)); do
    count[$i]=0
done

OUTPUT_FLAG=0
LINE_COUNT=0
echo -e "\nTesting :: Valid output - 1"

while read -r line ; do
    BOOKS=$(echo $line | cut -d ' ' -f 5)
    OUT=$(echo $line | cut -d ' ' -f 4)
    STUDENT=$(echo $line | cut -d ' ' -f 2)
    LINE_COUNT=$(($LINE_COUNT+1))
    if [ $OUT == "out" ] && [[ count[$STUDENT] -eq 0 ]] ; then
        count[$STUDENT]=1
        LIB_BOOKS=$(($LIB_BOOKS-$BOOKS))
    elif [ $OUT == "in" ] && [[ count[$STUDENT] -eq 1 ]]; then
        count[$STUDENT]=2
        LIB_BOOKS=$(($LIB_BOOKS+$BOOKS))
    else
        echo -e "  ${RED}Invalid check${OUT}${NC}"
        OUTPUT_FLAG=1
    fi
    if [[ $LIB_BOOKS -lt 0 ]]; then
        echo -e "  ${RED}Number of books went negative${NC}"
        OUTPUT_FLAG=1
    elif [[ $LIB_BOOKS -gt 100 ]]; then
        echo -e "  ${RED}Number of books exceeded ${LIB_BOOKS}${NC}"
        OUTPUT_FLAG=1
    fi
done < <(strace -f -o trace.txt ./library)

if [ $LINE_COUNT -ne $((2*$NUM_THREADS)) ]; then
    echo -e "  ${RED}Incorrect number of lines in output${NC}"
else
    echo -e "  ${GREEN}Test Two Passed${NC}"
    SCORE=$(($SCORE+5))
fi

echo -e "\nTesting :: Valid output - 2"
if [[ $OUTPUT_FLAG -eq 0 ]]; then
    echo -e "  ${GREEN}Test Three Passed${NC}"
    SCORE=$(($SCORE+15))
else
    echo -e "  ${RED}Failed${NC}"
fi

echo -e "\nTesting :: Thread creation"
if [[ $(grep -cF "clone(" trace.txt) -ge $NUM_THREADS ]]; then
    echo -e "  ${GREEN}Test Four Passed${NC}"
    SCORE=$(($SCORE+4))
else
    echo -e "  ${RED}Failed${NC}"
fi

nm --demangle library > symbols.txt
echo -e "\nTesting :: Correct conditional_variable usage"
if [[ $(grep -cF "FUTEX_WAKE_PRIVATE, 2147483647" trace.txt) -gt 2 ]] && grep -qoP "std::condition_variable::wait<(.+?)>\(std::unique_lock<std::mutex>&, \1\)" symbols.txt; then
    echo -e "  ${GREEN}Test Five Passed${NC}"
    SCORE=$(($SCORE+5))
else
    echo -e "  ${RED}Failed${NC}"
fi

echo -e "\nTesting :: Mutex usage"
grep -qF "std::unique_lock<std::mutex>::unique_lock(std::mutex&)" symbols.txt
if [[ $? -eq 0 ]]; then
    echo -e "  ${GREEN}Test Six passed${NC}"
    SCORE=$(($SCORE+4))
else
    echo -e "  ${RED}Failed${NC}"
fi

echo -e "\nTesting :: Thread join"
grep -qF "std::thread::join()" symbols.txt
if [[ $? -eq 0 ]]; then
    echo -e "  ${GREEN}Test Seven Passed${NC}"
    SCORE=$(($SCORE+4))
else
    echo -e "  ${RED}Failed${NC}"
fi

rm trace.txt
rm symbols.txt

echo "${SCORE}/40"


