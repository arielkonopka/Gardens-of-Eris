#!/bin/bash
objPath=./obj/GoE-objects/
extraflags="-g"
sonarQ="false"
defines="" #-D_VerbousMode_" 
opts="-Wall -std=gnu++20  ${defines} -g -O3 -Og"

. ./build.sh.cfg

rm -rf ${objPath}
mkdir -p ${objPath}/{src,unitTests}

if [ "${1}" = "-sq" ] ; then
    extraflags="${extraflags} --coverage -fprofile-abs-path "
    linkAdditionalFLags=" -lgcov --coverage "
    runTests="true"
    sonarQ="true"
    mkdir -p ${reportPath}/
    shift
fi

sudo apt install -y libboost-all-dev liballegro5-dev liballegro5.2 rapidjson-dev 
#
if [ "${1}" = "-m" ] ; then
    module="src/${2}"
    echo ${gccbin} ${extraflags} ${opts} -c ${module}.cpp -I./include  -o ${objPath}${module}.o
    ${gccbin} ${extraflags} ${opts} -c ${module}.cpp -I./include  -o ${objPath}${module}.o
    
fi
if [ "${1}" = "-a" ] || [ "${1}" = "-g" ] ; then

echo -n "Building the game ["

for x in src/*.cpp ./main.cpp ; do 
	echo    ${gccbin} ${extraflags} ${opts} -c ${x} -I./include  -o ${objPath}${x%.cpp}.o 
#    echo -n "5"
    ${gccbin} ${extraflags} ${opts} -c ${x} -I./include  -o ${objPath}${x%.cpp}.o  #2>&1 3>&1 >>compile.log
done
echo "]"

fi


if [ "${1}" = "-t" ] || [ "${1}" = "-a" ] ; then

echo "Building unit tests"
for x in unitTests/*.cpp ; do
#    echo -n "5"
    echo "${gccbin} ${extraflags} ${opts} -c ${x} -I./include  -o ${objPath}/${x%.cpp}.o -D_UNIT_TEST_BUILD_ "
    ${gccbin} ${extraflags} ${opts} -c ${x} -I./include  -o ${objPath}/${x%.cpp}.o -D_UNIT_TEST_BUILD_   3>&1 2>&1 >>compile.log
    a=$(basename $x)
    echo "${gccbin} ${objPath}src/*.o  ${objPath}/${x%.cpp}.o -o ./GoE-tests-${a%.cpp} -lstdc++ -lallegro -lallegro_image -lallegro_font -lallegro_ttf -lm -lboost_unit_test_framework ${linkAdditionalFLags}"
    ${gccbin} ${objPath}src/*.o  ${objPath}/${x%.cpp}.o -o ./GoE-tests-${a%.cpp} -lstdc++ -lallegro -lallegro_image -lallegro_font -lallegro_ttf -lm -lboost_unit_test_framework ${linkAdditionalFLags}   3>&1 2>&1 >>compile.log
    
    if [ "${runTests}" = "true" ]; then
	./GoE-tests-${a%.cpp} -r detailed -m CLF -e "Report_${a%.cpp}.log" -k testslog.log 
	r=$?
	if [ ${r} -gt 0 ] ; then
	    echo "Module ${a%.cpp} has failed the tests".
	fi
    fi
done

fi

if [ "${sonarQ}" = "true" ] ; then
    #generate coverage report
    p="${PWD}"
    cd "${reportPath}"
    for it in "${p}"/obj/GoE-objects/src/*.o "${p}"/obj/GoE-objects/unitTests/*.o ; do
	gcov --preserve-paths "${it}"
    done


fi

echo "Now linking the game..."

${gccbin} ${objPath}src/*.o  ${objPath}*.o -o ./GoEoOL/GoEoOL -lstdc++ -lallegro -lallegro_image -lallegro_font -lallegro_ttf -lm  ${linkAdditionalFLags}  3>&1   2>&1 >>compile.log

