#include <graphics.h> 
#include <conio.h> 
#include <Windows.h> 
#include <iostream> 
#include <mmsystem.h> 
#include <ctime> 
#pragma comment(lib, "winmm.lib") 

// Game er constant gulo
const int CELL_SIZE = 50;
const int ROWS = 10;
const int COLS = 15;
const int WINDOW_W = COLS * CELL_SIZE;
const int WINDOW_H = ROWS * CELL_SIZE;

// Game er elements
#define WALL '#'
#define PATH ' '
#define PLAYER 'P'
#define EXIT 'E'
#define HEALTH 'H'
#define COIN 'C'
#define MONSTER 'M'

// Level system er additions
const int MAX_LEVELS = 3;
int currentLevel = 0;

// 3 ti level er example mazes
char levelMazes[MAX_LEVELS][ROWS][COLS] = {
    { // Level 1
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
    },
    // ... (rest of the level definitions remain same)
};

// Background music chalano
void playBackgroundMusic() {
    mciSendString(_T("open \"sounds/background.wav\" type mpegvideo alias bgmusic"), NULL, 0, NULL);
    mciSendString(_T("play bgmusic repeat"), NULL, 0, NULL);
}

// Sound effect chalano
void playSoundEffect(const TCHAR* soundFile, bool stopBackground = false) {
    if (stopBackground) {
        mciSendString(_T("pause bgmusic"), NULL, 0, NULL);
    }
    mciSendString(_T("close effect"), NULL, 0, NULL);
    TCHAR cmd[256];
    _stprintf_s(cmd, _T("open \"%s\" type mpegvideo alias effect"), soundFile);
    mciSendString(cmd, NULL, 0, NULL);
    mciSendString(_T("play effect"), NULL, 0, NULL);
}

// Background music abar shuru kora
void resumeBackgroundMusic() {
    mciSendString(_T("resume bgmusic"), NULL, 0, NULL);
}

// Shob sound bondho kora
void stopAllSounds() {
    mciSendString(_T("close all"), NULL, 0, NULL);
}

char maze[ROWS][COLS];

// Player er position ar stats
int playerRow = 1, playerCol = 1;
bool facingRight = true;
int playerHealth = 100;
const int MAX_HEALTH = 100;
int playerScore = 0;

// Monster er shongkhya
const int MAX_MONSTERS = 10;
int monsterCount = MAX_MONSTERS;
int monsterPos[MAX_MONSTERS][2] = { {0, 0} };

// Monster er health ar damage
const int MONSTER_MAX_HEALTH = 30;
const int PLAYER_ATTACK_DAMAGE = 15;
const int MONSTER_ATTACK_DAMAGE = 25;
int monsterHealth[MAX_MONSTERS] = { MONSTER_MAX_HEALTH };
int monsterType[MAX_MONSTERS] = { 1 }; // 1: normal, 2: monster2, 3: monster3

// Asset gulo
IMAGE imgFloor, imgDoor, imgWallH, imgWallV, imgPlayerL, imgPlayerR, imgHealth, imgDefeat, imgCoin, imgMonster, imgMonster2, imgMonster3;
IMAGE imgMainMenu;

// Main menu dekhao
void showMainMenu() {
    loadimage(&imgMainMenu, _T("images/main_menu.png"));
    
    mciSendString(_T("close all"), NULL, 0, NULL);
    mciSendString(_T("open \"sounds/main_menu.wav\" type mpegvideo alias mainmenu"), NULL, 0, NULL);
    mciSendString(_T("play mainmenu repeat"), NULL, 0, NULL);
    
    cleardevice();
    setbkcolor(BLACK);
    clearcliprgn();
    
    int x = (WINDOW_W - imgMainMenu.getwidth()) / 2;
    int y = (WINDOW_H - imgMainMenu.getheight()) / 2;
    putimage(x, y, &imgMainMenu);
    
    settextcolor(WHITE);
    settextstyle(24, 0, _T("Arial"));
    setbkmode(TRANSPARENT);
    
    int textWidth = textwidth(_T("Press SPACE to start"));
    int textX = (WINDOW_W - textWidth) / 2;
    outtextxy(textX, y + imgMainMenu.getheight() + 30, _T("Press SPACE to start"));
    
    FlushBatchDraw();
    
    // Spacebar chhara wait koro
    while (true) {
        if (_kbhit()) {
            int ch = _getch();
            if (ch == ' ' || ch == VK_SPACE) {
                break;
            }
        }
        Sleep(10);
    }
    
    mciSendString(_T("close mainmenu"), NULL, 0, NULL);
}

// Asset gulo load kora
void loadAssets() {
    loadimage(&imgFloor, _T("images/floor.png"));
    loadimage(&imgDoor, _T("images/door.png"));
    loadimage(&imgWallH, _T("images/wall_horizontal.png"));
    loadimage(&imgWallV, _T("images/wall_vertical.png"));
    loadimage(&imgPlayerL, _T("images/player_left.png"));
    loadimage(&imgPlayerR, _T("images/player_right.png"));
    loadimage(&imgHealth, _T("images/health.png"));
    loadimage(&imgDefeat, _T("images/defeat_template.png"));
    loadimage(&imgCoin, _T("images/coin.png"));
    loadimage(&imgMonster, _T("images/monster.png"));
    loadimage(&imgMonster2, _T("images/monster2.png"));
    loadimage(&imgMonster3, _T("images/monster3.png"));
}

// Game draw kora
void drawGame() {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            int x = col * CELL_SIZE;
            int y = row * CELL_SIZE;
            putimage(x, y, &imgFloor);
            
            switch (maze[row][col]) {
                case WALL: {
                    bool isHorizontal = false;
                    if (col > 0 && col < COLS - 1) {
                        isHorizontal = (maze[row][col - 1] == PATH || maze[row][col + 1] == PATH);
                    }
                    putimage(x, y, isHorizontal ? &imgWallH : &imgWallV);
                    break;
                }
                // ... (rest of the drawing code remains same)
            }
        }
    }
    
    // Health bar draw kora
    setfillcolor(RED);
    fillrectangle(10, 10, 110, 30);
    setfillcolor(GREEN);
    fillrectangle(10, 10, 10 + playerHealth, 30);
    
    // Health text
    settextstyle(16, 0, _T("Arial"));
    setbkmode(TRANSPARENT);
    wchar_t healthText[20];
    swprintf_s(healthText, L"HP: %d/%d", playerHealth, MAX_HEALTH);
    outtextxy(120, 10, healthText);
    
    // Score dekhao
    settextstyle(16, 0, _T("Arial"));
    wchar_t scoreText[20];
    swprintf_s(scoreText, L"Score: %d", playerScore);
    outtextxy(WINDOW_W - 150, 10, scoreText);
}

// Player move kora
bool movePlayer(char dir) {
    int newRow = playerRow, newCol = playerCol;
    
    switch (dir) {
        case 'w': newRow--; break;
        case 's': newRow++; break;
        case 'a': newCol--; facingRight = false; break;
        case 'd': newCol++; facingRight = true; break;
    }
    
    // Boundary check
    if (newRow < 0 || newRow >= ROWS || newCol < 0 || newCol >= COLS) {
        return false;
    }
    
    if (maze[newRow][newCol] != WALL) {
        bool won = (maze[newRow][newCol] == EXIT);
        bool gotHealth = (maze[newRow][newCol] == HEALTH);
        bool gotCoin = (maze[newRow][newCol] == COIN);
        
        if (maze[playerRow][playerCol] != EXIT) 
            maze[playerRow][playerCol] = PATH;
            
        playerRow = newRow;
        playerCol = newCol;
        
        if (gotHealth) {
            playerHealth = min(MAX_HEALTH, playerHealth + 20);
            playSoundEffect(_T("sounds/health.wav"));
        }
        else if (gotCoin) {
            playerScore += 10;
            playSoundEffect(_T("sounds/coin.wav"));
        }
        
        if (!won) maze[playerRow][playerCol] = PLAYER;
        return won;
    }
    return false;
}

// Monster gulo move kora
void moveMonsters() {
    for (int i = 0; i < monsterCount; ++i) {
        int& mRow = monsterPos[i][0];
        int& mCol = monsterPos[i][1];
        
        // Dead monster skip koro
        if (mRow == -1 && mCol == -1) continue;
        
        if (maze[mRow][mCol] == MONSTER) 
            maze[mRow][mCol] = PATH;
            
        // Simple AI: player er dike move koro
        int dr = 0, dc = 0;
        if (playerRow < mRow) dr = -1;
        else if (playerRow > mRow) dr = 1;
        
        if (playerCol < mCol) dc = -1;
        else if (playerCol > mCol) dc = 1;
        
        // ... (rest of the monster movement code remains same)
    }
}

// Monster ke attack kora
void attackMonster() {
    for (int i = 0; i < monsterCount; ++i) {
        int mRow = monsterPos[i][0];
        int mCol = monsterPos[i][1];
        
        // Player er pashe thakle attack koro
        if ((abs(mRow - playerRow) + abs(mCol - playerCol)) == 1) {
            monsterHealth[i] -= PLAYER_ATTACK_DAMAGE;
            if (monsterHealth[i] <= 0) {
                playSoundEffect(_T("sounds/monster_death.wav"));
                maze[mRow][mCol] = PATH;
                monsterPos[i][0] = -1;
                monsterPos[i][1] = -1;
            }
            break;
        }
    }
}

// ... (rest of the functions remain same with Bengali comments)

int main() {
    initgraph(WINDOW_W, WINDOW_H);
    loadAssets();
    BeginBatchDraw();
    
    showMainMenu();
    currentLevel = 0;
    loadLevel(currentLevel);
    playBackgroundMusic();
    
    while (true) {
        cleardevice();
        drawGame();
        
        if (playerHealth <= 0) {
            showDefeatTemplate();
            break;
        }
        
        ExMessage msg;
        if (peekmessage(&msg, EM_KEY)) {
            if (msg.message == WM_KEYDOWN) {
                char input = tolower(msg.vkcode);
                
                if (input == 'q') break;
                
                if (msg.vkcode == VK_LEFT) input = 'a';
                if (msg.vkcode == VK_RIGHT) input = 'd';
                if (msg.vkcode == VK_UP) input = 'w';
                if (msg.vkcode == VK_DOWN) input = 's';
                
                if (input == ' ') {
                    attackMonster();
                    moveMonsters();
                }
                else if (movePlayer(input)) {
                    moveMonsters();
                    if (maze[playerRow][playerCol] == EXIT) {
                        currentLevel++;
                        if (currentLevel < MAX_LEVELS) {
                            loadLevel(currentLevel);
                            continue;
                        }
                        else {
                            showVictoryTemplate();
                            while (_getch() != 'q');
                            break;
                        }
                    }
                }
                else {
                    moveMonsters();
                }
            }
        }
        
        FlushBatchDraw();
        Sleep(10);
    }
    
    EndBatchDraw();
    closegraph();
    stopAllSounds();
    return 0;
}