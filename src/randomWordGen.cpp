#include "randomWordGen.h"

randomWordGen::randomWordGen()
{
    std::random_device rd;
    std::mt19937::result_type seed = rd() ^ (
                                         (std::mt19937::result_type)
                                         std::chrono::duration_cast<std::chrono::seconds>(
                                                 std::chrono::system_clock::now().time_since_epoch()
                                         ).count() +
                                         (std::mt19937::result_type)
                                         std::chrono::duration_cast<std::chrono::microseconds>(
                                                 std::chrono::high_resolution_clock::now().time_since_epoch()
                                         ).count() );
    this->randomNumberGenerator.seed(seed);


}

randomWordGen::~randomWordGen()
{
    //dtor
}
std::string randomWordGen::generateWord(int length)
{
    std::string res="";
    for(int c=0;c<length;c++)
    {
        res=res+this->sylables[this->randomNumberGenerator()%this->sylables.size()];

    }
    return res;
}
