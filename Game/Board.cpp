#include "pch.h"

#include "Board.hpp"

using namespace std;
using namespace Quoridor;

Board::Board()
  : _playerOnePosition(BOARD_SIZE / 2, 0)
  , _playerTwoPosition(BOARD_SIZE / 2, BOARD_SIZE - 1)
{ }

Point Board::playerPosition(Player player) const {
  switch (player) {
  case PLAYER_ONE:
    return _playerOnePosition;
  case PLAYER_TWO:
    return _playerTwoPosition;
  default:
    ARC_FAIL("There should only be 2 players and you asked for an invalid one!");
    return{-1,-1};
  }
}

int Board::wallCount(Player player) const {
  return _playerWalls.wallCountForPlayer(player);
}

const vector<Move> Board::availableMoves(Player player) {
  return{};
}

void Board::doMove(const Move& move) {

}

vector<Wall> Board::walls() const {
  return _wallsState.walls();
}

//////////////////////////////////////////////////////////////////////////
// Move
//////////////////////////////////////////////////////////////////////////

MoveInfo::MoveInfo(Direction dir)
  : pieceMoveDirection(dir)
{ }

MoveInfo::MoveInfo(Point p)
  : wallCenter(p)
{ }

Move::Move(Player p, Direction d)
  : player(p)
  , type(MOVE_PIECE)
  , info(d)
{ }

Move::Move(Player p, MoveType orientation, Point center)
  : player(p)
  , type(orientation)
  , info(center)
{ }

bool Move::operator==(const Move& other) const {

}

bool Move::operator<(const Move& other) const {

}

//////////////////////////////////////////////////////////////////////////
// Wall State
//////////////////////////////////////////////////////////////////////////

WallsState::WallsState() {
  fill(begin(_state), end(_state), 0);
}

static inline int8_t wallByteOffset(int8_t pointNumber) {
  return pointNumber / 4;
}

static inline int8_t wallBitOffset(int8_t pointNumber) {
  return (pointNumber % 4) * 2;
}

static inline int8_t wallNumber(int8_t x, int8_t y) {
  return x + y * (BOARD_SIZE - 1);
}

static const int8_t NO_WALL = 0x00;
static const int8_t HORIZONTAL_WALL = PLACE_HORIZONAL_WALL;
static const int8_t VERTICAL_WALL = PLACE_VERTICAL_WALL;
static const int8_t TWO_BIT_MASK = 0x03;
void WallsState::placeWall(int8_t centerX, int8_t centerY, MoveType type) {
  // Note: do not bother with validation. This is a dumb function that just assumes
  // the inputs are reasonable.
  const int8_t pointNumber = wallNumber(centerX, centerY);
  const int8_t byteNumber = wallByteOffset(pointNumber);
  const int8_t bitOffset = wallBitOffset(pointNumber);
  const int8_t clearMask = ~(TWO_BIT_MASK << bitOffset);
  const int8_t newValue = type << bitOffset;
  auto targetByte = _state.begin() + byteNumber;
  *targetByte = (*targetByte & clearMask) | newValue;
}

std::vector<Wall> Quoridor::WallsState::walls() const {
  std::vector<Wall> walls;
  const int totalCenterPoints = (BOARD_SIZE - 1) * (BOARD_SIZE - 1);
  for (int pointNumber = 0; pointNumber < totalCenterPoints; ++pointNumber) {
    const int8_t byteNumber = wallByteOffset(pointNumber);
    const int8_t bitOffset = wallBitOffset(pointNumber);
    const int8_t wallValue = (_state[byteNumber] >> bitOffset) & TWO_BIT_MASK;
    
    const int xCord = pointNumber % (BOARD_SIZE - 1);
    const int yCord = pointNumber / (BOARD_SIZE - 1);
    if (wallValue == VERTICAL_WALL) {
      walls.push_back({ xCord, yCord, true });
    }
    else if (wallValue == HORIZONTAL_WALL) {
      walls.push_back({ xCord, yCord, false });
    }
  }
  return walls;
}

bool Quoridor::Wall::operator==(const Wall& other) const {
  return this->centerX == other.centerX &&
         this->centerY == other.centerY &&
         this->isVertical == other.isVertical;
}

bool Quoridor::Wall::operator<(const Wall& other) const {
  return wallNumber(this->centerX, this->centerY) < wallNumber(other.centerX, other.centerY);
}
