#!/bin/bash

gccbin="gcc-12"
objPath=./obj/Release/
extraflags="-g"
mkdir -p ${objPath}
opts="-Wall -std=gnu++20 -march=native -Og -g -O3"



for x in src/*.cpp ./main.cpp ; do 
    ${gccbin} ${extraflags} ${opts} -c ${x} -I./include  -o ${objPath}${x%.cpp}.o 
done


${gccbin} ${objPath}src/*.o  ${objPath}*.o -o ./gardenOfEris -lstdc++ -lallegro -lallegro_image

