#include <boost/optional/optional.hpp>
#include <array>
#include <vector>

#include "Util/Arc_Assert.hpp"

namespace Quoridor {

  const int BOARD_SIZE = 9;
  const int BOARD_BYTES = (BOARD_SIZE * (BOARD_SIZE - 1) * 2 + 8 * 8) / 8;

  enum Direction : int8_t {
    UP,
    DOWN,
    LEFT,
    RIGHT
  };

  enum WallOrientation : int8_t {
    HORIZONTAL,
    VERTICAL
  };

  enum Player : int8_t {
    PLAYER_ONE = 0,
    PLAYER_TWO = 1
  };

  const int8_t MASK_HALF_BYTE = 0x0F;
  // Point class that supports only the range needed for the game. Ie. [0,9) x [0,9)
  // first 4 bits are X, next 4 bits are Y.
  class Point {
  public:
    Point(int8_t x, int8_t y)
      : pos((x & MASK_HALF_BYTE) | ((y & MASK_HALF_BYTE) << 4))
    {}

    inline int8_t x() const {
      return pos & MASK_HALF_BYTE;
    }
    inline int8_t y() const {
      return (pos >> 4) & MASK_HALF_BYTE;
    }

    inline bool operator==(const Point& other) const {
      return other.pos == this->pos;
    }
  private:
    int8_t pos;
  };

  const int8_t STARTING_WALL_COUNTS = 10;
  // Like the point class this only  supports the range of walls needed for the game. Ie. [0,10) x [0,10)
  // first 4 bits are p1 walls, next 4 bits are p2 walls.
  class WallCounts {
  public:
    WallCounts()
      : _counts((STARTING_WALL_COUNTS & MASK_HALF_BYTE) | ((STARTING_WALL_COUNTS & MASK_HALF_BYTE) << 4))
    {}

    inline int8_t wallCountForPlayer(Player player) const {
      return (_counts >> (4 * player)) & MASK_HALF_BYTE;
    }
    inline void decrementWallCountForPlayer(Player player) {
      int8_t wallCount = wallCountForPlayer(player);
      ARC_ASSERT(wallCount > 0);
      --wallCount;
      wallCount <<= (4 * player);
      const int8_t CLEAR_MASK = ~(MASK_HALF_BYTE << (4 * player));
      _counts = (_counts & CLEAR_MASK) | wallCount;
    }
  private:
    int8_t _counts;
  };

  enum MoveType : int8_t {
    MOVE_PIECE = 0,
    PLACE_HORIZONAL_WALL = 1,
    PLACE_VERTICAL_WALL = 2
  };

  union MoveInfo {
    MoveInfo(Direction);
    MoveInfo(Point);

    Direction pieceMoveDirection;
    Point wallCenter;
  };

  class Move {
  public:
    Move(Player player, Direction direction);
    Move(Player player, MoveType orientation, Point wallCenter);

    bool operator==(const Move& other) const;
    bool operator<(const Move& other) const;

    Player player;
    MoveType type;
    MoveInfo info;
  };

  class Wall {
  public:
    int centerX;
    int centerY;
    bool isVertical;

    bool operator==(const Wall& other) const;
    bool operator<(const Wall& other) const;
  };

  class WallsState {
  public:
    WallsState();

    void placeWall(int8_t centerX, int8_t centerY, MoveType type);
    std::vector<Wall> walls() const;
  private:
    std::array<int8_t, BOARD_BYTES> _state;
  };
  
  class Board final {
  public:
    Board();

    // For visualization
    Point playerPosition(Player player) const;
    int wallCount(Player player) const;
    std::vector<Wall> walls() const;

    // For changing state
    const std::vector<Move> availableMoves(Player player);
    void doMove(const Move& move);
  private:
    Point _playerOnePosition;
    Point _playerTwoPosition;
    WallCounts _playerWalls;
    WallsState _wallsState;
  };
}