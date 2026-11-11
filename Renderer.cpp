#include "Renderer.hpp"

#include "Game.hpp"

#include <array>
#include <sstream>
#include <string>

namespace snake {

namespace {

std::size_t cellIndex(Point point) {
    return static_cast<std::size_t>(point.y * kBoardWidth + point.x);
}

}  // namespace

std::string render(const Game& game) {
    std::array<char, kBoardArea> cells;
    cells.fill(' ');
    cells[cellIndex(game.fruit())] = '@';

    for (const Point segment : game.snake()) {
        cells[cellIndex(segment)] = 'o';
    }
    cells[cellIndex(game.snake().front())] = 'O';

    std::ostringstream screen;
    screen << '+' << std::string(kBoardWidth, '-') << "+\n";
    for (int y = 0; y < kBoardHeight; ++y) {
        screen << '|';
        for (int x = 0; x < kBoardWidth; ++x) {
            screen << cells[cellIndex({x, y})];
        }
        screen << "|\n";
    }

    screen << '+' << std::string(kBoardWidth, '-') << "+\n";
    screen << "Score: " << game.score() << "   WASD: move   Q/X: quit\n";

    if (game.won()) {
        screen << "You filled the board. You win!\n";
    } else if (game.gameOver()) {
        screen << "Game over! Final score: " << game.score() << "\n";
    } else if (game.direction() == Direction::stopped) {
        screen << "Press W, A, S, or D to start.\n";
    } else {
        screen << "                                  \n";
    }

    return screen.str();
}

std::string renderTerminalSizeWarning(int columns, int rows) {
    std::ostringstream message;
    message << "Terminal is too small (" << columns << 'x' << rows << ").\n"
            << "Resize it to at least " << kBoardWidth + 2 << 'x'
            << kBoardHeight + 4 << ". Press Q to quit.\n";
    return message.str();
}

}  // namespace snake
