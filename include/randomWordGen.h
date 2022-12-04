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
