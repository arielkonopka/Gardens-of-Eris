#!/bin/bash
objPath=./obj/GoE-objects/
extraflags="-g"
sonarQ="false"
defines="" #-D_VerbousMode_" 
opts="-Wall -std=gnu++20  ${defines} -g -O3 -Og"

. ./build.sh.cfg

#rm -rf ${objPath}
mkdir -p ${objPath}/{src,unitTests}
while [ $# -gt 0 ] ; do
    case "${1}" in
    "--help")
	echo "Garden of Erif or Obnoxious Labirynth build script for GNU Linux
usage:
./build.sh [-sq] [-m moduleName] [-g] [-a] [-t]
-gh - for github build system - install necessary packages for Ubuntu
-rt - run the tests while you build them, combine with -t and -m
-a - build all
-g - build only the game elements and link the game
-t - build only the tests
-m moduleName - build only one cpp file, if combinet with -t, then the test cse file is build.
-sq - build and run the tests just for the sonar qube analysis.
examples:
./build.sh -a
./build.sh -m bElem
./build.sh -t -m bElem-test"
    exit 0
    ;;
    "-rt")
	runTests="true"
	;;
    "-gh")
	sudo apt install -y libboost-all-dev liballegro5-dev liballegro5.2 rapidjson-dev
    ;;
    "-sq")
	extraflags="${extraflags} --coverage -fprofile-abs-path "
	linkAdditionalFLags=" -lgcov --coverage "
	runTests="true"
	sonarQ="true"
	mkdir -p ${reportPath}/
    ;;
    "-m")
	shift
	
	if [ "$buildTests" = "true" ] ;then
	    module="unitTests/${1}"
	else
	    module="src/${1}"
	fi
	echo "Building the ${1} module:"
	echo " * ${gccbin} ${extraflags} ${opts} -c ${module}.cpp -I./include  -o ${objPath}${module}.o"
	${gccbin} ${extraflags} ${opts} -c ${module}.cpp -I./include  -o ${objPath}${module}.o
	if [ "${buildTests}" = "true" ] ; then
	    a=`basename ${module}`
	    echo " * ${gccbin} ${objPath}${module}*.o  ${objPath}/${module}.o -o ./GoE-tests-${a%.cpp} -lstdc++ -lallegro -lallegro_image -lallegro_font -lallegro_ttf -lm -lboost_unit_test_framework ${linkAdditionalFLags}"
	    ${gccbin} ${objPath}/src/*.o  ${objPath}/${module}.o  -o ./GoE-tests-${a%.cpp} -lstdc++ -lallegro -lallegro_image -lallegro_font -lallegro_ttf -lm -lboost_unit_test_framework ${linkAdditionalFLags}   3>&1 2>&1 >>compile.log
	fi
	echo "Done"
	exit 0
    ;;
    "-g")
	buildGame="true"
    ;;
    "-a")
	buildGame="true"
	buildTests="true"
    ;;
    "-t")
	buildTests="true"
    ;;
    esac
    shift
done

if [ "${buildGame}" = "true" ] ; then
    echo  "Building the game:"
    for x in src/*.cpp ./main.cpp ; do 
	echo  " * ${gccbin} ${extraflags} ${opts} -c ${x} -I./include  -o ${objPath}${x%.cpp}.o"
	${gccbin} ${extraflags} ${opts} -c ${x} -I./include  -o ${objPath}${x%.cpp}.o 
    done
    echo "Done building game elements"
fi


if [ "${buildTests}" = "true" ] ; then
    echo "Building unit tests:"
    for x in unitTests/*.cpp ; do
	echo " * ${gccbin} ${extraflags} ${opts} -c ${x} -I./include  -o ${objPath}/${x%.cpp}.o -D_UNIT_TEST_BUILD_ "
	${gccbin} ${extraflags} ${opts} -c ${x} -I./include  -o ${objPath}/${x%.cpp}.o -D_UNIT_TEST_BUILD_   3>&1 2>&1 >>compile.log
	a=$(basename $x)
	echo " * ${gccbin} ${objPath}src/*.o  ${objPath}/${x%.cpp}.o -o ./GoE-tests-${a%.cpp} -lstdc++ -lallegro -lallegro_image -lallegro_font -lallegro_ttf -lm -lboost_unit_test_framework ${linkAdditionalFLags}"
	${gccbin} ${objPath}src/*.o  ${objPath}/${x%.cpp}.o -o ./GoE-tests-${a%.cpp} -lstdc++ -lallegro -lallegro_image -lallegro_font -lallegro_ttf -lm -lboost_unit_test_framework ${linkAdditionalFLags}   3>&1 2>&1 >>compile.log
	if [ "${runTests}" = "true" ]; then
	    ./GoE-tests-${a%.cpp} -r detailed -m CLF -e "Report_${a%.cpp}.log" -k testslog.log 
	    r=$?
	    if [ ${r} -gt 0 ] ; then
		echo "Module ${a%.cpp} has failed the tests".
	    fi
	fi
    done
    echo "Done building tests"
fi

if [ "${sonarQ}" = "true" ] ; then
    p="${PWD}"
    gcovr obj/GoE-objects/src/ obj/GoE-objects/unitTests/ --sonarqube -o "${PWD}/CoverageReports/coverageReport.XML"


fi

echo "Now linking the game..."

${gccbin} ${objPath}src/*.o  ${objPath}*.o -o ./GoEoOL/GoEoOL -lstdc++ -lallegro -lallegro_image -lallegro_font -lallegro_ttf -lm  ${linkAdditionalFLags}  3>&1   2>&1 >>compile.log

