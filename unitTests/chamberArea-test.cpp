// *** ADDED BY HEADER FIXUP ***
#include <list>
// *** END ***
#include "elements.h"
#include "commons.h"
#include "chamber.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

BOOST_AUTO_TEST_SUITE(ChamberAreaTests)

// Generate a more or less even tree, the result should be repeatable every time.
chamberArea* generateTree(chamberArea* _root,coords leftUp,coords downRight,int depth)
{
    chamberArea* root=_root;
    root=new chamberArea(leftUp.x,leftUp.y,downRight.x,downRight.y);
    root->parent=_root;
    int lx=downRight.x-leftUp.x;
    int ly=downRight.y-leftUp.y;
    int middlex=leftUp.x+lx/2;
    int middley=leftUp.y+ly/2;
    if(depth<0)
        return root;
    root->addChildNode(generateTree(root,leftUp, {middlex,middley},depth-1));
    root->addChildNode(generateTree(root, {middlex+1,leftUp.y},downRight,depth-1));
    root->addChildNode(generateTree(root, {leftUp.x,middley+1}, {middlex,downRight.y},depth-1));
    root->addChildNode(generateTree(root, {middlex+1,middley+1},downRight,depth-1));
    return root;
}

// Generate a more or less even tree, the result should be repeatable every time.
chamberArea* generateTreeAsimmetric(chamberArea* _root,coords leftUp,coords downRight,int depth)
{
    chamberArea* root=_root;
    root=new chamberArea(leftUp.x,leftUp.y,downRight.x,downRight.y);
    root->parent=_root;
    int lx=downRight.x-leftUp.x;
    int ly=downRight.y-leftUp.y;
    lx=(depth%2==0)?lx*0.4:lx*1.4;
    ly=(depth%2==0)?ly*0.4:ly*1.4;
    int middlex=leftUp.x+lx/2;
    int middley=leftUp.y+ly/2;
    if(depth<0)
        return root;
    root->addChildNode(generateTreeAsimmetric(root,leftUp, {middlex,middley},depth-1));
    root->addChildNode(generateTreeAsimmetric(root, {middlex+1,leftUp.y},downRight,depth-1));
    root->addChildNode(generateTreeAsimmetric(root, {leftUp.x,middley+1}, {middlex,downRight.y},depth-1));
    root->addChildNode(generateTreeAsimmetric(root, {middlex+1,middley+1},downRight,depth-1));
    return root;
}




BOOST_AUTO_TEST_CASE( TreeCreateAndDestroy)
{
    chamberArea* root=generateTree(nullptr, {0,0}, {100,100},4);

    delete root;

}



BOOST_AUTO_TEST_CASE( SearchForSurfacesThatFit )
{
    std::vector<unsigned long int> surfaces1,surfaces2;
    chamberArea* root=generateTree(nullptr, {0,0}, {250,250},5);
    root->calculateInitialSurface();
    for(int cnt=2; cnt<45; cnt++)
    {
        chamberArea::foundAreas.clear();
        root->findChambersCloseToSurface(cnt,100);
        BOOST_CHECK(chamberArea::foundAreas.size()>0);
        surfaces1.push_back(chamberArea::foundAreas.size());
#ifdef _VerbousMode_
        std::cout<<"[1] how many areas found for "<<cnt<<" surface? "<<chamberArea::foundAreas.size()<<"\nAll Areas list\n";
#endif
        for(unsigned int c2=0; c2<chamberArea::foundAreas.size(); c2++)
        {
            chamberArea* chmA=chamberArea::foundAreas[c2];
            if (c2<5) {
                    delete chmA;

                    };
        }
        root->removeEmptyNodes();
    }
    delete root;
    root=generateTree(nullptr, {0,0}, {250,250},5);

    for(int cnt=2; cnt<45; cnt++)
    {
        root->calculateInitialSurface();
        chamberArea::foundAreas.clear();
        root->findChambersCloseToSurface(cnt,100);
        BOOST_CHECK(chamberArea::foundAreas.size()>0);
        surfaces2.push_back(chamberArea::foundAreas.size());
#ifdef _VerbousMode_
        std::cout<<"[2] how many areas found for "<<cnt<<" surface? "<<chamberArea::foundAreas.size()<<"\nAll Areas list\n";
#endif
        for(unsigned int c2=0; c2<chamberArea::foundAreas.size(); c2++)
        {
            chamberArea* chmA=chamberArea::foundAreas[c2];
            if (c2<5){

             delete chmA;
            root->removeEmptyNodes();
            }
        }
       // root->removeEmptyNodes();

    }
    // Let's check if the recalculating size works properly
    for(unsigned int c=0;c<surfaces1.size();c++)
        BOOST_CHECK(surfaces1[c]==surfaces2[c]);
    bool ex=false;
    while(!ex)
    {
        chamberArea::foundAreas.clear();
        root->findChambersCloseToSurface(1,1900);
        if(chamberArea::foundAreas.size()==0) break;
        for(unsigned int c2=0; c2<chamberArea::foundAreas.size(); c2++)
        {
            chamberArea* chmA=chamberArea::foundAreas[c2];
            delete chmA;
        }

    }

    delete root;
}

BOOST_AUTO_TEST_CASE(AsimmetricChamberAreasSearch)
{
    chamberArea* root=generateTreeAsimmetric(nullptr, {0,0}, {250,250},5);
    root->calculateInitialSurface();
    for(int cnt=2; cnt<45; cnt++)
    {
        chamberArea::foundAreas.clear();
        root->findChambersCloseToSurface(cnt,100);
        BOOST_CHECK(chamberArea::foundAreas.size()>0);
#ifdef _VerbousMode_
        std::cout<<"how many areas found for "<<cnt<<" surface? "<<chamberArea::foundAreas.size()<<"\nAll Areas list\n";
#endif
        for(unsigned int c2=0; c2<chamberArea::foundAreas.size(); c2++)
        {
            chamberArea* chmA=chamberArea::foundAreas[c2];
            if (c2<5) delete chmA;
        }
    root->calculateInitialSurface();

    }
    bool ex=false;
    while(!ex)
    {
        chamberArea::foundAreas.clear();
        root->findChambersCloseToSurface(1,19000);
        if(chamberArea::foundAreas.size()==0) break;
        for(unsigned int c2=0; c2<chamberArea::foundAreas.size(); c2++)
        {
            chamberArea* chmA=chamberArea::foundAreas[c2];
            delete chmA;
        }

    }

    delete root;
}
/* Different element configurations on a board, and checking if placing element on a point is acceptable */


BOOST_AUTO_TEST_SUITE_END()
