#include <graphics.h>
#include <conio.h>
#include <Windows.h>
#include <iostream>
#pragma comment(lib, "winmm.lib")

// Game constants
const int CELL_SIZE = 50;
const int ROWS = 10;
const int COLS = 15;

// Game elements
#define WALL '#'
#define PATH ' '
#define PLAYER 'P'
#define EXIT 'E'

char maze[ROWS][COLS] = {
    {'#','#','#','#','#','#','#','#','#','#','#','#','#','#','#'},
    {'#','P',' ',' ','#',' ',' ',' ','#',' ',' ',' ',' ',' ','#'},
    {'#','#','#',' ','#',' ','#',' ','#',' ','#','#','#',' ','#'},
    {'#',' ',' ',' ',' ',' ','#',' ',' ',' ','#',' ',' ',' ','#'},
    {'#',' ','#','#','#','#','#','#','#',' ','#',' ','#','#','#'},
    {'#',' ',' ',' ','#',' ',' ',' ',' ',' ','#',' ',' ',' ','#'},
    {'#','#','#',' ','#',' ','#','#','#','#','#',' ','#',' ','#'},
    {'#',' ',' ',' ','#',' ',' ',' ',' ',' ',' ',' ','#',' ','#'},
    {'#',' ','#',' ',' ',' ','#','#','#','#','#','#','#','E','#'},
    {'#','#','#','#','#','#','#','#','#','#','#','#','#','#','#'}
};

int playerRow = 1, playerCol = 1;
bool facingRight = true;

// Assets
IMAGE imgFloor, imgDoor, imgWallH, imgWallV, imgPlayerL, imgPlayerR, imgVictory;

void loadAssets() {
    // Load images - NO RETURN VALUE CHECK (loadimage returns void)
    loadimage(&imgFloor, _T("images/floor.png"));
    loadimage(&imgDoor, _T("images/door.png"));
    loadimage(&imgWallH, _T("images/wall_horizontal.png"));
    loadimage(&imgWallV, _T("images/wall_vertical.png"));
    loadimage(&imgPlayerL, _T("images/player_left.png"));
    loadimage(&imgPlayerR, _T("images/player_right.png"));
    loadimage(&imgVictory, _T("images/victory.png"));

    // PROPER ERROR CHECKING - Verify images loaded by checking width
    if (imgFloor.getwidth() == 0 || imgDoor.getwidth() == 0) {
        MessageBox(NULL, _T("Failed to load game assets!"), _T("Error"), MB_ICONERROR);
        exit(1);
    }

    PlaySound(_T("sounds/background.mp3"), NULL, SND_ASYNC | SND_LOOP);
}

bool isWallHorizontal(int row, int col) {
    return (maze[row][col - 1] == PATH || maze[row][col + 1] == PATH);
}

void drawGame() {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            int x = col * CELL_SIZE;
            int y = row * CELL_SIZE;

            putimage(x, y, &imgFloor);

            switch (maze[row][col]) {
            case WALL:
                putimage(x, y, isWallHorizontal(row, col) ? &imgWallH : &imgWallV);
                break;
            case EXIT:
                putimage(x, y, &imgDoor);
                break;
            case PLAYER:
                int offsetX = (CELL_SIZE - 30) / 2;
                int offsetY = (CELL_SIZE - 30) / 2;
                putimage(x + offsetX, y + offsetY, facingRight ? &imgPlayerR : &imgPlayerL);
                break;
            }
        }
    }
}

bool movePlayer(char dir) {
    int newRow = playerRow, newCol = playerCol;
    switch (dir) {
    case 'w': newRow--; break;
    case 's': newRow++; break;
    case 'a': newCol--; facingRight = false; break;
    case 'd': newCol++; facingRight = true; break;
    }

    if (maze[newRow][newCol] != WALL) {
        maze[playerRow][playerCol] = PATH;
        playerRow = newRow;
        playerCol = newCol;
        maze[playerRow][playerCol] = PLAYER;

        PlaySound(_T("sounds/footsteps.wav"), NULL, SND_ASYNC | SND_FILENAME);

        return (maze[newRow][newCol] == EXIT);
    }
    return false;
}

void showVictory() {
    closegraph();
    initgraph(512, 512);
    putimage(0, 0, &imgVictory);

    PlaySound(_T("sounds/victory.wav"), NULL, SND_ASYNC | SND_FILENAME);

    settextcolor(YELLOW);
    settextstyle(36, 0, _T("Arial"));
    outtextxy(150, 450, _T("Press any key to exit"));

    _getch();
    closegraph();
}

int main() {
    initgraph(COLS * CELL_SIZE, ROWS * CELL_SIZE);
    loadAssets();
    BeginBatchDraw();

    while (true) {
        cleardevice();
        drawGame();
        FlushBatchDraw();

        if (_kbhit()) {
            char input = tolower(_getch());
            if (input == 'q') break;

            if (movePlayer(input)) {
                showVictory();
                break;
            }
        }
        Sleep(10);
    }

    EndBatchDraw();
    return 0;
}