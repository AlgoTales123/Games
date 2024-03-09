#include <assert.h>
#include <iostream>
#include <vector>
using namespace std;

class GameState;
class GameBoard;
class Pacman;
class Ghost;
class Pallet;
class Game;

enum CellType { EmptyT, WallT, PalletT, PowerPalletT, PacmanT, GhostT };

enum class PalletType { Regular, Power };

enum class Signal { GameOver, StartNextLife };

enum class Direction { Up, Down, Left, Right };

class GameBoard {
private:
  int rows;
  int cols;
  vector<vector<CellType>> board;
  bool initalized;

  char getCharByCellType(CellType cty) const {
    switch (cty) {
    case CellType::EmptyT:
      return ' ';
    case CellType::GhostT:
      return 'G';
    case CellType::PacmanT:
      return 'P';
    case CellType::PalletT:
      return '.';
    case CellType::PowerPalletT:
      return 'O';
    case CellType::WallT:
      return '#';
    default:
      assert("unknown cell type requested.");
    }
  }

public:
  GameBoard(int rows, int cols) {
    initalized = true;
    this->rows = rows;
    this->cols = cols;
    board.resize(rows, vector<CellType>(cols, CellType::EmptyT));
  }

  void renderBoard() const {
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        cout << getCharByCellType(board[i][j]) << " ";
      }
      cout << '\n';
    }
  }

  bool isValidCell(unsigned x, unsigned y) {
    assert(initalized and "Board is uninitalized");
    return (x >= 0 and x < rows and y >= 0 and y < cols);
  }

  void updateCell(unsigned x, unsigned y, CellType cty) {
    assert(isValidCell(x, y) and "trying to update an invalid cell");
    board[x][y] = cty;
  }

  CellType getEntityAt(unsigned x, unsigned y) {
    assert(isValidCell(x, y) and "trying to access an invalid cell");
    return board[x][y];
  }

  unsigned getRows() {
    assert(initalized and "board was not initalized");
    return rows;
  }

  unsigned getCols() {
    assert(initalized and "board was not initalized");
    return cols;
  }
};

class GameState {
public:
  GameBoard *board;
  Pacman *pac;
  vector<Ghost> *ghosts;

  GameState(GameBoard *uBoard, Pacman *uPac, vector<Ghost> *uGhosts) {
    this->ghosts = uGhosts;
    this->board = uBoard;
    this->pac = uPac;
  }
};

class Pallet {
private:
  unsigned row;
  unsigned col;
  PalletType pty;

public:
  Pallet(int row, int col, PalletType pty) {
    this->row = row;
    this->col = col;
    this->pty = pty;
  }

  pair<unsigned, unsigned> getPosition() { return make_pair(row, col); }

  PalletType getType() const { return pty; }
};

class Pacman {
private:
  unsigned row;
  unsigned col;
  unsigned score;
  unsigned lives = 3;
  bool isAlive = true;

  void updatePosition(unsigned r, unsigned c) {
    this->row = r;
    this->col = c;
  }

public:
  Pacman(unsigned srow, unsigned scol) {
    row = srow;
    col = scol;
    score = 0;
  }

  pair<unsigned, unsigned> getPosition() const { return make_pair(row, col); }

  unsigned getScore() const {
    assert(pacLifeStatus() and "Pacman not live");
    return score;
  }

  bool pacLifeStatus() const { return (isAlive and lives >= 1); }

  void updateAliveStaus(bool status) { this->isAlive = status; }

  unsigned getLifeLeftCount() { return lives; }

  Signal killPacman() {
    assert(pacLifeStatus() and "Cannot be killed without a life left...");
    if (lives == 1) {
      lives = 0;
      isAlive = false;
      cout << "GAME OVER :(\n";
      return Signal::GameOver;
    } else {
      lives -= 1;
      isAlive = false;
      cout << "PACMAN KILLED STARTING OVER, LIVES LEFT : " << lives << "\n";
      return Signal::StartNextLife;
    }
  }

  void move(GameState *state, Direction dir) {
    assert(state->pac->pacLifeStatus() and "Pacman not alive");
    switch (dir) {
    case (Direction::Up): {
      handleUpMovement(state->board);
      break;
    }
    case (Direction::Down): {
      handleDownMovement(state->board);
      break;
    }
    case (Direction::Left): {
      handleLeftMovement(state->board);
      break;
    }
    case (Direction::Right): {
      handleRightMovement(state->board);
      break;
    }
    }
  }

  void resetPac(GameBoard &board, pair<unsigned, unsigned> pos) {
    unsigned x = pos.first;
    unsigned y = pos.second;
    assert(x >= 0 and x < board.getRows() and y >= 0 and y < board.getCols() and
           "Invalid pacman position");
    row = x;
    col = y;
    board.updateCell(row, col, CellType::PacmanT);
  }
};

class Ghost {
private:
  unsigned row;
  unsigned col;

  void resetGhost(GameBoard &board, pair<unsigned, unsigned> pos) {
    unsigned x = pos.first;
    unsigned y = pos.second;
    assert(x >= 0 and x < board.getRows() and y >= 0 and y < board.getCols() and
           "Invalid ghost initalization");
    row = x;
    col = y;
    board.updateCell(row, col, CellType::GhostT);
  }

public:
  Ghost(unsigned r, unsigned c) {
    row = r;
    col = c;
  }

  pair<unsigned, unsigned> getPosition() const { return make_pair(row, col); }
};

class Game {
private:
  GameState *state;

public:
  Game(GameState *startState) { state = startState; }

  void trivalInitalState() {
    GameBoard &board = *(state->board);
    unsigned bsize = board.getRows();
    for (int i = 0; i < bsize; i++) {
      board.updateCell(0, i, CellType::WallT);
      board.updateCell(5, i, CellType::WallT);
      board.updateCell(i, 0, CellType::WallT);
      board.updateCell(i, 5, CellType::WallT);
    }
    for (int i = 2; i <= 3; i++) {
      for (int j = 2; j <= 3; j++) {
        board.updateCell(i, j, CellType::WallT);
      }
    }
    state->pac->resetPac(board, state->pac->getPosition());
    vector<Ghost> &ghosts = *state->ghosts;
    for (int i = 0; i < ghosts.size(); i++) {
      pair<unsigned, unsigned> pos = ghosts[i].getPosition();
      board.updateCell(pos.first, pos.second, CellType::GhostT);
    }
    board.updateCell(4, 2, CellType::PowerPalletT);
    for (int i = 0; i < bsize; i++) {
      for (int j = 0; j < bsize; j++) {
        if (board.getEntityAt(i, j) == CellType::EmptyT)
          board.updateCell(i, j, CellType::PalletT);
      }
    }
    board.renderBoard();
  }
};

int main() {
  int s = 6;
  GameBoard *board = new GameBoard(s, s);
  Pacman *pac = new Pacman(1, 2);
  vector<Ghost> ghosts = {Ghost(2, 1), Ghost(3, 4)};
  GameState *state = new GameState(board, pac, &ghosts);
  Game *game = new Game(state);
  game->trivalInitalState();
}