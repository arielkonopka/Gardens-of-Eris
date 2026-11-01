/*
 * Eight-way movement (user request of 2026-10-13): everything that moves can go diagonally.
 * A diagonal step passes between the two cells beside it and needs both of them free (user rule
 * of 2026-11-01, after the eight-way audit; until then one free cell was enough), so nothing
 * cuts round the end of a wall and walls touching at a corner still close the maze.
 */
#include "chamber.h"
#include "commons.h"
#include "elements.h"
#include "motion.h"
#include "testSupport.h"
#include <gtest/gtest.h>
#include <cstdlib>
#include <map>
#include <memory>
#include <random>

namespace {
template <typename T>
std::shared_ptr<T> put(const std::shared_ptr<chamber> &board, coords at, int subtype = 0)
{
    auto e = elementFactory::generateAnElement<T>(board, subtype);
    e->stepOnElement(board->getElement(at));
    return e;
}

/// a fresh chamber with an active player at `at`, so bElem::runLiveElements ticks it
std::shared_ptr<chamber> roomWithPlayer(coords size, coords at, std::shared_ptr<bElem> &plr)
{
    inputManager::getInstance(true);
    if (auto old = player::getActivePlayer())
        old->disposeElement();
    auto mc = chamber::makeNewChamber(size);
    plr = elementFactory::generateAnElement<player>(mc, 0);
    plr->stepOnElement(mc->getElement(at));
    return mc;
}

/// gives a controller's helpers to the tests
struct chaser : puppetMasterFR {
    using puppetMasterFR::pathTowards;
    using puppetMasterFR::touching;
    using puppetMasterFR::towards;
};

void settle()
{
    for (int i = 0; i < 40; i++)
        bElem::tick();
}
} // namespace

TEST(MovementTests, DirectionsKnowTheirDiagonals)
{
    using dir::direction;
    EXPECT_EQ(dir::dirToCoords(direction::UPLEFT), coords(-1, -1));
    EXPECT_EQ(dir::dirToCoords(direction::DOWNRIGHT), coords(1, 1));
    EXPECT_EQ(dir::getOppositeDirection(direction::UPRIGHT), direction::DOWNLEFT);
    EXPECT_EQ(dir::getOppositeDirection(direction::LEFT), direction::RIGHT);
    // an eighth of a turn anticlockwise from up, then a quarter, then all the way round
    EXPECT_EQ(dir::turned(direction::UP, 1), direction::UPLEFT);
    EXPECT_EQ(dir::turned(direction::UPLEFT, 2), direction::DOWNLEFT);
    EXPECT_EQ(dir::turned(direction::UP, -1), direction::UPRIGHT);
    EXPECT_EQ(dir::turned(direction::RIGHT, 8), direction::RIGHT);
    EXPECT_EQ(dir::turned(direction::NODIRECTION, 3), direction::NODIRECTION);
    EXPECT_EQ(dir::towards(coords(5, -2)), direction::UPRIGHT);
    EXPECT_EQ(dir::towards(coords(0, 3)), direction::DOWN);
    EXPECT_EQ(dir::towards(coords(0, 0)), direction::NODIRECTION);
    EXPECT_EQ(dir::across(direction::DOWNLEFT), direction::LEFT);
    EXPECT_EQ(dir::along(direction::DOWNLEFT), direction::DOWN);
    EXPECT_EQ(dir::across(direction::UP), direction::UP);
    for (auto d : dir::ways)
        EXPECT_EQ(dir::towards(dir::dirToCoords(d)), d);
    // the old numbers stay: saves and the agent store them
    EXPECT_EQ((int) direction::NODIRECTION, 4);
    EXPECT_TRUE(dir::isDiagonal(direction::UPRIGHT));
    EXPECT_FALSE(dir::isDiagonal(direction::NODIRECTION));
}

TEST(MovementTests, ADiagonalStepNeedsBothCellsBesideItFree)
{
    auto board = chamber::makeNewChamber(coords(8, 8));
    auto walker = put<monster>(board, coords(3, 3));
    // open floor: through
    EXPECT_TRUE(motion::cornerOpen(*board, coords(3, 3), dir::direction::UPRIGHT));
    EXPECT_TRUE(walker->moveInDirection(dir::direction::UPRIGHT));
    EXPECT_EQ(walker->getStats()->getMyPosition(), coords(4, 2));
    settle();
    // one side blocked: no cutting round the end of a wall
    put<wall>(board, coords(4, 3));
    EXPECT_FALSE(motion::cornerOpen(*board, coords(4, 2), dir::direction::DOWNLEFT));
    EXPECT_FALSE(walker->moveInDirection(dir::direction::DOWNLEFT));
    EXPECT_EQ(walker->getStats()->getMyPosition(), coords(4, 2));
    EXPECT_FALSE(motion::cornerOpen(*board, coords(3, 3), dir::direction::UPRIGHT)) << "the same corner seen from the other side";
    // both sides blocked: closed as well
    put<wall>(board, coords(3, 2));
    EXPECT_FALSE(motion::cornerOpen(*board, coords(3, 3), dir::direction::UPRIGHT));
    // the sides never need it
    EXPECT_TRUE(motion::cornerOpen(*board, coords(3, 3), dir::direction::UP));
    // a wall elsewhere does not matter
    EXPECT_TRUE(motion::cornerOpen(*board, coords(3, 3), dir::direction::DOWNLEFT));
}

TEST(MovementTests, AMissileNeverHitsWhatIsBehindAClosedCorner)
{
    // audit finding of 2026-10-31: a missile hurt a target diagonally behind two walls
    std::shared_ptr<bElem> plr;
    auto board = roomWithPlayer(coords(12, 12), coords(2, 2), plr);
    auto gun = put<plainGun>(board, coords(1, 1), 1); // never runs out
    ASSERT_TRUE(plr->collect(gun));
    plr->getStats()->setFacing(dir::direction::DOWNRIGHT);
    ASSERT_TRUE(gun->use(plr));
    const auto shot = board->getElement(3, 3);
    ASSERT_EQ(shot->getType(), bElemTypes::_plainMissile);
    // the corner after (3,3) is closed, and someone stands behind it
    put<wall>(board, coords(4, 3));
    put<wall>(board, coords(3, 4));
    auto target = put<player>(board, coords(4, 4));
    const int before = target->getAttrs()->getEnergy();
    for (int i = 0; i < 200; i++)
        bElem::runLiveElements();
    EXPECT_EQ(target->getAttrs()->getEnergy(), before);
    EXPECT_TRUE(shot->getStats()->isDisposed() || shot->getStats()->isDying());
    plr->disposeElement();
}

TEST(MovementTests, ABazookaMissileBlowsUpOnAClosedCornerInsteadOfHittingBehindIt)
{
    std::shared_ptr<bElem> plr;
    auto board = roomWithPlayer(coords(12, 12), coords(1, 1), plr);
    put<wall>(board, coords(4, 3));
    put<wall>(board, coords(3, 4));
    auto target = put<player>(board, coords(4, 4));
    const int before = target->getAttrs()->getEnergy();
    auto missile = elementFactory::generateAnElement<bazookaMissile>(board, 0);
    missile->getStats()->setStatsOwner(plr);
    missile->getStats()->setMyDirection(dir::direction::DOWNRIGHT);
    ASSERT_TRUE(missile->stepOnElement(board->getElement(3, 3)));
    missile->getAttrs()->setEnergy(100);
    missile->registerLiveElement(missile);
    for (int i = 0; i < 200; i++)
        bElem::runLiveElements();
    EXPECT_EQ(target->getAttrs()->getEnergy(), before) << "no direct hit, and the blast stays on this side";
    plr->disposeElement();
}

TEST(MovementTests, InteractNeverReachesRoundAClosedCorner)
{
    std::shared_ptr<bElem> plr;
    auto board = roomWithPlayer(coords(10, 10), coords(2, 2), plr);
    put<wall>(board, coords(3, 2));
    auto behind = put<key>(board, coords(3, 3));
    inputManager::getInstance().setControlItem(controlItem(2, dir::direction::DOWNRIGHT));
    for (int i = 0; i < 20; i++)
        bElem::runLiveElements();
    EXPECT_EQ(behind->getStats()->getMyPosition(), coords(3, 3)) << "a wall on one side closes the corner";
    EXPECT_FALSE(behind->getStats()->isCollected());
    // on open floor it reaches diagonally
    inputManager::getInstance().setControlItem(controlItem(-1, dir::direction::NODIRECTION));
    settle();
    auto open = put<key>(board, coords(1, 3));
    inputManager::getInstance().setControlItem(controlItem(2, dir::direction::DOWNLEFT));
    for (int i = 0; i < 20 && !open->getStats()->isCollected(); i++)
        bElem::runLiveElements();
    EXPECT_TRUE(open->getStats()->isCollected());
    inputManager::getInstance().setControlItem(controlItem(-1, dir::direction::NODIRECTION));
    plr->disposeElement();
}

TEST(MovementTests, ThePlayerWalksAndPushesDiagonally)
{
    std::shared_ptr<bElem> plr;
    auto board = roomWithPlayer(coords(10, 10), coords(2, 2), plr);
    inputManager::getInstance().setControlItem(controlItem(0, dir::direction::DOWNRIGHT));
    for (int i = 0; i < 3 && plr->getStats()->getMyPosition() == coords(2, 2); i++)
        bElem::runLiveElements();
    EXPECT_EQ(plr->getStats()->getMyPosition(), coords(3, 3));
    EXPECT_EQ(plr->getStats()->getFacing(), dir::direction::DOWNRIGHT);
    // a brick pushed diagonally moves on diagonally
    auto brick = put<brickCluster>(board, coords(4, 4));
    inputManager::getInstance().setControlItem(controlItem(-1, dir::direction::NODIRECTION));
    settle();
    if (plr->moveInDirection(dir::direction::DOWNRIGHT)) {
        EXPECT_EQ(brick->getStats()->getMyPosition(), coords(5, 5));
        EXPECT_EQ(plr->getStats()->getMyPosition(), coords(4, 4));
    } else
        EXPECT_FALSE(brick->getAttrs()->canBePushed()) << "only a brick that cannot be pushed stays";
    plr->disposeElement();
}

TEST(MovementTests, AMissileFliesDiagonallyAndStopsAtAClosedCorner)
{
    std::shared_ptr<bElem> plr;
    auto board = roomWithPlayer(coords(12, 12), coords(2, 2), plr);
    auto gun = put<plainGun>(board, coords(1, 1), 1); // never runs out
    ASSERT_TRUE(plr->collect(gun));
    plr->getStats()->setFacing(dir::direction::DOWNRIGHT);
    ASSERT_TRUE(gun->use(plr));
    const auto shot = board->getElement(3, 3);
    ASSERT_EQ(shot->getType(), bElemTypes::_plainMissile);
    EXPECT_EQ(shot->getStats()->getMyDirection(), dir::direction::DOWNRIGHT);
    // two walls touching at a corner on its way: (6,5) and (5,6) close the corner after (5,5)
    put<wall>(board, coords(6, 5));
    put<wall>(board, coords(5, 6));
    for (int i = 0; i < 200; i++)
        bElem::runLiveElements();
    EXPECT_TRUE(shot->getStats()->isDisposed() || shot->getStats()->isDying());
    EXPECT_NE(board->getElement(6, 6)->getType(), bElemTypes::_plainMissile) << "it must not slip through";
    // shooting straight into a closed corner does nothing beyond it
    put<wall>(board, coords(3, 2));
    put<wall>(board, coords(2, 3));
    for (int i = 0; i < 200; i++)
        bElem::tick();
    gun->use(plr);
    EXPECT_NE(board->getElement(3, 3)->getType(), bElemTypes::_plainMissile);
    plr->disposeElement();
}

TEST(MovementTests, ChasersCutCornersButNotThroughWalls)
{
    auto board = chamber::makeNewChamber(coords(12, 12));
    auto body = put<monster>(board, coords(2, 2));
    // open floor: the shortest way to (6,6) is diagonal
    EXPECT_EQ(chaser::pathTowards(body, coords(6, 6), coords(5, 5), 10), dir::direction::DOWNRIGHT);
    // a wall line from (0,4) to (4,4) and one from (4,0) to (4,3): the way round is through (5,5)'s side
    for (int x = 0; x <= 4; x++)
        put<wall>(board, coords(x, 4));
    for (int y = 0; y < 4; y++)
        put<wall>(board, coords(4, y));
    // (2,2) is closed in on the right and below: no walk leads out
    EXPECT_EQ(chaser::pathTowards(body, coords(6, 6), coords(5, 5), 10), dir::direction::NODIRECTION);
    // touching by a corner counts only when a move could pass it
    EXPECT_TRUE(chaser::touching(*board, coords(2, 2), coords(3, 3)));
    EXPECT_FALSE(chaser::touching(*board, coords(3, 3), coords(5, 5)));
    // which way to face
    EXPECT_EQ(chaser::towards(coords(0, 0), coords(5, 1)), dir::direction::RIGHT);
    EXPECT_EQ(chaser::towards(coords(0, 0), coords(4, 3)), dir::direction::DOWNRIGHT);
    EXPECT_EQ(chaser::towards(coords(0, 0), coords(-1, -6)), dir::direction::UP);
}

TEST(MovementTests, TwoWallsTouchingAtACornerStillSealARoom)
{
    // the shortest walk to a cell behind a corner closed on both sides goes the long way round
    auto board = chamber::makeNewChamber(coords(10, 10));
    auto body = put<monster>(board, coords(3, 3));
    put<wall>(board, coords(4, 3));
    put<wall>(board, coords(3, 4));
    const auto first = chaser::pathTowards(body, coords(4, 4), coords(4, 4), 9);
    EXPECT_NE(first, dir::direction::DOWNRIGHT);
    EXPECT_NE(first, dir::direction::NODIRECTION);
}

namespace {
/// the movement mode for one test; eight ways again afterwards, whatever happens
struct movementMode {
    explicit movementMode(motion::mode m) { motion::setMode(m); }
    ~movementMode() { motion::setMode(motion::mode::eightWay); }
    movementMode(const movementMode &) = delete;
    movementMode &operator=(const movementMode &) = delete;
};

/// where every element on top of a cell stands, by instance id
std::map<unsigned long, coords> whoIsWhere(const chamber &board, coords size)
{
    std::map<unsigned long, coords> at;
    for (int x = 0; x < size.x; x++)
        for (int y = 0; y < size.y; y++)
            if (const auto e = board.topAt(coords(x, y)); e && e->getType() != bElemTypes::_floorType)
                at[e->getStats()->getInstanceId()] = coords(x, y);
    return at;
}
} // namespace

TEST(MovementTests, FourWayModeClosesEveryCorner)
{
    auto board = chamber::makeNewChamber(coords(8, 8));
    EXPECT_EQ(motion::currentMode(), motion::mode::eightWay) << "eight ways is the default";
    {
        movementMode four(motion::mode::fourWay);
        EXPECT_FALSE(motion::cornerOpen(*board, coords(3, 3), dir::direction::UPRIGHT)) << "even on open floor";
        EXPECT_TRUE(motion::cornerOpen(*board, coords(3, 3), dir::direction::UP));
        auto walker = put<monster>(board, coords(3, 3));
        EXPECT_FALSE(walker->moveInDirection(dir::direction::DOWNLEFT));
        EXPECT_EQ(walker->getStats()->getMyPosition(), coords(3, 3));
        EXPECT_TRUE(walker->moveInDirection(dir::direction::LEFT));
    }
    EXPECT_TRUE(motion::cornerOpen(*board, coords(3, 3), dir::direction::UPRIGHT));
    EXPECT_EQ(motion::modeOf(4), motion::mode::fourWay);
    EXPECT_EQ(motion::modeOf(8), motion::mode::eightWay);
    EXPECT_EQ(motion::modeOf(0), motion::mode::eightWay);
}

TEST(MovementTests, InFourWayModeNothingEndsATickOnADiagonalNeighbour)
{
    movementMode four(motion::mode::fourWay);
    const coords size(18, 18);
    std::shared_ptr<bElem> plr;
    auto board = roomWithPlayer(size, coords(9, 9), plr);
    auto gun = put<plainGun>(board, coords(9, 8), 1); // never runs out
    plr->collect(gun);
    // everything that moves: monsters with each controller, drones, bricks to push, a bunker that shoots
    for (int s = 0; s < 6; s++)
        put<monster>(board, coords(2 + 2 * s, 2), s);
    for (int s = 0; s < 3; s++)
        put<patrollingDrone>(board, coords(3 + 4 * s, 15), s);
    for (int x = 6; x < 13; x += 3)
        put<brickCluster>(board, coords(x, 11));
    put<bunker>(board, coords(15, 9));
    std::mt19937 pick(31337);
    auto before = whoIsWhere(*board, size);
    int moves = 0;
    for (int t = 0; t < 3000; t++) {
        // the player presses any of the nine ways, walking or shooting
        const auto d = static_cast<dir::direction>(pick() % 9);
        inputManager::getInstance().setControlItem(controlItem(pick() % 4 == 0 ? 1 : 0, d));
        bElem::runLiveElements();
        const auto after = whoIsWhere(*board, size);
        for (const auto &[id, was] : before)
            if (const auto now = after.find(id); now != after.end() && !(now->second == was)) {
                const coords by = now->second - was;
                moves++;
                EXPECT_FALSE(std::abs(by.x) == 1 && std::abs(by.y) == 1)
                    << "element " << id << " went diagonally from (" << was.x << "," << was.y << ") at tick " << t;
            }
        before = after;
    }
    EXPECT_GT(moves, 100) << "the test needs things that move";
    inputManager::getInstance().setControlItem(controlItem(-1, dir::direction::NODIRECTION));
    if (const auto p = player::getActivePlayer())
        p->disposeElement();
}
