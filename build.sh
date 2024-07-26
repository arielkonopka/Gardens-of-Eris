#!/bin/bash

#
# As the name implies, prepare a sonar coverage report path
#
sonarDirPrepare() {
    pth=$(dirname `grep 'sonar.coverageReportPaths' sonar-project.properties |cut -f2 -d'='`)
    if [ -n "${pth}" ] ; then
	mkdir -p "${pth}" 
    fi
    
}

objPath=./obj/GoE-objects/
extraflags="-g"
sonarQ="false"
defines="" #-D_VerbousMode_" 
opts="-Wall -std=gnu++20  ${defines} -g -O3 -Og"
libs="-lstdc++ -lallegro -lallegro_image -lallegro_font -lallegro_primitives -lallegro_ttf -lm -lopenal -lsndfile "
. ./build.sh.cfg

#rm -rf ${objPath}
mkdir -p ${objPath}/{src,unitTests}
while [ $# -gt 0 ] ; do
    case "${1}" in
    "--help")
	echo "Garden of Eris or Obnoxious Labirynth build script for GNU Linux
usage:
./build.sh [-sq] [-m moduleName] [-g] [-a] [-t]
-gh - for github build system - install necessary packages for Ubuntu
-rt - run the tests while you build them, combine with -t and -m
-a - build all
-g - build only the game elements and link the game
-t - build only the tests
-m moduleName - build only one cpp file, if combined with -t, then the test suit file is build.
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
	
	echo "Update repository info"
	sudo apt update 2>&1 >/dev/null
	echo "Performming full upgrade on the system"
	sudo apt full-upgrade -y
	echo "Installing allegro5, boost, rapidjson, gcovr"
	sudo apt install -y libboost-all-dev liballegro5-dev liballegro5.2 rapidjson-dev gcovr
	echo "Installing openAl and libsndFile"
	sudo apt install -y libopenal-dev libalut-dev libsndfile1-dev
	exit 0
    ;;
    "-sq")
	sonarDirPrepare
	extraflags="${extraflags} --coverage -fprofile-abs-path "
	linkAdditionalFLags=" -lgcov --coverage "
	runTests="true"
	sonarQ="true"
    ;;
    "-m")
	shift
        stopme="true"
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
	    echo " * ${gccbin} ${objPath}${module}*.o  ${objPath}/${module}.o -o ./GoE-tests-${a%.cpp} ${libs} -lboost_unit_test_framework ${linkAdditionalFLags}"
	    ${gccbin} ${objPath}/src/*.o  ${objPath}/${module}.o  -o ./GoE-tests-${a%.cpp} ${libs} -lboost_unit_test_framework ${linkAdditionalFLags}   3>&1 2>&1 >>compile.log
	fi
	
    ;;
    "-g")
	buildGame="true"
	buildTests="false"
    ;;
    "-a")
	buildGame="true"
	buildTests="true"
    ;;
    "-t")
	buildTests="true"
	buildGame="false"
    ;;
    esac
    shift
done
if [ "${stopme}" = "true" ] ; then
    echo "Now linking the game..."
    echo "${gccbin} ${objPath}src/*.o  ${objPath}*.o -o ./GoEoOL/GoEoOL  ${libs} ${linkAdditionalFLags}"
    ${gccbin} ${objPath}src/*.o  ${objPath}*.o -o ./GoEoOL/GoEoOL  ${libs} ${linkAdditionalFLags}  3>&1   2>&1 >>compile.log
    echo "Done"
    echo "Enjoy..."
    exit 0
fi
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
	echo " * ${gccbin} ${objPath}src/*.o  ${objPath}/${x%.cpp}.o -o ./GoE-tests-${a%.cpp}  ${libs} -lboost_unit_test_framework ${linkAdditionalFLags}"
	${gccbin} ${objPath}src/*.o  ${objPath}/${x%.cpp}.o -o ./GoE-tests-${a%.cpp}  ${libs} -lboost_unit_test_framework ${linkAdditionalFLags}   3>&1 2>&1 >>compile.log
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
echo "${gccbin} ${objPath}src/*.o  ${objPath}*.o -o ./GoEoOL/GoEoOL ${libs}  ${linkAdditionalFLags}"
 
${gccbin} ${objPath}src/*.o  ${objPath}*.o -o ./GoEoOL/GoEoOL ${libs}  ${linkAdditionalFLags}  3>&1   2>&1 >>compile.log

