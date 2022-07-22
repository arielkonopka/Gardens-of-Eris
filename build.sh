#!/bin/bash


objPath=./obj/Release/
extraflags="-g"
mkdir -p ${objPath}

for x in src/*.cpp ./main.cpp ; do 
    gcc-11 ${extraflags} -O3 -c ${x} -I./include --std=gnu++20 -Wall -march=skylake -o ${objPath}${x%.cpp}.o 
done

gcc-11 ${objPath}src/*.o  ${objPath}*.o -o ./gardenOfEris -lstdc++ -lallegro -lallegro_image

