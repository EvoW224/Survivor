#include <iostream>
#include "C:\raylib\raylib\src\raylib.h"
#include <cmath>
#include "C:\raylib\raylib\src\raymath.h"
#include <deque>
#include <cfloat>

using namespace std;

// Screen size values
const int screenWidth = 1200;
const int screenHeight = 800;

// Core Variables and Values
int mainMenuOption = 0;  // Selected option for main menu
bool inMainMenu = true;
bool isPaused = false;
bool inHowToPlayMenu = false;  
bool shouldExit = false;
int howToPlayPage = 0;
const int totalHowToPlayPages = 5;  
bool inGameplayMode = false;
bool inControlsMenu = false;
bool wasPaused = false;
bool isDeveloperMenu = false;
const int developerMenuKey = KEY_ENTER;
const int pauseKey = KEY_TAB;
Music backgroundMusic;
Music mainMenuMusic;
Music classSelectMusic;
Music gameplayMusic;
Music pauseMusic;
Music victoryMusic;
Music gameOverMusic;
float musicVolume = 0.5f;  
bool isFortified;            // Indicates if the player is in fortified position
float fortifiedCooldown;     // Cooldown for fortified ability
float lastFortifiedTime;     // Time since the last fortified ability activation
float fortifiedDuration; 

typedef enum MusicType {
    NONE,
    MAIN_MENU,
    CLASS_SELECT,
    GAMEPLAY,
    PAUSE,
    VICTORY,
    GAME_OVER
} MusicType;

typedef struct KeyDiagram {
    Vector2 position;
    int keySize;
    int keySpacing;
    const char* keys[5];
    int keyCount;
} KeyDiagram;

struct MouseDiagram {
    Vector2 position;
    int width;
    int height;
    int buttonHeight;
};

// Game Rules
int enemiesDefeated = 0;  // Tracks the number of enemies defeated
bool isVictory = false;
bool isGameOver = false;  // Tracks if the game is over
float survivalTime = 0.0f;  // Tracks how long the player has survived
float survivalGoal = 120.0f;  
// Shockwave effect variables
bool shockwaveActive = false;
float shockwaveRadius = 0.0f;
float shockwaveOpacity = 1.0f;
float shockwaveDuration = 1.0f;  // Shockwave lasts for 1 second
float shockwaveTimer = 0.0f;
Vector2 shockwavePosition = {0.0f, 0.0f}; // IDK

// Trail structure to store dash trail information
typedef struct DashTrail {
    Vector2 position;
    float opacity;
} DashTrail;

std::deque<DashTrail> dashTrail;

// Tank Class
typedef enum TankClass {
    ASSAULT,
    SCOUT,
    SPREAD
} TankClass;

// Player Structure
typedef struct Player {
    Vector2 position;
    const int EVOLUTION_1_MAX_LEVEL = 2;
    const int EVOLUTION_2_MIN_LEVEL = 3;
    const int EVOLUTION_3_MIN_LEVEL = 5;
    float radius;
    float speed;
    float baseSpeed;
    float rotation;
    int health;
    int maxHealth;
    Rectangle cannon;
    float fireRate;
    float lastShotTime;
    float healthBarVisibleTime;
    int level;
    int experience;
    int experienceNeeded;
    TankClass tankClass;
    float bulletDamage;       // Damage dealt by bullets
    int cannonCount;          // Number of cannons the player has
    float repulseCooldown;    // Cooldown for repulse ability (Assault class)
    float lastRepulseTime;    // Time since last repulse ability use
    float lastDashTime;       // Time since last dash (Scout class)
    bool isDashing;           // Whether the player is currently dashing (Scout class)
    bool isFortified;         // Indicates if the player is in fortified position
    bool speedModified;
    float fortifiedCooldown;  // Cooldown for fortified ability
    float lastFortifiedTime;  // Time since the last fortified ability activation
    float fortifiedDuration;  // Duration of the fortified state

    // New visual attributes for demo purposes
    Color playerColor;        // Color of the player based on the class
    float visualCooldown;     // Cooldown for visual animations (shockwave, dash, fortify)
    float lastVisualTime;     // Time since last visual animation
} Player;

// Declare players for demonstration purposes
Player assaultDemo, scoutDemo, spreadDemo;

Player player;

void InitClassDemos() {
    // Initialize demo player for Assault class
    assaultDemo.position = {400.0f, 400.0f};
    assaultDemo.tankClass = ASSAULT;
    assaultDemo.radius = 20.0f;
    assaultDemo.playerColor = RED;  // Assign distinct color
    assaultDemo.visualCooldown = 2.0f;  // Time between animations in seconds
    assaultDemo.lastVisualTime = 0.0f;

    // Initialize demo player for Scout class
    scoutDemo.position = {600.0f, 400.0f};
    scoutDemo.tankClass = SCOUT;
    scoutDemo.radius = 20.0f;
    scoutDemo.playerColor = GREEN;  // Assign distinct color
    scoutDemo.visualCooldown = 5.0f;  // Time between animations in seconds
    scoutDemo.lastVisualTime = 0.0f;

    // Initialize demo player for Spread class
    spreadDemo.position = {800.0f, 400.0f};
    spreadDemo.tankClass = SPREAD;
    spreadDemo.radius = 20.0f;
    spreadDemo.playerColor = BLUE;  // Assign distinct color
    spreadDemo.visualCooldown = 2.0f;  // Time between animations in seconds
    spreadDemo.lastVisualTime = 0.0f;
}

// Player contact damage to enemies
int playerContactDamage = 5;
int gridSpacing = 25;  // Distance between grid lines

// Spawn Mechanics
float spawnRate = 2.0f;  // Enemies spawn every 2 seconds at the start
float timeSinceLastSpawn = 0.0f;
float spawnRateDecrement = 0.05f;  // Spawn rate increases slightly over time
float minSpawnRate = 0.5f;  // Limit the spawn rate increase
bool classSelected = false;  // Indicates if the player has selected a class

// Menu Elements
int selectedOption = 0;
Rectangle classBox = {300, 175, 600, 500};

// Draw Controls Menu Function Definition

void DrawSurvivalTimer(float survivalTime, float survivalGoal) {
    // Define the timer bar dimensions
    float barWidth = 300.0f;
    float barHeight = 20.0f;
    float barX = (screenWidth / 2) - (barWidth / 2);
    float barY = 50.0f;

    // Calculate the percentage of the survival timer filled
    float fillPercentage = survivalTime / survivalGoal;

    // Draw the timer bar border
    DrawRectangle(barX - 2, barY - 2, barWidth + 4, barHeight + 4, BLACK);

    // Draw the timer bar fill in red
    DrawRectangle(barX, barY, barWidth * fillPercentage, barHeight, RED);

    // Draw the label above the timer bar
    DrawText("Time Until Rescue", barX + barWidth / 2 - MeasureText("Time Until Rescue", 20) / 2, barY - 30, 20, BLACK);
}

void DrawSpaceBar(Vector2 position, int width, int height) {
    DrawRectangle(position.x, position.y, width, height, DARKGRAY); // Bevel
    DrawRectangle(position.x + 4, position.y - 4, width - 8, height - 8, LIGHTGRAY); // Top Surface
    DrawText("Space", position.x + width / 2 - MeasureText("Space", 20) / 2, position.y + height / 2 - 10, 20, BLACK);
}

// Draw ESC Key Function
void DrawESCKey(Vector2 position, int keySize) {
    DrawRectangle(position.x, position.y, keySize, keySize, DARKGRAY); // Bevel
    DrawRectangle(position.x + 4, position.y - 4, keySize - 8, keySize - 8, LIGHTGRAY); // Top Surface
    DrawText("ESC", position.x + keySize / 2 - MeasureText("ESC", 20) / 2, position.y + keySize / 2 - 10, 20, BLACK);
}

// Draw TAB Key Function
void DrawTABKey(Vector2 position, int keySize) {
    DrawRectangle(position.x, position.y, keySize * 2, keySize, DARKGRAY); // Bevel
    DrawRectangle(position.x + 4, position.y - 4, keySize * 2 - 8, keySize - 8, LIGHTGRAY); // Top Surface
    DrawText("TAB", position.x + keySize - MeasureText("TAB", 20) / 2, position.y + keySize / 2 - 10, 20, BLACK);
}

// Draw ENTER Key Function
void DrawEnterKey(Vector2 position, int keySize) {
    DrawRectangle(position.x, position.y, keySize * 2, keySize, DARKGRAY); // Bevel
    DrawRectangle(position.x + 4, position.y - 4, keySize * 2 - 8, keySize - 8, LIGHTGRAY); // Top Surface
    DrawText("ENTER", position.x + keySize - MeasureText("ENTER", 20) / 2, position.y + keySize / 2 - 10, 20, BLACK);
}

void DrawWASDKeys(KeyDiagram wasdDiagram) {
    // Draw "W" Key
    DrawRectangle(wasdDiagram.position.x + wasdDiagram.keySize + wasdDiagram.keySpacing, wasdDiagram.position.y - wasdDiagram.keySize - wasdDiagram.keySpacing, wasdDiagram.keySize, wasdDiagram.keySize, DARKGRAY); // Bevel
    DrawRectangle(wasdDiagram.position.x + wasdDiagram.keySize + wasdDiagram.keySpacing + 4, wasdDiagram.position.y - wasdDiagram.keySize - wasdDiagram.keySpacing - 4, wasdDiagram.keySize - 8, wasdDiagram.keySize - 8, LIGHTGRAY); // Top Surface
    DrawText("W", wasdDiagram.position.x + wasdDiagram.keySize + wasdDiagram.keySpacing + wasdDiagram.keySize / 2 - MeasureText("W", 20) / 2, wasdDiagram.position.y - wasdDiagram.keySize - wasdDiagram.keySpacing + wasdDiagram.keySize / 2 - 10, 20, BLACK);

    // Draw "A" Key
    DrawRectangle(wasdDiagram.position.x, wasdDiagram.position.y, wasdDiagram.keySize, wasdDiagram.keySize, DARKGRAY); // Bevel
    DrawRectangle(wasdDiagram.position.x + 4, wasdDiagram.position.y - 4, wasdDiagram.keySize - 8, wasdDiagram.keySize - 8, LIGHTGRAY); // Top Surface
    DrawText("A", wasdDiagram.position.x + wasdDiagram.keySize / 2 - MeasureText("A", 20) / 2, wasdDiagram.position.y + wasdDiagram.keySize / 2 - 10, 20, BLACK);

    // Draw "S" Key
    DrawRectangle(wasdDiagram.position.x + wasdDiagram.keySize + wasdDiagram.keySpacing, wasdDiagram.position.y, wasdDiagram.keySize, wasdDiagram.keySize, DARKGRAY); // Bevel
    DrawRectangle(wasdDiagram.position.x + wasdDiagram.keySize + wasdDiagram.keySpacing + 4, wasdDiagram.position.y - 4, wasdDiagram.keySize - 8, wasdDiagram.keySize - 8, LIGHTGRAY); // Top Surface
    DrawText("S", wasdDiagram.position.x + wasdDiagram.keySize + wasdDiagram.keySpacing + wasdDiagram.keySize / 2 - MeasureText("S", 20) / 2, wasdDiagram.position.y + wasdDiagram.keySize / 2 - 10, 20, BLACK);

    // Draw "D" Key
    DrawRectangle(wasdDiagram.position.x + 2 * (wasdDiagram.keySize + wasdDiagram.keySpacing), wasdDiagram.position.y, wasdDiagram.keySize, wasdDiagram.keySize, DARKGRAY); // Bevel
    DrawRectangle(wasdDiagram.position.x + 2 * (wasdDiagram.keySize + wasdDiagram.keySpacing) + 4, wasdDiagram.position.y - 4, wasdDiagram.keySize - 8, wasdDiagram.keySize - 8, LIGHTGRAY); // Top Surface
    DrawText("D", wasdDiagram.position.x + 2 * (wasdDiagram.keySize + wasdDiagram.keySpacing) + wasdDiagram.keySize / 2 - MeasureText("D", 20) / 2, wasdDiagram.position.y + wasdDiagram.keySize / 2 - 10, 20, BLACK);
}

void DrawArrowKeys(KeyDiagram arrowDiagram) {
    // Draw "Up" Arrow Key
    DrawRectangle(arrowDiagram.position.x + arrowDiagram.keySize + arrowDiagram.keySpacing, arrowDiagram.position.y - arrowDiagram.keySize - arrowDiagram.keySpacing, arrowDiagram.keySize, arrowDiagram.keySize, DARKGRAY); // Bevel
    DrawRectangle(arrowDiagram.position.x + arrowDiagram.keySize + arrowDiagram.keySpacing + 4, arrowDiagram.position.y - arrowDiagram.keySize - arrowDiagram.keySpacing - 4, arrowDiagram.keySize - 8, arrowDiagram.keySize - 8, LIGHTGRAY); // Top Surface
    DrawText("^", arrowDiagram.position.x + arrowDiagram.keySize + arrowDiagram.keySpacing + arrowDiagram.keySize / 2 - MeasureText("^", 20) / 2, arrowDiagram.position.y - arrowDiagram.keySize - arrowDiagram.keySpacing + arrowDiagram.keySize / 2 - 10, 20, BLACK);

    // Draw "Left" Arrow Key
    DrawRectangle(arrowDiagram.position.x, arrowDiagram.position.y, arrowDiagram.keySize, arrowDiagram.keySize, DARKGRAY); // Bevel
    DrawRectangle(arrowDiagram.position.x + 4, arrowDiagram.position.y - 4, arrowDiagram.keySize - 8, arrowDiagram.keySize - 8, LIGHTGRAY); // Top Surface
    DrawText("<", arrowDiagram.position.x + arrowDiagram.keySize / 2 - MeasureText("<", 20) / 2, arrowDiagram.position.y + arrowDiagram.keySize / 2 - 10, 20, BLACK);

    // Draw "Down" Arrow Key
    DrawRectangle(arrowDiagram.position.x + arrowDiagram.keySize + arrowDiagram.keySpacing, arrowDiagram.position.y, arrowDiagram.keySize, arrowDiagram.keySize, DARKGRAY); // Bevel
    DrawRectangle(arrowDiagram.position.x + arrowDiagram.keySize + arrowDiagram.keySpacing + 4, arrowDiagram.position.y - 4, arrowDiagram.keySize - 8, arrowDiagram.keySize - 8, LIGHTGRAY); // Top Surface
    DrawText("v", arrowDiagram.position.x + arrowDiagram.keySize + arrowDiagram.keySpacing + arrowDiagram.keySize / 2 - MeasureText("v", 20) / 2, arrowDiagram.position.y + arrowDiagram.keySize / 2 - 10, 20, BLACK);

    // Draw "Right" Arrow Key
    DrawRectangle(arrowDiagram.position.x + 2 * (arrowDiagram.keySize + arrowDiagram.keySpacing), arrowDiagram.position.y, arrowDiagram.keySize, arrowDiagram.keySize, DARKGRAY); // Bevel
    DrawRectangle(arrowDiagram.position.x + 2 * (arrowDiagram.keySize + arrowDiagram.keySpacing) + 4, arrowDiagram.position.y - 4, arrowDiagram.keySize - 8, arrowDiagram.keySize - 8, LIGHTGRAY); // Top Surface
    DrawText(">", arrowDiagram.position.x + 2 * (arrowDiagram.keySize + arrowDiagram.keySpacing) + arrowDiagram.keySize / 2 - MeasureText(">", 20) / 2, arrowDiagram.position.y + arrowDiagram.keySize / 2 - 10, 20, BLACK);
}

void DrawMouseDiagram(MouseDiagram mouseDiagram) {
    // Draw mouse base
    DrawRectangle(mouseDiagram.position.x, mouseDiagram.position.y, mouseDiagram.width, mouseDiagram.height, DARKGRAY); // Bevel
    DrawRectangle(mouseDiagram.position.x + 4, mouseDiagram.position.y - 4, mouseDiagram.width - 8, mouseDiagram.height - 8, LIGHTGRAY); // Top Surface

    // Draw left click button highlighted
    DrawRectangle(mouseDiagram.position.x + 4, mouseDiagram.position.y - 4, mouseDiagram.width / 2 - 4, mouseDiagram.buttonHeight - 8, YELLOW);

    // Draw button separation line
    DrawLine(mouseDiagram.position.x + mouseDiagram.width / 2, mouseDiagram.position.y - 4, mouseDiagram.position.x + mouseDiagram.width / 2, mouseDiagram.position.y + mouseDiagram.height - 8, BLACK);

    // Draw scroll wheel
    DrawRectangle(mouseDiagram.position.x + mouseDiagram.width / 2 - 10, mouseDiagram.position.y + 10, 20, 30, DARKGRAY);
}

void DrawControlsMenu() {
    // Draw a semi-transparent background for the controls menu
    DrawRectangle(200, 100, screenWidth - 400, screenHeight - 200, Fade(BLACK, 0.7f));

    // Header for Controls Menu
    DrawText("CONTROLS", screenWidth / 2 - MeasureText("CONTROLS", 50) / 2, 150, 50, WHITE);

    // Draw columns for Movement, Combat, and Interface controls
    DrawText("Movement", 300, 250, 30, YELLOW);
    DrawText("Combat", 550, 250, 30, YELLOW);
    DrawText("Interface", 800, 250, 30, YELLOW);

    // Set up the diagrams and call the respective drawing functions
    KeyDiagram wasdDiagram = {{300.0f, 350.0f}, 50, 10};
    DrawText("OR", 355, 425, 45, WHITE);
    KeyDiagram arrowDiagram = {{300.0f, 550.0f}, 50, 10};  // Positioned below WASD keys with more spacing
    MouseDiagram mouseDiagram = {{550.0f, 350.0f}, 100, 150, 70};

    // Draw WASD and Arrow Key diagrams
    DrawWASDKeys(wasdDiagram);
    DrawArrowKeys(arrowDiagram);

    // Draw Mouse diagram
    DrawMouseDiagram(mouseDiagram);

    // Add Space Bar diagram for Use Ability
    DrawSpaceBar({515.0f, 550.0f}, 200, 50);
    DrawText("Use Ability", screenWidth / 2 - 35, 515, 20, WHITE);

    // Add Combat control text
    DrawText("Attack", screenWidth / 2 - 35, 300, 20, WHITE);

    // Add Interface controls (ESC, TAB, ENTER)
    Vector2 escKeyPosition = {800.0f, 350.0f};
    Vector2 tabKeyPosition = {800.0f, 450.0f};
    Vector2 enterKeyPosition = {800.0f, 550.0f};

    // Draw Interface keys with corresponding labels
    DrawESCKey(escKeyPosition, 50);
    DrawText("Quit", escKeyPosition.x + 60, escKeyPosition.y + 15, 20, WHITE);

    DrawTABKey(tabKeyPosition, 50);
    DrawText("Pause", tabKeyPosition.x + 110, tabKeyPosition.y + 15, 20, WHITE);

    DrawEnterKey(enterKeyPosition, 50);
    DrawText("Cheats", enterKeyPosition.x + 110, enterKeyPosition.y + 15, 20, WHITE);

    // Add a back option to return to the pause menu
    DrawText("Press SPACE to go back", screenWidth / 2 - MeasureText("Press SPACE to go back", 20) / 2, screenHeight - 150, 20, WHITE);
}

typedef enum EnemyType {
    TRIANGLE,
    SQUARE,
    PENTAGON
} EnemyType;

// Enemy Structure
typedef struct Enemy {
    Vector2 position;
    float size;
    float speed;
    EnemyType type;           // Type of the enemy (TRIANGLE, SQUARE, PENTAGON)
    int health;
    int maxHealth;
    bool active;
    float healthBarVisibleTime; // Timer to control when the health bar fades out
    bool stunned;               // Indicates if the enemy is stunned
    float stunTimer;            // Timer to track how long the enemy is stunned
} Enemy;

Enemy enemies[100];  // Array of enemies

// Projectile Structure
typedef struct Projectile {
    Vector2 position;
    Vector2 velocity;
    float radius;
    bool active;
    int piercesLeft;          // Number of enemies the projectile can pierce through
    bool enemiesHit[100];     // Array to track which enemies have been hit by this projectile
} Projectile;

Projectile projectiles[50];  // Array of projectiles

// Experience Pip Structure
typedef struct ExperiencePip {
    Vector2 position;
    bool active;
} ExperiencePip;

ExperiencePip experiencePips[100];  // Array of experience pips

// Function Prototypes
void DrawMainMenu();
void InitPlayer(Player &player);
void UpdatePlayer(Player &player);
void DrawPlayer(Player &player);
void InitEnemies(Enemy enemies[], int count);
void UpdateEnemies(Enemy enemies[], Player &player);
void DrawEnemies(Enemy enemies[]);
void SpawnEnemy();
void InitProjectiles(Projectile projectiles[], int count);
void UpdateProjectiles(Projectile projectiles[]);
void DrawProjectiles(Projectile projectiles[]);
void FireProjectiles();
void InitExperiencePips(ExperiencePip experiencePips[], int count);
void DrawExperiencePips(ExperiencePip experiencePips[]);
void CheckCollisions();
void ResetGameElements();
void DeveloperMenu(Player &player);
void ApplyEvolution(Player &player);
void DrawExperienceBar(Player &player);
void ClassSelectionMenu();
void DrawPauseMenu();
void DrawHealthBar(Vector2 position, int health, int maxHealth, float size);
void RepulseEnemies(Vector2 playerPosition, float radius);
void InitMusic();
void UpdateMusicPlayback();
void UnloadMusic();

// Function Definitions

// Initialize Music Function
void InitMusic() {
    InitAudioDevice();  // Initialize the audio device

    // Load different music tracks for each game state
    mainMenuMusic = LoadMusicStream("resources/Cobalt Core.mp3"); 
    classSelectMusic = LoadMusicStream("resources/Epoch (Map).mp3");
    gameplayMusic = LoadMusicStream("resources/DT.mp3");
    pauseMusic = LoadMusicStream("resources/Void.mp3");
    victoryMusic = LoadMusicStream("resources/Gravity Well.mp3");
    gameOverMusic = LoadMusicStream("resources/bass drop sound effect.mp3");

    // Set initial volume for each music track
    SetMusicVolume(mainMenuMusic, musicVolume);
    SetMusicVolume(classSelectMusic, musicVolume);
    SetMusicVolume(gameplayMusic, musicVolume);
    SetMusicVolume(pauseMusic, musicVolume);
    SetMusicVolume(victoryMusic, musicVolume);
    SetMusicVolume(gameOverMusic, musicVolume);

    // Set main menu music to not loop
    mainMenuMusic.looping = false;  // Setting looping to false
    gameOverMusic.looping = false;
}

// Update Music Playback
void UpdateMusicPlayback() {
    UpdateMusicStream(mainMenuMusic);
    UpdateMusicStream(classSelectMusic);
    UpdateMusicStream(gameplayMusic);
    UpdateMusicStream(pauseMusic);
    UpdateMusicStream(victoryMusic);
    UpdateMusicStream(gameOverMusic);
}

void StopAllMusic() {
    if (IsMusicStreamPlaying(mainMenuMusic)) {
        StopMusicStream(mainMenuMusic);
    }
    if (IsMusicStreamPlaying(classSelectMusic)) {
        StopMusicStream(classSelectMusic);
    }
    if (IsMusicStreamPlaying(gameplayMusic)) {
        StopMusicStream(gameplayMusic);
    }
    if (IsMusicStreamPlaying(pauseMusic)) {
        StopMusicStream(pauseMusic);
    }
    if (IsMusicStreamPlaying(victoryMusic)) {
        StopMusicStream(victoryMusic);
    }
    if (IsMusicStreamPlaying(gameOverMusic)) {
        StopMusicStream(gameOverMusic);
    }
}


void PlayMusicForState() {
    static MusicType currentMusic = NONE;  // Track which music is currently playing

    if (inMainMenu) {
        if (currentMusic != MAIN_MENU) {
            StopAllMusic();  // Stop all other music streams
            PlayMusicStream(mainMenuMusic);
            currentMusic = MAIN_MENU;
        }
        wasPaused = false;
    } else if (!classSelected) {
        if (currentMusic != CLASS_SELECT) {
            StopAllMusic();
            PlayMusicStream(classSelectMusic);
            currentMusic = CLASS_SELECT;
        }
        wasPaused = false;
    } else if (isVictory) {
        if (currentMusic != VICTORY) {
            StopAllMusic();
            PlayMusicStream(victoryMusic);
            currentMusic = VICTORY;
        }
        wasPaused = false;
    } else if (isGameOver) {
        if (currentMusic != GAME_OVER) {
            StopAllMusic();
            PlayMusicStream(gameOverMusic);
            currentMusic = GAME_OVER;
        }
        wasPaused = false;
    } else if (isPaused) {
        if (currentMusic != PAUSE) {
            PauseMusicStream(gameplayMusic);
            PlayMusicStream(pauseMusic);
            currentMusic = PAUSE;
        }
        wasPaused = true;
    } else {
        if (currentMusic != GAMEPLAY) {
            StopAllMusic();
            if (!wasPaused) {
                SeekMusicStream(gameplayMusic, 0.0f);  // Restart gameplay music only if it wasn't paused
            }
            PlayMusicStream(gameplayMusic);
            currentMusic = GAMEPLAY;
        }
        wasPaused = false;
    }
}
// Unload Music Function
void UnloadMusic() {
    UnloadMusicStream(mainMenuMusic);
    UnloadMusicStream(classSelectMusic);
    UnloadMusicStream(gameplayMusic);
    UnloadMusicStream(pauseMusic);
    UnloadMusicStream(victoryMusic);
    UnloadMusicStream(gameOverMusic);
    CloseAudioDevice();  // Close the audio device when the game is done
}

void ResetGameElements() {
    InitPlayer(player);
    InitEnemies(enemies, 20); 
    InitProjectiles(projectiles, 50);
    InitExperiencePips(experiencePips, 100);
    InitClassDemos();  

    spawnRate = 2.0f;
    timeSinceLastSpawn = 0.0f;
    dashTrail.clear();  
    shockwaveActive = false;  

    for (int i = 0; i < 100; i++) {
        enemies[i].active = false;
    }
}

void DrawMainMenu() {
    DrawText("SURVIVOR", screenWidth / 2 - MeasureText("SURVIVOR", 60) / 2, 100, 60, DARKBLUE);
    DrawText("Press W or S to navigate", 25, 700, 30, BLACK);
    DrawText("Press Space to confirm", 25, 750, 30, BLACK);

    // Menu options
    const char* options[] = {
        "Start Game",
        "How to Play",
        "Exit Game"
    };

    int numOptions = sizeof(options) / sizeof(options[0]);

    // Handle menu navigation
    if (IsKeyPressed(KEY_W)) {
        mainMenuOption--;
        if (mainMenuOption < 0) mainMenuOption = numOptions - 1;
    }

    if (IsKeyPressed(KEY_S)) {
        mainMenuOption++;
        if (mainMenuOption >= numOptions) mainMenuOption = 0;
    }

    // Draw menu options and highlight the selected option
    for (int i = 0; i < numOptions; i++) {
        Color color = (i == mainMenuOption) ? YELLOW : WHITE;
        DrawText(
            options[i],
            screenWidth / 2 - MeasureText(options[i], 30) / 2,
            300 + i * 60,
            30,
            color
        );

        // Draw a white box outline around the selected option
        if (i == mainMenuOption) {
            DrawRectangleLines(
                screenWidth / 2 - MeasureText(options[i], 30) / 2 - 10,
                300 + i * 60 - 5,
                MeasureText(options[i], 30) + 20,
                40,
                WHITE
            );
        }
    }

    // Handle selection with SPACE key
    if (IsKeyPressed(KEY_SPACE)) {
        switch (mainMenuOption) {
            case 0:
                // Start Game
                inMainMenu = false;  // Leave main menu to start the game
                inGameplayMode = true;  // Set gameplay mode to true
                classSelected = false;  // Ensure the class selection comes next
                ResetGameElements();  // Initialize game elements
                break;
            case 1:
                // How to Play
                inMainMenu = false;  // Leave main menu
                inHowToPlayMenu = true;  // Enter How to Play menu
                howToPlayPage = 0;  // Start at the first page of the How to Play guide
                break;
            case 2:
                // Exit Game
                shouldExit = true;  // Set the exit flag
                break;
            default:
                break;
        }
    }
}

void DrawPlayerDemo(Player &demoPlayer) {
    // Draw the main body of the player as a circle
    DrawCircleV(demoPlayer.position, demoPlayer.radius, demoPlayer.playerColor);

    // Draw the cannons for each class (similar to DrawPlayer but without mouse tracking)
    if (demoPlayer.tankClass == SPREAD) {
        // Cannons for Spread class
        for (int i = 0; i < demoPlayer.cannonCount; i++) {
            float angleOffset = (360.0f / demoPlayer.cannonCount) * i;
            Vector2 cannonPos = {
                demoPlayer.position.x + cos(angleOffset * DEG2RAD) * demoPlayer.radius,
                demoPlayer.position.y + sin(angleOffset * DEG2RAD) * demoPlayer.radius
            };
            DrawRectanglePro(
                (Rectangle){cannonPos.x, cannonPos.y, 40, 10},
                (Vector2){0, 5},
                angleOffset,
                DARKBLUE
            );
        }
    } else if (demoPlayer.tankClass == SCOUT) {
        // Cannons for Scout class
        float notchLength = 10.0f;
        float notchHeight = 5.0f;

        for (int j = 0; j < demoPlayer.cannonCount; j++) {
            float angleOffset = (j - (demoPlayer.cannonCount - 1) / 2.0f) * 15.0f;
            Vector2 cannonPos = {
                demoPlayer.position.x + cos(angleOffset * DEG2RAD) * demoPlayer.radius,
                demoPlayer.position.y + sin(angleOffset * DEG2RAD) * demoPlayer.radius
            };
            DrawRectanglePro(
                (Rectangle){cannonPos.x, cannonPos.y, 40, 10},
                (Vector2){0, 5},
                angleOffset,
                DARKGREEN
            );

            // Draw notches for each evolution level
            for (int notch = 0; notch < demoPlayer.level; notch++) {
                float notchOffsetX = cannonPos.x + (40 + notchLength * notch) * cos(angleOffset * DEG2RAD);
                float notchOffsetY = cannonPos.y + (40 + notchLength * notch) * sin(angleOffset * DEG2RAD);
                DrawRectanglePro(
                    (Rectangle){notchOffsetX, notchOffsetY, notchLength, notchHeight},
                    (Vector2){0, notchHeight / 2},
                    angleOffset,
                    DARKGREEN
                );
            }
        }
    } else {
        // Cannons for Assault class
        for (int j = 0; j < demoPlayer.cannonCount; j++) {
            float angleOffset = (j - (demoPlayer.cannonCount - 1) / 2.0f) * 15.0f;
            Vector2 cannonPos = {
                demoPlayer.position.x + cos(angleOffset * DEG2RAD) * demoPlayer.radius,
                demoPlayer.position.y + sin(angleOffset * DEG2RAD) * demoPlayer.radius
            };
            DrawRectanglePro(
                (Rectangle){cannonPos.x, cannonPos.y, 40, 10},
                (Vector2){0, 5},
                angleOffset,
                DARKBLUE
            );
        }
    }
}


void DrawClassDemos() {
    float currentTime = GetTime();

    // Draw Assault Demo
    assaultDemo.position = {400.0f, 600.0f};  // Fixed position for demonstration
    DrawText("Assault", assaultDemo.position.x - MeasureText("Assault", 20) / 2, assaultDemo.position.y - 50, 20, assaultDemo.playerColor);
    DrawPlayerDemo(assaultDemo);  // Use the player drawing function for consistent visuals

    // Assault class shockwave effect
    if (currentTime - assaultDemo.lastVisualTime <= assaultDemo.visualCooldown) {
        float shockwaveRadius = assaultDemo.radius * 2.0f + (50.0f * (currentTime - assaultDemo.lastVisualTime));  // Expand shockwave radius
        float shockwaveOpacity = 1.0f - (currentTime - assaultDemo.lastVisualTime) / assaultDemo.visualCooldown;  // Gradual fade
        if (shockwaveOpacity > 0) {
            Color shockwaveColor = Fade(RED, shockwaveOpacity);
            DrawCircleV(assaultDemo.position, shockwaveRadius, shockwaveColor);
        }
    } else {
        assaultDemo.lastVisualTime = currentTime;  // Reset the timer
    }

    // Draw Scout Demo
    scoutDemo.position = {600.0f, 600.0f};  // Fixed position for demonstration
    DrawText("Scout", scoutDemo.position.x - MeasureText("Scout", 20) / 2, scoutDemo.position.y - 50, 20, scoutDemo.playerColor);

    // Scout class dash effect
    if (currentTime - scoutDemo.lastVisualTime <= scoutDemo.visualCooldown) {
        DashTrail newTrailSegment = {scoutDemo.position, 1.0f};
        dashTrail.push_back(newTrailSegment);
    } else {
        scoutDemo.lastVisualTime = currentTime;  // Reset the timer
    }

    // Draw the dash trail for the Scout demo before drawing the Scout player model
    for (auto &segment : dashTrail) {
        segment.opacity -= GetFrameTime();  // Reduce opacity over time
        if (segment.opacity > 0) {
            Color trailColor = Fade(SKYBLUE, segment.opacity);
            DrawCircleV(segment.position, scoutDemo.radius + 10.0f, trailColor);
        }
    }

    // Remove expired trail segments from deque
    while (!dashTrail.empty() && dashTrail.front().opacity <= 0.0f) {
        dashTrail.pop_front();
    }

    // Draw Scout player demo after the dash trail to ensure the player is not covered by the trail
    DrawPlayerDemo(scoutDemo);

    // Draw Spread Demo (remains unchanged for now)
    spreadDemo.position = {800.0f, 600.0f};  // Fixed position for demonstration
    DrawText("Spread", spreadDemo.position.x - MeasureText("Spread", 20) / 2, spreadDemo.position.y - 50, 20, spreadDemo.playerColor);
    DrawPlayerDemo(spreadDemo);  // Use the player drawing function for consistent visuals

    // Spread class visual effect
    if (currentTime - spreadDemo.lastVisualTime <= spreadDemo.visualCooldown) {
        DrawRectangleLines(spreadDemo.position.x - spreadDemo.radius, spreadDemo.position.y - spreadDemo.radius, spreadDemo.radius * 2, spreadDemo.radius * 2, BLUE);
    } else {
        spreadDemo.lastVisualTime = currentTime;  // Reset the timer
    }
}

void DrawHowToPlayMenu() {
    // Clear the screen and prepare to draw the current page
    DrawRectangle(200, 100, screenWidth - 400, screenHeight - 200, Fade(BLACK, 0.7f));
    DrawText("HOW TO PLAY", screenWidth / 2 - MeasureText("HOW TO PLAY", 50) / 2, 150, 50, WHITE);

    // Declare all the variables at the beginning to avoid jump errors
    KeyDiagram wasdDiagram = {{0.0f, 0.0f}, 0, 0};
    KeyDiagram arrowDiagram = {{0.0f, 0.0f}, 0, 0};
    MouseDiagram mouseDiagram = {{0.0f, 0.0f}, 0, 0, 0};
    Vector2 tabKeyPosition = {0.0f, 0.0f};  // Initialize tabKeyPosition

    switch (howToPlayPage) {
        case 0: {
            DrawText("Overview", (screenWidth/2) - 50, 250, 30, YELLOW);
            DrawText("You must survive for 2 minutes against countless enemies", 300, 300, 20, WHITE);
            DrawText("Over time, the difficulty will ramp up", 300, 350, 20, WHITE);
            DrawText("However, you gain new abilities and power by gathering", 300, 400, 20, WHITE);
            DrawText("red experience orbs from fallen enemies", 300, 425, 20, WHITE);
            break;
        }

        case 1: {
            DrawText("Movement Controls:", 250, 250, 30, YELLOW);
            DrawText("WASD or Arrow Keys to move", 300, 300, 20, WHITE);

            // Initialize WASD and Arrow diagrams for this case
            wasdDiagram = {{300.0f, 400.0f}, 50, 10};
            DrawWASDKeys(wasdDiagram);

            arrowDiagram = {{600.0f, 400.0f}, 50, 10};
            DrawArrowKeys(arrowDiagram);
            break;
        }

        case 2: {
            DrawText("Combat Controls:", 250, 250, 30, YELLOW);
            DrawText("Left Click to shoot, Space to use abilities", 300, 300, 20, WHITE);

            // Initialize Mouse and Space Bar diagrams for combat controls
            mouseDiagram = {{300.0f, 350.0f}, 100, 150, 70};
            DrawMouseDiagram(mouseDiagram);

            DrawSpaceBar({600.0f, 400.0f}, 200, 50);
            DrawText("Use Ability", 650, 470, 20, WHITE);
            break;
        }

        case 3: {
            DrawText("Enemies:", 250, 250, 30, YELLOW);
            DrawText("Triangle - Fast, Square - Balanced, Pentagon - Strong", 300, 300, 20, WHITE);

            // Draw visual representations of each enemy type
            DrawText("Triangle", 300, 350, 20, RED);
            DrawTriangle((Vector2){350, 400}, (Vector2){325, 450}, (Vector2){375, 450}, RED);

            DrawText("Square", 500, 350, 20, YELLOW);
            DrawRectangle(500, 400, 50, 50, YELLOW);

            DrawText("Pentagon", 700, 350, 20, DARKBLUE);
            DrawPoly((Vector2){750, 425}, 5, 30, 0.0f, DARKBLUE);
            break;
        }

        case 4: {
            DrawText("Classes:", 250, 250, 30, YELLOW);
            DrawText("Each class has a different playstyle", screenWidth / 2 - MeasureText("Each class has a different playstyle", 20) / 2, 300, 20, WHITE);
            DrawText("At level 3, each class unlocks their ability", screenWidth / 2 - MeasureText("Each class has a different playstyle", 20) / 2, 325, 20, WHITE);
            DrawText("At level 5, each class is fully evolved", screenWidth / 2 - MeasureText("Each class has a different playstyle", 20) / 2, 350, 20, WHITE);
            DrawText("Test them out and see what happens!", screenWidth / 2 - MeasureText("Each class has a different playstyle", 20) / 2, 375, 20, WHITE);
            DrawText("*Note: The demonstrations below are not yet accurate*", (screenWidth / 2 -  MeasureText("Each class has a different playstyle", 20) / 2) - 65, 425, 20, LIGHTGRAY);

            DrawClassDemos();
            break;
        }

        case 5: {
            DrawText("Tips:", 250, 250, 30, YELLOW);
            DrawText("Keep moving to avoid getting surrounded.\nCollect experience pips to level up.", 300, 300, 20, WHITE);
            DrawText("Press TAB to pause", 300, 400, 20, WHITE);

            // Initialize tabKeyPosition and draw the TAB key diagram below the text
            tabKeyPosition = {300.0f, 450.0f};
            DrawTABKey(tabKeyPosition, 50);
            break;
        }

        default: {
            break;
        }
    }


    // Instructions to navigate between pages
    if (howToPlayPage < totalHowToPlayPages - 1) {
        DrawText("Press SPACE to continue", screenWidth / 2 - MeasureText("Press SPACE to continue", 20) / 2, screenHeight - 100, 20, WHITE);
    } else {
        DrawText("Press SPACE to return to Main Menu", screenWidth / 2 - MeasureText("Press SPACE to return to Main Menu", 20) / 2, screenHeight - 100, 20, WHITE);
    }

    // Page indicator
    DrawText(TextFormat("Page %d / %d", howToPlayPage + 1, totalHowToPlayPages), screenWidth / 2 - MeasureText(TextFormat("Page %d / %d", howToPlayPage + 1, totalHowToPlayPages), 20) / 2, screenHeight - 50, 20, WHITE);
}

void DrawPauseMenu() {
    // Draw a semi-transparent background
    DrawRectangle(200, 100, screenWidth - 400, screenHeight - 200, Fade(BLACK, 0.7f));

    // Pause Menu Header
    DrawText("PAUSED", screenWidth / 2 - MeasureText("PAUSED", 50) / 2, 150, 50, WHITE);

    // Menu options
    const char* options[] = {
        "Resume Game",
        "Controls",
        "Restart",
        "Return to Main Menu"
    };

    int numOptions = sizeof(options) / sizeof(options[0]);

    // Handle menu navigation
    if (IsKeyPressed(KEY_W)) {
        selectedOption--;
        if (selectedOption < 0) selectedOption = numOptions - 1;
    }

    if (IsKeyPressed(KEY_S)) {
        selectedOption++;
        if (selectedOption >= numOptions) selectedOption = 0;
    }

    // Draw menu options and highlight selected option
    for (int i = 0; i < numOptions; i++) {
        Color color = (i == selectedOption) ? YELLOW : WHITE;
        DrawText(
            options[i],
            screenWidth / 2 - MeasureText(options[i], 30) / 2,
            250 + i * 60,
            30,
            color
        );

        // Draw a white box outline around the selected option
        if (i == selectedOption) {
            DrawRectangleLines(
                screenWidth / 2 - MeasureText(options[i], 30) / 2 - 10,
                250 + i * 60 - 5,
                MeasureText(options[i], 30) + 20,
                40,
                WHITE
            );
        }
    }

    // Handle selection with SPACE key
    if (IsKeyPressed(KEY_SPACE)) {
        switch (selectedOption) {
            case 0:
                // Resume Game
                isPaused = false;
                break;
            case 1:
                // Controls (for now just prints a message)
                std::cout << "Controls menu selected (not yet implemented)" << std::endl;
                break;
            case 2:
                // Restart
                isVictory = false;
                isGameOver = false;
                isPaused = false;
                isDeveloperMenu = false;
                survivalTime = 0.0f;
                enemiesDefeated = 0;
                classSelected = false;

                // Reinitialize all elements
                ResetGameElements();
                break;
            case 3:
                // Return to Main Menu
                isVictory = false;
                isGameOver = false;
                isPaused = false;
                isDeveloperMenu = false;
                survivalTime = 0.0f;
                enemiesDefeated = 0;
                classSelected = false;

                inMainMenu = true;  // Switch back to the main menu
                ResetGameElements();  // Reinitialize game elements
                break;
            default:
                break;
        }
    }
}


// Developer Menu Function Definition
void DeveloperMenu(Player &player) {
    if (isDeveloperMenu) {
        // Draw the developer menu UI over everything else
        DrawRectangle(200, 100, screenWidth - 400, screenHeight - 200, Fade(BLACK, 0.5f));
        DrawText("Developer Menu", screenWidth / 2 - MeasureText("Developer Menu", 40) / 2, 120, 40, WHITE);

        // Speed adjustment
        DrawText(TextFormat("Speed: %.1f", player.speed), 300, 200, 20, WHITE);
        if (IsKeyPressed(KEY_UP)) {
            player.speed += 10.0f;
            player.speedModified = true; // Mark the speed as modified
        }
        if (IsKeyPressed(KEY_DOWN)) {
            player.speed -= 10.0f;
            if (player.speed < 0) player.speed = 0;
            player.speedModified = true; // Mark the speed as modified
        }

        // Fire rate adjustment
        DrawText(TextFormat("Fire Rate: %.2f", player.fireRate), 300, 250, 20, WHITE);
        if (IsKeyPressed(KEY_RIGHT)) player.fireRate -= 0.05f; // Decrease value means faster fire rate
        if (IsKeyPressed(KEY_LEFT)) player.fireRate += 0.05f;
        if (player.fireRate < 0.05f) player.fireRate = 0.05f;  // Minimum value

        // Tank Level adjustment
        DrawText(TextFormat("Tank Level: %d", player.level), 300, 300, 20, WHITE);
        if (IsKeyPressed(KEY_W)) {
            player.level++;
            ApplyEvolution(player); // Apply evolution when level increases
        }
        if (IsKeyPressed(KEY_S) && player.level > 1) {
            player.level--;
            ApplyEvolution(player); // Apply evolution when level decreases
        }

        // Player Health adjustment
        DrawText(TextFormat("Health: %d / %d", player.health, player.maxHealth), 300, 350, 20, WHITE);
        if (IsKeyPressed(KEY_A) && player.health < player.maxHealth) player.health += 10;
        if (IsKeyPressed(KEY_D) && player.health > 0) player.health -= 10;

        // Instruction to close developer menu
        DrawText("Press TAB to close Developer Menu", screenWidth / 2 - MeasureText("Press TAB to close Developer Menu", 20) / 2, screenHeight - 150, 20, WHITE);
    }
}

// Apply Evolution Function Definition
void ApplyEvolution(Player &player) {
    if (player.level <= player.EVOLUTION_1_MAX_LEVEL) {
        // Evolution 1 (Base Form)
        player.cannonCount = (player.tankClass == SPREAD) ? 2 : 1; // Spread starts with 2 cannons, others with 1
    } else if (player.level >= player.EVOLUTION_2_MIN_LEVEL && player.level < player.EVOLUTION_3_MIN_LEVEL) {
        // Evolution 2
        if (player.tankClass == SPREAD) {
            player.cannonCount = 4; // Spread gets 4 cannons
        } else if (player.tankClass == ASSAULT) {
            player.cannonCount = 2; // Assault gets 2 cannons
        }
    } else if (player.level >= player.EVOLUTION_3_MIN_LEVEL) {
        // Evolution 3
        if (player.tankClass == SPREAD) {
            player.cannonCount = 8; // Spread gets 8 cannons
            player.speed *= 0.5f; // Reduce speed by half for Spread class at Evolution 3
        } else if (player.tankClass == ASSAULT) {
            player.cannonCount = 3; // Assault gets 3 cannons
        }
    }
}

// Draw Experience Bar Function Definition
void DrawExperienceBar(Player &player) {
    float experiencePercentage = (float)player.experience / player.experienceNeeded;

    // Define experience bar dimensions
    float barWidth = 300.0f;
    float barHeight = 20.0f;
    float barX = (screenWidth / 2) - (barWidth / 2);
    float barY = screenHeight - 50;

    // Draw the experience bar border
    DrawRectangle(barX - 2, barY - 2, barWidth + 4, barHeight + 4, BLACK);

    // Draw the experience bar fill
    DrawRectangle(barX, barY, barWidth * experiencePercentage, barHeight, SKYBLUE);

    // Draw the current level above the experience bar
    DrawText(TextFormat("Level: %d", player.level), barX + barWidth / 2 - MeasureText(TextFormat("Level: %d", player.level), 20) / 2, barY - 30, 20, DARKBLUE);
}

// Class Selection Menu Function Definition
void ClassSelectionMenu() {
    DrawText("SELECT YOUR TANK CLASS", screenWidth / 2 - MeasureText("SELECT YOUR TANK CLASS", 40) / 2, 100, 40, DARKGRAY);
    DrawRectangleLinesEx(classBox, 8.0f, BLACK);
    DrawRectangle(300, 175, 600, 500, Fade(BLACK, 0.3f));
    DrawText("Press 1-3 to select", 500, 600, 20, BLACK);
    

    DrawText("1. ASSAULT", 325, 200, 30, RED);
    DrawText("Spray n' Pray", 325, 240, 20, DARKGRAY);
    if (IsKeyPressed(KEY_ONE)) {
        player.tankClass = ASSAULT;
        player.fireRate = .35f;        // High fire rate
        player.speed = 200.0f;         // Moderate speed
        player.baseSpeed = 200.0f;  
        player.maxHealth = 120;        // Moderate health
        player.health = player.maxHealth;
        player.bulletDamage = 5.0f;    
        player.cannonCount = 1;        // Starts with 1 cannon
        player.repulseCooldown = 13.0f;
        player.lastRepulseTime = -10.0f; 
        classSelected = true;
    }

    DrawText("2. SCOUT", 325, 300, 30, DARKGREEN);
    DrawText("Every shot counts", 325, 340, 20, DARKGRAY);
    if (IsKeyPressed(KEY_TWO)) {
        player.tankClass = SCOUT;
        player.fireRate = 0.8f;        // Slow fire rate
        player.speed = 300.0f;         // Fast speed
        player.baseSpeed = 300.0f;  
        player.maxHealth = 80;         // Low health
        player.health = player.maxHealth;
        player.bulletDamage = 15.0f;   // High bullet damage
        player.cannonCount = 1;        // Starts with 1 cannon
        player.lastDashTime = -10.0f;  // Initialize dash ability cooldown
        classSelected = true;
    }

    DrawText("3. SPREAD", 325, 400, 30, BLUE);
    DrawText("Mobile Fortress", 325, 440, 20, DARKGRAY);
    if (IsKeyPressed(KEY_THREE)) {
        player.tankClass = SPREAD;
        player.fireRate = 0.6f;        // Slower fire rate
        player.speed = 150.0f;         // Slow speed
        player.baseSpeed = 150.0f;
        player.maxHealth = 150;        // Highest health
        player.health = player.maxHealth;
        player.bulletDamage = 8.0f;    // Moderate bullet damage
        player.cannonCount = 2;        // Starts with 2 cannons facing opposite directions
        classSelected = true;
    }
}

// InitExperiencePips Function Definition
void InitExperiencePips(ExperiencePip experiencePips[], int count) {
    for (int i = 0; i < count; i++) {
        experiencePips[i].active = false; // All pips start inactive
    }
}

// DrawExperiencePips Function Definition
void DrawExperiencePips(ExperiencePip experiencePips[]) {
    for (int i = 0; i < 50; i++) {
        if (experiencePips[i].active) {
            DrawCircleV(experiencePips[i].position, 5.0f, MAROON); // Draw pips as dark red circles
        }
    }
}


void ActivateShockwave(Vector2 playerPosition, float radius) {
    shockwaveActive = true;
    shockwaveRadius = radius;
    shockwaveOpacity = 1.0f;
    shockwaveTimer = shockwaveDuration;
    shockwavePosition = playerPosition;
}

void UpdateShockwave() {
    if (shockwaveActive) {
        // Expand shockwave radius gradually over time
        shockwaveRadius += 200.0f * GetFrameTime();  // Increase radius at a controlled rate to match visual effect
        shockwaveOpacity = shockwaveTimer / shockwaveDuration;  // Gradually reduce opacity

        // Draw shockwave with fading effect
        Color currentColor = Fade(ORANGE, shockwaveOpacity);
        DrawCircleV(shockwavePosition, shockwaveRadius, currentColor);

        // Repulse enemies within the expanding shockwave
        for (int i = 0; i < 100; i++) {
            if (enemies[i].active) {
                // Check if the enemy is within the current radius of the shockwave
                if (CheckCollisionCircles(shockwavePosition, shockwaveRadius, enemies[i].position, enemies[i].size)) {
                    Vector2 direction = Vector2Normalize(Vector2Subtract(enemies[i].position, shockwavePosition));
                    
                    // Push the enemy at a rate that matches the expansion of the shockwave
                    enemies[i].position = Vector2Add(enemies[i].position, Vector2Scale(direction, 200.0f * GetFrameTime()));

                    // If the enemy reaches the edge of the shockwave, apply stun
                    float distanceToShockwaveCenter = Vector2Distance(shockwavePosition, enemies[i].position);
                    if (distanceToShockwaveCenter >= shockwaveRadius) {
                        enemies[i].stunned = true;
                        enemies[i].stunTimer = 2.0f;  // Stun the enemy for 2 seconds
                    }
                }
            }
        }

        // Update shockwave timer
        shockwaveTimer -= GetFrameTime();

        // If the timer is finished, end the shockwave effect
        if (shockwaveTimer <= 0.0f) {
            shockwaveActive = false;
        }
    }
}


void DrawBackgroundGrid() {
    // Draw vertical grid lines
    for (int x = 0; x < screenWidth; x += gridSpacing) {
        DrawLine(x, 0, x, screenHeight, Fade(DARKGRAY, 0.3f));
    }

    // Draw horizontal grid lines
    for (int y = 0; y < screenHeight; y += gridSpacing) {
        DrawLine(0, y, screenWidth, y, Fade(DARKGRAY, 0.3f));
    }
}

// Main function
int main() {
    InitWindow(screenWidth, screenHeight, "Survivor");
    InitMusic();  // Load the background music
    InitClassDemos();
    SetTargetFPS(60);
    EnableCursor();
    bool musicPlaying = false;

    while (!WindowShouldClose() && !shouldExit) {
        UpdateMusicPlayback();
        PlayMusicForState();  // Play music according to the current state

        // Set the cursor based on the current game state
        if (inMainMenu || isPaused || !classSelected || isVictory || isGameOver) {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);  // Use an arrow cursor for menu states, pause, victory, or game over
        } else {
            SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);  // Use a crosshair cursor for gameplay
        }

        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        if (inMainMenu) {
            // Draw the main menu
            DrawMainMenu();
        } else if (inHowToPlayMenu) {
            // Draw the How to Play menu
            DrawHowToPlayMenu();

            // Navigate pages using the space bar
            if (IsKeyPressed(KEY_SPACE)) {
                howToPlayPage++;
                if (howToPlayPage >= totalHowToPlayPages) {
                    // If we're past the last page, return to the main menu
                    inHowToPlayMenu = false;
                    inMainMenu = true;
                }
            }
        } else {
            // Toggle pause state with the assigned key (Tab by default)
            if (IsKeyPressed(pauseKey)) {
                isPaused = !isPaused;
                isDeveloperMenu = false;  // Reset developer menu when toggling pause
                inControlsMenu = false;  // Reset controls menu when toggling pause
            }

            // Toggle developer menu with Enter key when paused
            if (isPaused && IsKeyPressed(developerMenuKey)) {
                isDeveloperMenu = !isDeveloperMenu;
            }

            // Restart the game when pressing 'R' on Victory or Game Over screens
            if ((isVictory || isGameOver) && IsKeyPressed(KEY_R)) {
                // Reset game state variables
                isVictory = false;
                isGameOver = false;
                isPaused = false;
                isDeveloperMenu = false;
                inControlsMenu = false;
                survivalTime = 0.0f;
                enemiesDefeated = 0;
                classSelected = false;

                StopMusicStream(backgroundMusic);  // Stop the music to prepare for restart
                PlayMusicStream(backgroundMusic);  // Restart the music for the new game
                musicPlaying = true;

                // Reinitialize all elements
                ResetGameElements();
            }

            if (isPaused) {
                if (isDeveloperMenu) {
                    // Draw the developer menu
                    DeveloperMenu(player);
                } else if (inControlsMenu) {
                    // Draw the controls menu
                    DrawControlsMenu();
                    if (IsKeyPressed(KEY_SPACE)) {
                        inControlsMenu = false;  // Return to pause menu
                    }
                } else {
                    // Draw the pause menu
                    DrawPauseMenu();
                    if (selectedOption == 1 && IsKeyPressed(KEY_SPACE)) {
                        inControlsMenu = true;  // Open Controls Menu when selected
                    }
                }
            } else {
                // Update and draw game only if not paused
                if (!classSelected) {
                    ClassSelectionMenu();
                } else if (!isGameOver && !isVictory) {
                    if (!musicPlaying) {
                        PlayMusicStream(backgroundMusic);  // Start the music after the player selects a tank class
                        musicPlaying = true;
                    }

                    // Increment survival time and check for victory
                    survivalTime += GetFrameTime();
                    if (survivalTime >= survivalGoal) {
                        isVictory = true;  // Set victory state if survival goal is reached
                        StopMusicStream(backgroundMusic);  // Stop the music on victory
                        musicPlaying = false;
                    }

                    // Draw the background grid
                    DrawBackgroundGrid();

                    // Update and draw core game elements
                    UpdatePlayer(player);
                    UpdateEnemies(enemies, player);
                    UpdateProjectiles(projectiles);
                    UpdateShockwave();
                    CheckCollisions();

                    // Draw the game elements
                    DrawPlayer(player);
                    DrawEnemies(enemies);
                    DrawProjectiles(projectiles);
                    DrawExperiencePips(experiencePips);
                    DrawExperienceBar(player);  // Draw experience bar at the bottom of the screen
                    DrawSurvivalTimer(survivalTime, survivalGoal);

                    // Survival timer
                    DrawText(TextFormat("Survival Time: %.1f / %.1f", survivalTime, survivalGoal), 20, 20, 20, BLACK);

                    // Enemy spawning mechanism
                    timeSinceLastSpawn += GetFrameTime();
                    if (timeSinceLastSpawn >= spawnRate) {
                        SpawnEnemy();
                        timeSinceLastSpawn = 0.0f;

                        // Gradually decrease spawn rate over time, but do not go below the minimum spawn rate
                        if (spawnRate > minSpawnRate) {
                            spawnRate -= spawnRateDecrement;
                        }
                    }
                } else if (isVictory) {
                    // Victory Screen
                    DrawText("VICTORY!", screenWidth / 2 - MeasureText("VICTORY!", 50) / 2, screenHeight / 2 - 100, 50, GREEN);
                    DrawText("Press R to Restart or ESC to Quit", screenWidth / 2 - MeasureText("Press R to Restart or ESC to Quit", 20) / 2, screenHeight / 2 + 100, 20, BLACK);
                } else {
                    // Game Over Screen
                    DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 50) / 2, screenHeight / 2 - 50, 50, RED);
                    DrawText("Press R to Restart or ESC to Quit", screenWidth / 2 - MeasureText("Press R to Restart or ESC to Quit", 20) / 2, screenHeight / 2 + 20, 20, BLACK);
                }
            }
        }

        EndDrawing();
    }

    // If exiting game
    UnloadMusic();  // Unload the music from memory
    CloseWindow();
    return 0;
}

// Health Bar Function
void DrawHealthBar(Vector2 position, int health, int maxHealth, float size) {
    // Health bar attributes
    float healthBarWidth = size;
    float healthBarHeight = 5.0f;
    float rounding = 0.2f; // Rounding amount for rounded rectangle

    // Calculate health percentage
    float healthPercentage = (float)health / maxHealth;

    // Draw black border
    DrawRectangleRounded((Rectangle){position.x - healthBarWidth / 2, position.y + size / 2 + 5, healthBarWidth, healthBarHeight}, rounding, 4, BLACK);

    // Draw green health bar
    DrawRectangleRounded((Rectangle){position.x - healthBarWidth / 2, position.y + size / 2 + 5, healthBarWidth * healthPercentage, healthBarHeight}, rounding, 4, GREEN);
}

// Player Functions
void InitPlayer(Player &player) {
    player.position = {400, 300};
    player.radius = 20.0f;
    player.speed = 200.0f;
    player.rotation = 0.0f;
    player.health = 100;
    player.maxHealth = 100;
    player.cannon = {player.position.x, player.position.y, 40, 10};
    player.fireRate = 0.3f;
    player.lastShotTime = 0.0f;
    player.healthBarVisibleTime = 0.0f;
    player.level = 1;
    player.experience = 0;
    player.experienceNeeded = 5; // Level 2 requires 2 XP
    player.isFortified = false;
    player.fortifiedCooldown = 15.0f; // 15-second cooldown
    player.lastFortifiedTime = -15.0f;
    player.fortifiedDuration = 5.0f; // Fortified state lasts for 5 seconds
}

void ApplyStatBuffs(Player &player) {
    switch (player.tankClass) {
        case SCOUT:
            player.bulletDamage += 2.5;
            player.fireRate = max(0.05f, player.fireRate - 0.02f); // Fire rate cannot be lower than 0.05 seconds
            player.lastDashTime = max(2.0f, player.lastDashTime - 1.5f); // Minimum cooldown is 2 seconds
            player.maxHealth += 4; // Increase max health
            player.health = min(player.maxHealth, player.health + 4); // Heal the player and keep health <= maxHealth
            break;
        case ASSAULT:
            player.bulletDamage += 1;
            player.fireRate = max(0.05f, player.fireRate - 0.030f);
            player.repulseCooldown = max(3.0f, player.repulseCooldown - 0.5f); // Minimum cooldown is 3 seconds
            player.maxHealth += 7; // Increase max health
            player.health = min(player.maxHealth, player.health + 7); // Heal the player and keep health <= maxHealth
            break;
        case SPREAD:
            player.bulletDamage += 1.25f;
            player.fireRate = max(0.05f, player.fireRate - 0.025f);
            player.fortifiedDuration += 1.0f;
            player.maxHealth += 10; // Increase max health
            player.health = min(player.maxHealth, player.health + 10); // Heal the player and keep health <= maxHealth
            break;
    }
}

void FireProjectiles(Player &player) {
    for (int j = 0; j < player.cannonCount; j++) {
        for (int i = 0; i < 50; i++) {
            if (!projectiles[i].active) {
                projectiles[i].active = true;

                // Set position and velocity for each cannon
                float angleOffset;
                Vector2 cannonPos;

                if (player.tankClass == SPREAD) {
                    // For the Spread tank, each cannon fires from its unique position, spread evenly around the player
                    angleOffset = player.rotation + (360.0f / player.cannonCount) * j;  // Spread cannons equally around player's current rotation
                    cannonPos = {
                        player.position.x + cos(angleOffset * DEG2RAD) * player.radius,
                        player.position.y + sin(angleOffset * DEG2RAD) * player.radius
                    };
                } else {
                    // For other tanks, use a fan-like pattern in front of the player
                    angleOffset = player.rotation + (j - (player.cannonCount - 1) / 2.0f) * 15.0f;
                    cannonPos = {
                        player.position.x + cos(angleOffset * DEG2RAD) * player.radius,
                        player.position.y + sin(angleOffset * DEG2RAD) * player.radius
                    };
                }

                // Set projectile position to the exact cannon position
                projectiles[i].position = cannonPos;

                // Projectiles should move in the direction of the cannon's rotation
                projectiles[i].velocity = {
                    cos(angleOffset * DEG2RAD) * 400.0f,
                    sin(angleOffset * DEG2RAD) * 400.0f
                };

                // Set pierce count directly for Scout's level 5 evolution
                if (player.tankClass == SCOUT && player.level >= 5) {
                    projectiles[i].piercesLeft = 3; // Scout can pierce 3 enemies at level 5
                } else if (player.tankClass == SCOUT && player.level >= 3) {
                    projectiles[i].piercesLeft = 2; // Scout has pierce ability from level 3
                } else {
                    projectiles[i].piercesLeft = 0; // No pierce ability for other classes
                }

                // Initialize the enemiesHit array to track which enemies have been hit
                for (int k = 0; k < 100; k++) {
                    projectiles[i].enemiesHit[k] = false;
                }

                // Fire trailing smaller shots for Scout Level 5 evolution
                if (player.tankClass == SCOUT && player.level >= 5) {
                    for (int t = 0; t < 2; t++) {
                        for (int l = 0; l < 50; l++) {
                            if (!projectiles[l].active) {
                                projectiles[l].active = true;

                                // Set smaller trailing shot positions slightly behind the main projectile
                                projectiles[l].position = Vector2Subtract(cannonPos, Vector2Scale(projectiles[i].velocity, (t + 1) * 0.1f));

                                // Set smaller shots to have a smaller velocity, creating a trailing effect
                                projectiles[l].velocity = Vector2Scale(projectiles[i].velocity, 0.8f);

                                projectiles[l].radius = 5.0f; // Smaller size for trailing shots
                                projectiles[l].piercesLeft = 0; // No pierce ability for trailing shots

                                // Initialize the enemiesHit array to track which enemies have been hit
                                for (int k = 0; k < 100; k++) {
                                    projectiles[l].enemiesHit[k] = false;
                                }
                                break; // Fire only 2 smaller shots per main projectile
                            }
                        }
                    }
                }

                break; // Fire only one projectile per cannon
            }
        }
    }
}



void RepulseEnemies(Vector2 playerPosition, float radius) {
 
    ActivateShockwave(playerPosition, radius);


    // Repulse enemies within the radius
    for (int i = 0; i < 100; i++) {
        if (enemies[i].active) {
            if (CheckCollisionCircles(playerPosition, radius, enemies[i].position, enemies[i].size)) {
                Vector2 direction = Vector2Normalize(Vector2Subtract(enemies[i].position, playerPosition));
                enemies[i].position = Vector2Add(enemies[i].position, Vector2Scale(direction, 400.0f * GetFrameTime())); // Push enemies away
            }
        }
    }

    // If the shockwave is active, update the effect
    if (shockwaveActive) {
        // Update shockwave parameters
        float timeElapsed = shockwaveDuration - shockwaveTimer;

        shockwaveRadius = radius + 80.0f * timeElapsed;  // Expand shockwave over time
        shockwaveOpacity = shockwaveTimer / shockwaveDuration;  // Gradually reduce opacity

        // Draw shockwave with fading effect
        Color currentColor = Fade(RED, shockwaveOpacity);
        DrawCircleV(playerPosition, shockwaveRadius, currentColor);

        // Update timer
        shockwaveTimer -= GetFrameTime();

        // If the timer is finished, end the shockwave effect
        if (shockwaveTimer <= 0.0f) {
            shockwaveActive = false;
        }
    }
}

// Function to activate the shockwave effect





int FindClosestEnemy(Vector2 position) {
    float closestDistance = FLT_MAX;
    int closestIndex = -1;

    for (int i = 0; i < 100; i++) {
        if (enemies[i].active) {
            float distance = Vector2Distance(position, enemies[i].position);
            if (distance < closestDistance) {
                closestDistance = distance;
                closestIndex = i;
            }
        }
    }
    return closestIndex;
}

void WrapMousePosition() {
    Vector2 mousePos = GetMousePosition();

    // Wrap the mouse cursor around if it leaves the screen bounds
    if (mousePos.x < 0) mousePos.x = screenWidth;
    if (mousePos.x > screenWidth) mousePos.x = 0;
    if (mousePos.y < 0) mousePos.y = screenHeight;
    if (mousePos.y > screenHeight) mousePos.y = 0;

    SetMousePosition(mousePos.x, mousePos.y);
}


void UpdatePlayer(Player &player) {
    // Movement
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) player.position.y -= player.speed * GetFrameTime();
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) player.position.y += player.speed * GetFrameTime();
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) player.position.x -= player.speed * GetFrameTime();
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) player.position.x += player.speed * GetFrameTime();

    if (player.tankClass == SPREAD && player.level >= 3 && IsKeyPressed(KEY_SPACE)) {
        if (GetTime() - player.lastFortifiedTime >= player.fortifiedCooldown) {
            player.isFortified = true;
            player.lastFortifiedTime = GetTime();
        }
    }

    if (player.isFortified) {
        if (GetTime() - player.lastFortifiedTime >= player.fortifiedDuration) {
            player.isFortified = false; // End fortified state
            player.speedModified = false; // Reset the speed modification flag
        } else {
            player.speed = 0.0f; // Player cannot move while fortified
        }
    } else if (!player.speedModified) {
        // Only reset speed if the player hasn't modified it manually
        player.speed = player.baseSpeed;
    }

    // Collect experience pips
    for (int i = 0; i < 50; i++) {
        if (experiencePips[i].active && CheckCollisionCircles(player.position, player.radius, experiencePips[i].position, 5.0f)) {
            experiencePips[i].active = false;
            player.experience += 1;

            // Check if player should level up
            if (player.experience >= player.experienceNeeded) {
                player.level++;
                player.experience = 0; // Reset experience after leveling up
                player.experienceNeeded += 5; // Increase XP needed for next level

                // Show player health bar to indicate level-up (optional feedback)
                player.healthBarVisibleTime = 2.0f;
                
                ApplyStatBuffs(player);

                ApplyEvolution(player);
            }
        }
    }

    // Cannon movement
    Vector2 mousePosition = GetMousePosition();
    player.rotation = atan2f(mousePosition.y - player.position.y, mousePosition.x - player.position.x) * (180 / PI); // Convert to degrees

    // Keep the cursor within window bounds
    WrapMousePosition();

    // Repulse Ability for Assault (activated by pressing space bar)
    if (player.tankClass == ASSAULT && player.level >= 3 && IsKeyPressed(KEY_SPACE)) {
        DrawCircleV(player.position, player.radius, ORANGE);
        if (GetTime() - player.lastRepulseTime >= player.repulseCooldown) {
            RepulseEnemies(player.position, 150.0f); // Repulse enemies within 100 units
            player.lastRepulseTime = GetTime();
        }
    }

    // Dash Ability for Scout (activated by pressing space bar)
    if (player.tankClass == SCOUT && player.level >= 3 && IsKeyPressed(KEY_SPACE)) {
        if (GetTime() - player.lastDashTime >= 10.0f) { // 10-second cooldown
            player.isDashing = true; // Start dashing
            player.lastDashTime = GetTime();
        }
    }

    // Handle dash effect for Scout
    static float dashDuration = 1.0f; // Duration of the dash effect
    if (player.isDashing) {
        player.speed = 500.0f; // Increase speed temporarily

        // Store current position for trail
        DashTrail newTrailSegment = {player.position, 1.0f};
        dashTrail.push_back(newTrailSegment);

        if (GetTime() - player.lastDashTime >= dashDuration) {
            player.isDashing = false; // End dash
            player.speed = player.baseSpeed; // Reset speed to base
        }
    }

    // Update the dash trail
    for (auto &segment : dashTrail) {
        segment.opacity -= GetFrameTime(); // Reduce opacity over time
    }

    // Remove old trail segments
    while (!dashTrail.empty() && dashTrail.front().opacity <= 0.0f) {
        dashTrail.pop_front();
    }

    // Shooting logic 
    player.lastShotTime += GetFrameTime();
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && player.lastShotTime >= player.fireRate) {
        FireProjectiles(player);
        player.lastShotTime = 0.0f;
    }

    // Check if player's health is 0 or less, and set game over flag
    if (player.health <= 0) {
        player.health = 0;
        isGameOver = true;  // Set the game over state when player's health reaches zero
    }
}


void DrawPlayer(Player &player) {
    // Draw the dash trail
    for (const auto &segment : dashTrail) {
        Color trailColor = Fade(SKYBLUE, segment.opacity);
        DrawCircleV(segment.position, player.radius + 10.0f, trailColor);
    }


    // Draw the cannons for each class
    if (player.tankClass == SPREAD) {
        DrawCircleV(player.position, player.radius, BLUE);
        // Cannons for Spread class
        for (int i = 0; i < player.cannonCount; i++) {
            float angleOffset = (360.0f / player.cannonCount) * i + player.rotation;
            Vector2 cannonPos = {
                player.position.x + cos(angleOffset * DEG2RAD) * player.radius,
                player.position.y + sin(angleOffset * DEG2RAD) * player.radius
            };
            DrawRectanglePro(
                (Rectangle){cannonPos.x, cannonPos.y, 40, 10},
                (Vector2){0, 5},
                angleOffset,
                DARKBLUE
            );
        }
    } else if (player.tankClass == SCOUT) {
        DrawCircleV(player.position, player.radius, GREEN);
        // Cannons for Scout class
        float notchLength = 10.0f;
        float notchHeight = 5.0f;

        for (int j = 0; j < player.cannonCount; j++) {
            float angleOffset = player.rotation + (j - (player.cannonCount - 1) / 2.0f) * 15.0f;
            Vector2 cannonPos = {
                player.position.x + cos(angleOffset * DEG2RAD) * player.radius,
                player.position.y + sin(angleOffset * DEG2RAD) * player.radius
            };
            DrawRectanglePro(
                (Rectangle){cannonPos.x, cannonPos.y, 40, 10},
                (Vector2){0, 5},
                angleOffset,
                DARKGREEN
            );

            // Draw notches for each evolution level
            for (int notch = 0; notch < player.level; notch++) {
                float notchOffsetX = cannonPos.x + (40 + notchLength * notch) * cos(angleOffset * DEG2RAD);
                float notchOffsetY = cannonPos.y + (40 + notchLength * notch) * sin(angleOffset * DEG2RAD);
                DrawRectanglePro(
                    (Rectangle){notchOffsetX, notchOffsetY, notchLength, notchHeight},
                    (Vector2){0, notchHeight / 2},
                    angleOffset,
                    DARKGREEN
                );
            }
        }
    } else {  DrawCircleV(player.position, player.radius, RED);
        // Cannons for Assault class
        for (int j = 0; j < player.cannonCount; j++) {
            float angleOffset = player.rotation + (j - (player.cannonCount - 1) / 2.0f) * 15.0f;
            Vector2 cannonPos = {
                player.position.x + cos(angleOffset * DEG2RAD) * player.radius,
                player.position.y + sin(angleOffset * DEG2RAD) * player.radius
            };
            DrawRectanglePro(
                (Rectangle){cannonPos.x, cannonPos.y, 40, 10},
                (Vector2){0, 5},
                angleOffset,
                MAROON
            );
        }
    }

    // Draw the player's health bar if recently damaged
    if (player.healthBarVisibleTime > 0) {
        DrawHealthBar(player.position, player.health, player.maxHealth, 40.0f);
    }
}

void InitEnemies(Enemy enemies[], int count) {
    for (int i = 0; i < count; i++) {
        enemies[i].position = {(float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight)};
        
        // Randomly assign enemy type
        int randomType = GetRandomValue(0, 2);  // 0 for TRIANGLE, 1 for SQUARE, 2 for PENTAGON
        enemies[i].type = (EnemyType)randomType;

        // Set size, speed, and health based on the enemy type
        switch (enemies[i].type) {
            case TRIANGLE:
                enemies[i].size = 10.0f;
                enemies[i].speed = 200.0f;
                enemies[i].health = 15;
                enemies[i].maxHealth = 10;
                break;

            case SQUARE:
                enemies[i].size = 35.0f;
                enemies[i].speed = 85.0f;
                enemies[i].health = 20;
                enemies[i].maxHealth = 20;
                break;

            case PENTAGON:
                enemies[i].size = 45.0f;
                enemies[i].speed = 50.0f;
                enemies[i].health = 60;
                enemies[i].maxHealth = 60;
                break;
        }

        enemies[i].active = false;
        enemies[i].healthBarVisibleTime = 0.0f;  // Start with health bar invisible
        enemies[i].stunned = false;
        enemies[i].stunTimer = 0.0f;
    }
}

void UpdateEnemies(Enemy enemies[], Player &player) {
    for (int i = 0; i < 100; i++) {
        if (enemies[i].active) {
            // Check if the enemy is stunned
            if (enemies[i].stunned) {
                enemies[i].stunTimer -= GetFrameTime();
                if (enemies[i].stunTimer <= 0.0f) {
                    enemies[i].stunned = false;  // End stun state when timer runs out
                }
                continue;  // Skip movement if stunned
            }

            // Move towards the player if not stunned
            Vector2 direction = Vector2Normalize(Vector2Subtract(player.position, enemies[i].position));
            enemies[i].position.x += direction.x * enemies[i].speed * GetFrameTime();
            enemies[i].position.y += direction.y * enemies[i].speed * GetFrameTime();
        }
    }
}


void SpawnEnemy() {
    for (int i = 0; i < 100; i++) {
        if (!enemies[i].active) {
            enemies[i].active = true;

            // Randomly select which edge to spawn the enemy from
            int edge = GetRandomValue(0, 3);
            if (edge == 0) {  // Top edge
                enemies[i].position = {(float)GetRandomValue(0, screenWidth), 0.0f};
            } else if (edge == 1) {  // Bottom edge
                enemies[i].position = {(float)GetRandomValue(0, screenWidth), (float)screenHeight};
            } else if (edge == 2) {  // Left edge
                enemies[i].position = {0.0f, (float)GetRandomValue(0, screenHeight)};
            } else if (edge == 3) {  // Right edge
                enemies[i].position = {(float)screenWidth, (float)GetRandomValue(0, screenHeight)};
            }

            // Randomly assign enemy type
            int randomType = GetRandomValue(0, 2);  // 0 for TRIANGLE, 1 for SQUARE, 2 for PENTAGON
            enemies[i].type = (EnemyType)randomType;

            // Set size, speed, and health based on the enemy type
            switch (enemies[i].type) {
                case TRIANGLE:
                    enemies[i].size = 10.0f;
                    enemies[i].speed = 200.0f;
                    enemies[i].health = 10;
                    enemies[i].maxHealth = 10;
                    break;

                case SQUARE:
                    enemies[i].size = 30.0f;
                    enemies[i].speed = 80.0f;
                    enemies[i].health = 20;
                    enemies[i].maxHealth = 20;
                    break;

                case PENTAGON:
                    enemies[i].size = 50.0f;
                    enemies[i].speed = 40.0f;
                    enemies[i].health = 40;
                    enemies[i].maxHealth = 40;
                    break;
            }

            enemies[i].healthBarVisibleTime = 0.0f; // Start with health bar invisible
            break;
        }
    }
}

void DrawEnemies(Enemy enemies[]) {
    for (int i = 0; i < 100; i++) {
        if (enemies[i].active) {
            Color enemyColor;

            switch (enemies[i].type) {
                case TRIANGLE:
                    enemyColor = RED;
                    // Draw Triangle
                    DrawTriangle((Vector2){enemies[i].position.x, enemies[i].position.y - enemies[i].size},
                                 (Vector2){enemies[i].position.x - enemies[i].size, enemies[i].position.y + enemies[i].size},
                                 (Vector2){enemies[i].position.x + enemies[i].size, enemies[i].position.y + enemies[i].size}, enemyColor);
                    break;

                case SQUARE:
                    enemyColor = YELLOW;
                    // Draw Square
                    DrawRectangleV((Vector2){enemies[i].position.x - enemies[i].size / 2, enemies[i].position.y - enemies[i].size / 2},
                                   (Vector2){enemies[i].size, enemies[i].size}, enemyColor);
                    break;

                case PENTAGON:
                    enemyColor = DARKBLUE;
                    // Draw Pentagon
                    DrawPoly(enemies[i].position, 5, enemies[i].size, 1.0f, enemyColor);
                    break;
            }

            // Draw the enemy's health bar if recently damaged
            if (enemies[i].healthBarVisibleTime > 0) {
                DrawHealthBar(enemies[i].position, enemies[i].health, enemies[i].maxHealth, enemies[i].size);
            }
        }
    }
}

// Projectile functions
void InitProjectiles(Projectile projectiles[], int count) {
    for (int i = 0; i < count; i++) {
        projectiles[i].active = false;
        projectiles[i].radius = 10.0f; // Projectiles are small circles
        projectiles[i].piercesLeft = 0;

        // Initialize all enemies as not hit by this projectile
        for (int j = 0; j < 100; j++) {
            projectiles[i].enemiesHit[j] = false;
        }
    }
}

void UpdateProjectiles(Projectile projectiles[]) {
    for (int i = 0; i < 50; i++) {
        if (projectiles[i].active) {
            projectiles[i].position.x += projectiles[i].velocity.x * GetFrameTime();
            projectiles[i].position.y += projectiles[i].velocity.y * GetFrameTime();

            // Deactivate the projectile if it goes off screen
            if (projectiles[i].position.x < 0 || projectiles[i].position.x > screenWidth ||
                projectiles[i].position.y < 0 || projectiles[i].position.y > screenHeight) {
                projectiles[i].active = false;
            }

            // Ricochet logic if in fortified mode
            if (player.isFortified && !projectiles[i].active) {
                int closestEnemy = FindClosestEnemy(projectiles[i].position);
                if (closestEnemy != -1) {
                    Vector2 direction = Vector2Normalize(Vector2Subtract(enemies[closestEnemy].position, projectiles[i].position));
                    projectiles[i].velocity = Vector2Scale(direction, 400.0f);
                    projectiles[i].active = true;  // Reactivate the projectile to ricochet
                }
            }
        }
    }
}

void DrawProjectiles(Projectile projectiles[]) {
    for (int i = 0; i < 50; i++) {
        if (projectiles[i].active) {
            DrawCircleV(projectiles[i].position, projectiles[i].radius, SKYBLUE); // Projectiles are black circles
        }
    }
}

// Collision checking
void CheckCollisions() {
    // Enemy-Enemy Collision
    for (int i = 0; i < 100; i++) {
        if (!enemies[i].active) continue;

        for (int j = i + 1; j < 100; j++) {
            if (!enemies[j].active) continue;

            if (CheckCollisionCircles(enemies[i].position, enemies[i].size, enemies[j].position, enemies[j].size)) {
                // Calculate direction vector between the two enemies
                Vector2 direction = Vector2Normalize(Vector2Subtract(enemies[j].position, enemies[i].position));

                // Move each enemy slightly apart to prevent overlap
                enemies[i].position = Vector2Subtract(enemies[i].position, Vector2Scale(direction, 2.0f));
                enemies[j].position = Vector2Add(enemies[j].position, Vector2Scale(direction, 2.0f));
            }
        }
    }

    // Enemy-Player Collision
    for (int i = 0; i < 100; i++) {
        if (enemies[i].active && CheckCollisionCircleRec(enemies[i].position, enemies[i].size, (Rectangle){player.position.x - player.radius, player.position.y - player.radius, player.radius * 2, player.radius * 2})) {
            // Handle player taking damage
            player.health -= 10; // Reduce player health by 10

            // Show player health bar when damaged
            player.healthBarVisibleTime = 2.0f;

            // Ensure player health doesn't go below 0
            if (player.health < 0) {
                player.health = 0;
            }

            // Set game over flag if health reaches 0
            if (player.health <= 0) {
                isGameOver = true;
            }

            // Handle enemy taking damage from player contact
            enemies[i].health -= playerContactDamage;

            // Show enemy health bar when hit
            enemies[i].healthBarVisibleTime = 2.0f;

            // Deactivate the enemy if health reaches 0
            if (enemies[i].health <= 0) {
                enemies[i].active = false;
                enemiesDefeated++;
            }
        }
    }

    // Projectile-Enemy Collision
    for (int i = 0; i < 100; i++) {
        if (!enemies[i].active) continue;

        for (int j = 0; j < 50; j++) {
            if (!projectiles[j].active) continue;

            // Check if the projectile has already hit this enemy
            if (projectiles[j].enemiesHit[i]) continue;

            // Check collision between projectile and enemy
            if (CheckCollisionCircles(projectiles[j].position, projectiles[j].radius, enemies[i].position, enemies[i].size)) {
                // Mark the enemy as hit by this projectile
                projectiles[j].enemiesHit[i] = true;

                // Damage the enemy
                enemies[i].health -= player.bulletDamage; // Use player's bullet damage
                enemies[i].healthBarVisibleTime = 2.0f;

                // Deactivate the enemy if health reaches 0 or below
                if (enemies[i].health <= 0) {
                    enemies[i].active = false;
                    enemiesDefeated++;

                    // Drop an experience pip
                    for (int k = 0; k < 50; k++) {
                        if (!experiencePips[k].active) {
                            experiencePips[k].active = true;
                            experiencePips[k].position = enemies[i].position;
                            break;
                        }
                    }
                }

                // Reduce pierce count and deactivate if necessary
                if (projectiles[j].piercesLeft > 0) {
                    projectiles[j].piercesLeft--;
                }

                // Deactivate projectile if it has no pierces left
                if (projectiles[j].piercesLeft <= 0) {
                    projectiles[j].active = false;
                }
            }
        }
    }
}