# Snake

A terminal Snake game that builds natively on macOS, Linux, and Windows.

## macOS or Linux

    make run

Use W, A, S, and D to move. Press Q or X to quit.

Run the automated game-rule checks with:

    make test

## Windows

Open Snake.cbp in Code::Blocks and build the project, or compile the four source
files with a C++17 compiler:

    g++ -std=c++17 MAIN.cpp Game.cpp Renderer.cpp Terminal.cpp -o Snake.exe

Run the game in Windows Terminal, PowerShell, or Command Prompt.
