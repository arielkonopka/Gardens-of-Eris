#!/bin/bash

gccbin="gcc-12"
objPath=./obj/GoE-objects/
extraflags="-g"
mkdir -p ${objPath}/{src,unitTests}

opts="-Wall -std=gnu++20 -march=native -Og -g -O3"

echo -n "Building the game ["

#
if [ "${1}" = "-a" ] ; then


for x in src/*.cpp ./main.cpp ; do 
	echo    ${gccbin} ${extraflags} ${opts} -c ${x} -I./include  -o ${objPath}${x%.cpp}.o 
#    echo -n "5"
    ${gccbin} ${extraflags} ${opts} -c ${x} -I./include  -o ${objPath}${x%.cpp}.o  #2>&1 3>&1 >>compile.log
done

fi


if [ "${1}" = "-t" ] || [ "${1}" = "-a" ] ; then

for x in unitTests/*.cpp ; do
    echo -n "5"
    ${gccbin} ${extraflags} ${opts} -c ${x} -I./include  -o ${objPath}/${x%.cpp}.o -D_UNIT_TEST_BUILD_   3>&1 2>&1 >>compile.log
    a=$(basename $x)
    ${gccbin} ${objPath}src/*.o  ${objPath}/${x%.cpp}.o -o ./GoE-tests-${a%.cpp} -lstdc++ -lallegro -lallegro_image -lallegro_font -lallegro_ttf -lm -lboost_unit_test_framework  3>&1 2>&1 >>compile.log
done
fi
echo "]"

echo "Now building the game..."

${gccbin} ${objPath}src/*.o  ${objPath}*.o -o ./GoE -lstdc++ -lallegro -lallegro_image -lallegro_font -lallegro_ttf -lm  3>&1   2>&1 >>compile.log

