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
  auto allMoves = availablePieceMovesForPlayer(player);
  allMoves.reserve(132); // Maximum possible moves at any point
  auto wallMoves = availableWallPlacementsForPlayer(player);
  copy(begin(wallMoves), end(wallMoves), back_inserter(allMoves));
  return allMoves;
}

void Board::doMove(const Move& move) {

}

vector<Move> Board::availablePieceMovesForPlayer(Player player) {
  vector<Move> moves;
  moves.reserve(5); // max possible moves from any given position.
  // TODO::JT logic does not yet account for collisions or jumping rules.
  const Point playerPosition = player == PLAYER_ONE ? _playerOnePosition : _playerTwoPosition;

  // check board bounds
  if (playerPosition.x() != 0) {
    moves.push_back({ player, LEFT });
  }
  if (playerPosition.x() != BOARD_SIZE - 1) {
    moves.push_back({ player, RIGHT });
  }
  if (playerPosition.y() != 0) {
    moves.push_back({ player, UP });
  }
  if (playerPosition.y() != BOARD_SIZE - 1) {
    moves.push_back({ player, DOWN });
  }
  return moves;
}

vector<Move> Board::availableWallPlacementsForPlayer(Player player) {
  // TODO::JT logic does not account for pathing errors caused by bad walls.
  return _wallsState.availableWallPlacements(player);
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
  if (other.player != this->player) {
    return false;
  }
  if (other.type != this->type) {
    return false;
  }
  if (this->type == MOVE_PIECE) {
    return other.info.pieceMoveDirection == this->info.pieceMoveDirection;
  }
  else {
    return other.info.wallCenter == this->info.wallCenter;
  }
}

bool Move::operator<(const Move& other) const {
  if (other.player != this->player) {
    return this->player < other.player;
  }
  if (other.type != this->type) {
    return this->type < other.type;
  }
  if (this->type == MOVE_PIECE) {
    return this->info.pieceMoveDirection < other.info.pieceMoveDirection;
  }
  else {
    return this->info.wallCenter < other.info.wallCenter;
  }
}

//////////////////////////////////////////////////////////////////////////
// Wall State
//////////////////////////////////////////////////////////////////////////

WallsState::WallsState() {
  fill(begin(_state), end(_state), 0);
}

static inline int8_t wallNumber(int8_t x, int8_t y) {
  return x + y * (BOARD_SIZE - 1);
}

static inline int8_t xCordinate(int8_t wallNumber) {
  return wallNumber % (BOARD_SIZE - 1);
}

static inline int8_t yCordinate(int8_t wallNumber) {
  return wallNumber / (BOARD_SIZE - 1);
}

static const int8_t NO_WALL = 0x00;
static const int8_t HORIZONTAL_WALL = PLACE_HORIZONAL_WALL;
static const int8_t VERTICAL_WALL = PLACE_VERTICAL_WALL;
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

std::vector<Wall> WallsState::walls() const {
  std::vector<Wall> walls;
  const int totalCenterPoints = (BOARD_SIZE - 1) * (BOARD_SIZE - 1);
  for (int pointNumber = 0; pointNumber < totalCenterPoints; ++pointNumber) {
    const int8_t value = wallValue(pointNumber);
    const int xCord = xCordinate(pointNumber);
    const int yCord = yCordinate(pointNumber);
    if (value == VERTICAL_WALL) {
      walls.push_back({ xCord, yCord, true });
    }
    else if (value == HORIZONTAL_WALL) {
      walls.push_back({ xCord, yCord, false });
    }
  }
  return walls;
}

vector<Move> WallsState::availableWallPlacements(Player player) const {
  vector<Move> moves;
  moves.reserve(128); // max number of possible wall placements.
  const int totalCenterPoints = (BOARD_SIZE - 1) * (BOARD_SIZE - 1);
  for (int pointNumber = 0; pointNumber < totalCenterPoints; ++pointNumber) {
    const int8_t value = wallValue(pointNumber);
    if (value == NO_WALL) {
      const int8_t x = pointNumber % (BOARD_SIZE - 1);
      const int8_t y = pointNumber / (BOARD_SIZE - 1);

      // if vertical there can't be a wall above or below.
      // if horizontal there can't be a wall left or right.
      auto upWallNumber = wallNumber(x, y - 1);
      auto downWallNumber = wallNumber(x, y + 1);
      auto leftWallNumber = wallNumber(x - 1, y);
      auto rightWallNumber = wallNumber(x + 1, y);

      // check if a vertical wall is valid here
      if (wallValue(upWallNumber) != VERTICAL_WALL && wallValue(downWallNumber) != VERTICAL_WALL) {
        moves.push_back({ player, PLACE_VERTICAL_WALL, {x, y} });
      }
      // next up is if a horizontal wall is valid here
      if (wallValue(leftWallNumber) != HORIZONTAL_WALL && wallValue(rightWallNumber) != HORIZONTAL_WALL) {
        moves.push_back({ player, PLACE_HORIZONAL_WALL,{ x, y } });
      }
    }
  }
  return moves;
}

bool Quoridor::Wall::operator==(const Wall& other) const {
  return this->centerX == other.centerX &&
         this->centerY == other.centerY &&
         this->isVertical == other.isVertical;
}

bool Quoridor::Wall::operator<(const Wall& other) const {
  return wallNumber(this->centerX, this->centerY) < wallNumber(other.centerX, other.centerY);
}
