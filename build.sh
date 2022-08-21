#!/bin/bash

gccbin="gcc-11"
objPath=./obj/Release/
extraflags="-g"
mkdir -p ${objPath}/src
opts="-Wall -std=gnu++20 -march=native -Og -g -O3"



for x in src/*.cpp ./main.cpp ; do 
echo    ${gccbin} ${extraflags} ${opts} -c ${x} -I./include  -o ${objPath}${x%.cpp}.o 
    ${gccbin} ${extraflags} ${opts} -c ${x} -I./include  -o ${objPath}${x%.cpp}.o 
done


${gccbin} ${objPath}src/*.o  ${objPath}*.o -o ./GoE -lstdc++ -lallegro -lallegro_image -lallegro_font -lallegro_ttf

