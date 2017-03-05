#include <optional>

namespace Quoridor {

  enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
  };

  enum class WallOrientation {
    HORIZONTAL,
    VERTICAL
  };

  enum class Player {
    PLAYER_ONE,
    PLAYER_TWO
  };

  class Point {
  public:
    int x;
    int y;
  };

  class Move {
  public:
    Player player;
    std::optional<Direction> pieceMoveDirection;
    std::optional<Point> wallCenter;
    WallOrientation wallDirection;
  };
  
  class Board {

  };
}