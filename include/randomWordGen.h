/*
 * Copyright (c) 2023, Ariel Konopka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef RANDOMWORDGEN_H
#define RANDOMWORDGEN_H
#include <vector>
#include <string>
#include <chrono>
#include <random>


class randomWordGen
{
    public:
        randomWordGen();
        virtual ~randomWordGen();
        std::string generateWord(int length);
         std::mt19937 randomNumberGenerator;
    private:
        std::vector<std::string> sylables={"a","o","u","i","ba","ca","da","fa","ga","gwa","ha","ja","ka","la","ma","na","pa","ra","sa","va","wa","xa","za",
                "bo","co","do","fo","go","gwo","ho","jo","ko","lo","mo","no","po","ro","so","vo","wo","xo","zo",
                "bae","cae","dae","fae","gae","gwae","hae","jae","kae","lae","mae","nae","pae","rae","sae","vae","wae","xae","zae",
                "by","cy","dy","fy","gy","gwy","hy","jy","ky","ly","my","ny","py","ry","sy","vy","wy","xy","zy",
                "be","ce","de","fe","ge","he","je","ke","le","me","ne","pe","re","se","ve","we","xe","ze",
                "bu","cu","du","fu","gu","hu","ju","ku","lu","mu","nu","pu","ru","su","vu","wu","xu","zu",
                "bi","ci","di","fi","gi","hi","ji","ki","li","mi","ni","pi","ri","si","vi","wi","xi","zi"};

};

#endif // RANDOMWORDGEN_H
