#include "elements.h"
#include <memory>
#include "commons.h"
#include "chamber.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include "stackedElement.h"


BOOST_AUTO_TEST_SUITE( StackedCompositeObjectTests )

BOOST_AUTO_TEST_CASE( CreateStackedObjectAndPlaceItOnBoard )
{
    coords point={3,3};
    std::shared_ptr<chamber> board=chamber::makeNewChamber({10,10});
    std::shared_ptr<stackedElement> se=std::make_shared<stackedElement>(board);
    std::shared_ptr<stackedElement> se1=std::make_shared<stackedElement>(board);
    std::shared_ptr<stackedElement> se2=std::make_shared<stackedElement>(board);
    std::shared_ptr<stackedElement> se3=std::make_shared<stackedElement>(board);
    std::shared_ptr<bElem> b0=board->getElement(point);
    se->linkAnElement(se);
    se->setSubtype(0);
    se1->setSubtype(1);
    se2->setSubtype(1);
    se->linkAnElement(se1);
    se->linkAnElement(se2);
    se->linkAnElement(se3);
    BOOST_CHECK(board->getElement(point).get()!=nullptr);
    se->stepOnElement(board->getElement(point));
    std::shared_ptr<bElem> be=board->getElement(point);
    BOOST_CHECK(be->getInstanceid()==se->getInstanceid());
    BOOST_CHECK(be->getSteppingOnElement().get()!=nullptr);
    BOOST_CHECK(be->getSteppingOnElement()->getInstanceid()==se1->getInstanceid());
    BOOST_CHECK(se1->getSteppingOnElement()->getInstanceid()==se2->getInstanceid());
    BOOST_CHECK(se2->getSteppingOnElement()->getInstanceid()==se3->getInstanceid());
    BOOST_CHECK(se3->getSteppingOnElement()->getInstanceid()==b0->getInstanceid());
    se->removeElement();
    be=board->getElement(point);
    BOOST_CHECK(be->getInstanceid()==b0->getInstanceid());

}

BOOST_AUTO_TEST_CASE( StackedCompositeObjectMoveElement)
{
    coords point={3,3};
    direction d=UP;

    std::shared_ptr<chamber> board=chamber::makeNewChamber({10,10});
    std::shared_ptr<stackedElement> se=std::make_shared<stackedElement>(board);
    std::shared_ptr<stackedElement> se1=std::make_shared<stackedElement>(board);
    std::shared_ptr<stackedElement> se2=std::make_shared<stackedElement>(board);
    std::shared_ptr<stackedElement> se3=std::make_shared<stackedElement>(board);
    std::shared_ptr<bElem> b0=board->getElement(point);
    std::shared_ptr<bElem> b1;
    std::shared_ptr<bElem> b2;
    coords np=board->getElement(point)->getAbsCoords(d);

    se->linkAnElement(se);
    se->setSubtype(0);
    se1->setSubtype(1);
    se2->setSubtype(1);
    se->linkAnElement(se1);
    se->linkAnElement(se2);
    se->linkAnElement(se3);
    se->stepOnElement(b0);
    se->moveInDirection(d);
    b1=board->getElement(point);
    b2=board->getElement(np);
    BOOST_CHECK(b1->getInstanceid()==b0->getInstanceid());
    BOOST_CHECK(b2->getInstanceid()==se->getInstanceid());


}

BOOST_AUTO_TEST_SUITE_END()
