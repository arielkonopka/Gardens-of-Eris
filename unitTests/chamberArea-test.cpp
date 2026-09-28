// *** ADDED BY HEADER FIXUP ***
#include <list>
// *** END ***
#include "elements.h"
#include "commons.h"
#include "chamber.h"
#include <gtest/gtest.h>
#include "testSupport.h"


// Generate a more or less even tree, the result should be repeatable every time.
std::unique_ptr<chamberArea> generateTree(coords leftUp,coords downRight,int depth)
{
    auto root=std::make_unique<chamberArea>(leftUp.x,leftUp.y,downRight.x,downRight.y);
    int lx=downRight.x-leftUp.x;
    int ly=downRight.y-leftUp.y;
    int middlex=leftUp.x+lx/2;
    int middley=leftUp.y+ly/2;
    if(depth<0)
        return root;
    root->addChildNode(generateTree(leftUp, {middlex,middley},depth-1));
    root->addChildNode(generateTree({middlex+1,leftUp.y},downRight,depth-1));
    root->addChildNode(generateTree({leftUp.x,middley+1}, {middlex,downRight.y},depth-1));
    root->addChildNode(generateTree({middlex+1,middley+1},downRight,depth-1));
    return root;
}

// Generate an uneven tree, the result should be repeatable every time.
std::unique_ptr<chamberArea> generateTreeAsimmetric(coords leftUp,coords downRight,int depth)
{
    auto root=std::make_unique<chamberArea>(leftUp.x,leftUp.y,downRight.x,downRight.y);
    int lx=downRight.x-leftUp.x;
    int ly=downRight.y-leftUp.y;
    lx=(depth%2==0)?lx*0.4:lx*1.4;
    ly=(depth%2==0)?ly*0.4:ly*1.4;
    int middlex=leftUp.x+lx/2;
    int middley=leftUp.y+ly/2;
    if(depth<0)
        return root;
    root->addChildNode(generateTreeAsimmetric(leftUp, {middlex,middley},depth-1));
    root->addChildNode(generateTreeAsimmetric({middlex+1,leftUp.y},downRight,depth-1));
    root->addChildNode(generateTreeAsimmetric({leftUp.x,middley+1}, {middlex,downRight.y},depth-1));
    root->addChildNode(generateTreeAsimmetric({middlex+1,middley+1},downRight,depth-1));
    return root;
}

TEST(ChamberAreaTests, TreeCreateAndDestroy)
{
    auto root=generateTree({0,0}, {100,100},4);
    EXPECT_EQ(root->children.size(),4u);
}

TEST(ChamberAreaTests, SearchForSurfacesThatFit)
{
    std::vector<unsigned long int> surfaces1,surfaces2;
    auto root=generateTree({0,0}, {250,250},5);
    root->calculateInitialSurface();
    for(int cnt=2; cnt<45; cnt++)
    {
        auto found=root->findChambersCloseToSurface(cnt,100);
        EXPECT_TRUE(found.size()>0);
        surfaces1.push_back(found.size());
        for(unsigned int c2=0; c2<found.size() && c2<5; c2++)
            EXPECT_TRUE(root->removeArea(found[c2]));
        root->removeEmptyNodes();
    }
    root=generateTree({0,0}, {250,250},5);

    for(int cnt=2; cnt<45; cnt++)
    {
        root->calculateInitialSurface();
        auto found=root->findChambersCloseToSurface(cnt,100);
        EXPECT_TRUE(found.size()>0);
        surfaces2.push_back(found.size());
        for(unsigned int c2=0; c2<found.size() && c2<5; c2++)
        {
            EXPECT_TRUE(root->removeArea(found[c2]));
            root->removeEmptyNodes();
        }
    }
    // Let's check if the recalculating size works properly
    for(unsigned int c=0;c<surfaces1.size();c++)
        EXPECT_TRUE(surfaces1[c]==surfaces2[c]);
    while(true)
    {
        auto found=root->findChambersCloseToSurface(1,1900);
        if(found.empty() || !root->parentOf(found.front())) break;
        for(auto &area : found)
            root->removeArea(area);
    }
}

TEST(ChamberAreaTests, AsimmetricChamberAreasSearch)
{
    auto root=generateTreeAsimmetric({0,0}, {250,250},5);
    root->calculateInitialSurface();
    for(int cnt=2; cnt<45; cnt++)
    {
        auto found=root->findChambersCloseToSurface(cnt,100);
        EXPECT_TRUE(found.size()>0);
        for(unsigned int c2=0; c2<found.size() && c2<5; c2++)
            root->removeArea(found[c2]);
        root->calculateInitialSurface();
    }
    while(true)
    {
        auto found=root->findChambersCloseToSurface(1,19000);
        if(found.empty() || !root->parentOf(found.front())) break;
        for(auto &area : found)
            root->removeArea(area);
    }
}

TEST(ChamberAreaTests, RemovingAnAreaShrinksItsAncestors)
{
    auto root=generateTree({0,0}, {100,100},2);
    const int total=root->calculateInitialSurface();
    auto &leaf=*root->children[0]->children[0]->children[0];
    const int leafSurface=leaf.surface;
    auto parent=root->parentOf(leaf);
    ASSERT_TRUE(parent.has_value());
    EXPECT_EQ(&parent->get(),root->children[0]->children[0].get());
    EXPECT_TRUE(root->removeArea(leaf));
    EXPECT_EQ(root->surface,total-leafSurface);
    EXPECT_FALSE(root->parentOf(*root));
}
