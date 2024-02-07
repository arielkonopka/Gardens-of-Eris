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
#include <list>
// *** END ***
#include "elements.h"
#include "commons.h"
#include "chamber.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Fixtures
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <viewPoint.h>
#include "bElem.h"
#include "elementFactory.h"
#include <memory>
BOOST_AUTO_TEST_SUITE( ViewPointTests )
    BOOST_AUTO_TEST_CASE( CheckEmptyViewPoint)
    {
        BOOST_CHECK(viewPoint::get_instance()->getViewPoint()==NOCOORDS);
        BOOST_CHECK(viewPoint::get_instance()->getViewPointOffset()==NOCOORDS);
    }
    BOOST_AUTO_TEST_CASE(CheckAddOwner)
    {
        coords csize={50,50};
        coords point={10,10};
        std::shared_ptr<chamber> ch=chamber::makeNewChamber(csize);
        std::shared_ptr<bElem> be=elementFactory::generateAnElement<bElem>(ch,0);
        std::shared_ptr<bElem> be2=elementFactory::generateAnElement<bElem>(ch,0);

        be->stepOnElement(ch->getElement(point));
        be2->stepOnElement(ch->getElement(point+1));
        BOOST_CHECK(viewPoint::get_instance()->getViewPoint()==NOCOORDS);

        viewPoint::get_instance()->setOwner(be);
        BOOST_CHECK(viewPoint::get_instance()->getViewPoint()==point);
        be->stepOnElement(ch->getElement(point+2));
        BOOST_CHECK(viewPoint::get_instance()->getViewPoint()==point+2);
        be->disposeElement();
        BOOST_CHECK(viewPoint::get_instance()->getViewPoint()==NOCOORDS);

    }
BOOST_AUTO_TEST_SUITE_END()