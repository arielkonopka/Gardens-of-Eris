
#ifdef _UNIT_TEST_BUILD_
#ifndef BELEM_H_INCLUDED
#define BELEM_H_INCLUDED
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

    bElem* beOrig=chmbr->getElement(0,0);
    bElem* be=new bElem(chmbr);
    BOOST_ASSERT( be!=NULL );
    be->stepOnElement(chmbr->getElement(0,0));
    bElem* be2=chmbr->getElement(0,0);
    BOOST_CHECK(be->getInstanceid()==be2->getInstanceid());
    BOOST_ASSERT(be->steppingOn!=NULL);
    BOOST_CHECK(be->steppingOn->getInstanceid()==beOrig->getInstanceid());
    BOOST_CHECK(beOrig->getStomper()!=NULL);
    BOOST_CHECK(beOrig->getStomper()->getInstanceid()==be->getInstanceid());
    be->removeElement();
    BOOST_CHECK(beOrig->getStomper()==NULL);
    be2=chmbr->getElement(0,0);
    BOOST_CHECK(beOrig->getInstanceid()==be2->getInstanceid());
    delete be;
    delete chmbr;


    /*
     Napisać resztę testu - tutaj t
    */



}

BOOST_AUTO_TEST_SUITE_END()





#endif // BELEM_H_INCLUDED
#endif
