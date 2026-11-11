#pragma once

#include <string>

namespace snake {

class Game;

std::string render(const Game& game);
std::string renderTerminalSizeWarning(int columns, int rows);

}  // namespace snake
