
//#ifndef _UNIT_TEST_BUILD_
//#ifndef BELEM_H_INCLUDED
//#define BELEM_H_INCLUDED
#include "bElem.h"
#include "commons.h"
#include "chamber.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>







/* bElem class unit tests */

BOOST_AUTO_TEST_SUITE( BasicObjectTests )
BOOST_AUTO_TEST_CASE( bElemCreateDestroy )
{
    // the simpliest possible test case
    bElem* be=new bElem();
    BOOST_CHECK( be!=NULL);
    delete be;

}
BOOST_AUTO_TEST_CASE( bElemCreateDestroyChamber)
{
    chamber* chmbr=new chamber(10,10); // we need only a small chamber
    BOOST_ASSERT( chmbr!=NULL );

    for(int c=0; c<chmbr->width; c++) // check that all elements are not null
        for(int d=0; d<chmbr->height; d++)
        {
            bElem* beOrig=chmbr->getElement(c,d);
            BOOST_ASSERT(beOrig!=NULL);
            BOOST_CHECK( beOrig->getType()==_belemType);
            coords crds=beOrig->getCoords();
            BOOST_ASSERT(crds.x==c);
            BOOST_ASSERT(crds.y==d);
            BOOST_CHECK(beOrig->getStomper()==NULL);
            BOOST_ASSERT(beOrig->steppingOn==NULL);
        }

    bElem* beOrig=chmbr->getElement(0,0); // ok, now let's step on something
    bElem* be=new bElem(chmbr);
    BOOST_ASSERT( be!=NULL );
    be->stepOnElement(chmbr->getElement(0,0));
    BOOST_CHECK(be->getBoard()==chmbr);
    bElem* be2=chmbr->getElement(0,0); // check if the element is placed
    BOOST_CHECK(be->getInstanceid()==be2->getInstanceid());
    BOOST_ASSERT(be->steppingOn!=NULL); // something is under the new object
    BOOST_CHECK(be->steppingOn->getInstanceid()==beOrig->getInstanceid()); // check it is original background
    BOOST_CHECK(beOrig->getStomper()!=NULL); // check, that the object below, "knows" it is below.
    BOOST_CHECK(beOrig->getStomper()->getInstanceid()==be->getInstanceid());
    be->removeElement(); // remove the object from the board
    BOOST_CHECK(beOrig->getStomper()==NULL); //check if the original object is being stepped on
    be2=chmbr->getElement(0,0); // fetch the element from the board, and compare it with the original object, there should be a match
    BOOST_CHECK(beOrig->getInstanceid()==be2->getInstanceid());
    BOOST_CHECK(beOrig->getBoard()==chmbr);
    BOOST_CHECK(be->getBoard()==NULL);
    BOOST_CHECK(be->getCoords()==NOCOORDS);
    delete be;
    delete chmbr;


    /*
     Napisać resztę testu - tutaj t
    */



}
BOOST_AUTO_TEST_CASE(SubTypeChecker)
{
    bElem* myobj=new bElem();
    for(int x=0;x<10;x++){
    myobj->setSubtype(x);
    BOOST_ASSERT(myobj->getSubtype()==x);
    }
    delete myobj;

}

// BOOST_AUTO_TEST_CASE()

BOOST_AUTO_TEST_SUITE_END()





//#endif // BELEM_H_INCLUDED
//#endif
