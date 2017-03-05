#include "stdafx.h"
#include "CppUnitTest.h"

#include <algorithm>
#include <numeric>
#include <string>

#include "Board.hpp"

using namespace Quoridor;
using namespace std;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

template <> static wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString(const Point& p) {
  return wstring(L"Point (") + to_wstring(p.x()) + L", " + to_wstring(p.y()) + L")";
}

template <> static wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString(const vector<Wall>& walls) {
  return accumulate(begin(walls), end(walls), wstring(L"Walls:\n"), [](wstring str, const Wall& wall) {
    return str + wstring(L"  { x:") + to_wstring(wall.centerX) + L", y:" + to_wstring(wall.centerY) + L", isVertical:" + to_wstring(wall.isVertical) + L" }\n";
  });
}

static const int MAX_POSSIBLE_WALL_POSITIONS = 128;
class WallMoveGenerator {
public:
  WallMoveGenerator(Player player);
  Move operator()();
private:
  Player _player;
  int _nextWallNumber;
  MoveType _nextWallOrientation;
};

WallMoveGenerator::WallMoveGenerator(Player player)
  : _nextWallNumber(0)
  , _nextWallOrientation(PLACE_HORIZONAL_WALL)
  , _player(player)
{ }

Move WallMoveGenerator::operator()() {
  Move m{ _player, _nextWallOrientation, { (int8_t)_nextWallNumber % (BOARD_SIZE - 1), (int8_t)_nextWallNumber / (BOARD_SIZE - 1)} };
  ++_nextWallNumber;
  if (_nextWallNumber >= MAX_POSSIBLE_WALL_POSITIONS / 2) {
    _nextWallNumber = 0;
    if (_nextWallOrientation == PLACE_HORIZONAL_WALL) {
      _nextWallOrientation = PLACE_VERTICAL_WALL;
    }
    else {
      _nextWallOrientation = PLACE_HORIZONAL_WALL;
    }
  }
  return m;
}

static vector<Move> allPossibleWallMoves(Player player) {
  vector<Move> allPossible{ (uint32_t)MAX_POSSIBLE_WALL_POSITIONS };
  generate_n(begin(allPossible), MAX_POSSIBLE_WALL_POSITIONS, WallMoveGenerator(player));
  sort(begin(allPossible), end(allPossible));
  return allPossible;
}

namespace Tests
{		
  TEST_CLASS(BoardTest)
  {
  public:

    TEST_METHOD(TestPoint) {
      auto p0 = Point(0, 0);
      auto p1 = Point(5, 5);
      auto p2 = Point(3, 8);

      Assert::AreEqual(p0.x(), (int8_t)0);
      Assert::AreEqual(p0.y(), (int8_t)0);
      Assert::AreEqual(p1.x(), (int8_t)5);
      Assert::AreEqual(p1.y(), (int8_t)5);
      Assert::AreEqual(p2.x(), (int8_t)3);
      Assert::AreEqual(p2.y(), (int8_t)8);
    }

    TEST_METHOD(TestWallCounts) {
      WallCounts counts;
      Assert::AreEqual(counts.wallCountForPlayer(PLAYER_ONE), (int8_t)10);
      Assert::AreEqual(counts.wallCountForPlayer(PLAYER_TWO), (int8_t)10);
      counts.decrementWallCountForPlayer(PLAYER_ONE);
      Assert::AreEqual(counts.wallCountForPlayer(PLAYER_ONE), (int8_t)9);
      Assert::AreEqual(counts.wallCountForPlayer(PLAYER_TWO), (int8_t)10);
      counts.decrementWallCountForPlayer(PLAYER_TWO);
      counts.decrementWallCountForPlayer(PLAYER_ONE);
      Assert::AreEqual(counts.wallCountForPlayer(PLAYER_ONE), (int8_t)8);
      Assert::AreEqual(counts.wallCountForPlayer(PLAYER_TWO), (int8_t)9);
    }
    
    TEST_METHOD(TestInitialState) {
      Board defaultBoard;

      Assert::AreEqual(defaultBoard.playerPosition(PLAYER_ONE), Point(4, 0));
      Assert::AreEqual(defaultBoard.playerPosition(PLAYER_TWO), Point(4, 8));
      Assert::AreEqual(defaultBoard.wallCount(PLAYER_ONE), 10);
      Assert::AreEqual(defaultBoard.wallCount(PLAYER_TWO), 10);
      Assert::AreEqual(defaultBoard.walls(), vector<Wall>{});

      defaultBoard.availableMoves(PLAYER_ONE);
      vector<Move> playerOneInitialMoves {
        {PLAYER_ONE, LEFT },
        { PLAYER_ONE, RIGHT },
        { PLAYER_ONE, DOWN },
      };
    }

    TEST_METHOD(TestWallState) {
      WallsState state;
      state.placeWall(5, 3, PLACE_HORIZONAL_WALL);
      state.placeWall(0, 0, PLACE_VERTICAL_WALL);
      state.placeWall(7, 7, PLACE_VERTICAL_WALL);
      vector<Wall> expectedWalls = {
        {5, 3, false},
        {0, 0, true},
        {7, 7, true}
      };
      auto actualWalls = state.walls();
      sort(begin(actualWalls), end(actualWalls));
      sort(begin(expectedWalls), end(expectedWalls));
      Assert::AreEqual(actualWalls, expectedWalls);
    }

  };
}