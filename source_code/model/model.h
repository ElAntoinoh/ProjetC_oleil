#ifndef MODEL_H
#define MODEL_H

/* ---------- */
/* Structures */
/* ---------- */

typedef struct {
    double posX, posY;
} Position;

typedef struct {
    Position position;

    int radius;
    int orbit;
} Planet;

typedef struct {
    Position position;
    int radius;
} Sun;

typedef struct {
    Sun sun;

    int nbPlanets;
    Planet * planets;
} SolarSystem;

typedef struct {
    double strength;
    double angle;
} Vector;

typedef struct {
    Position position;

    Vector trajectoryAngle;
    Vector instructionsAngle;
    Vector gravityAngle;
    int weight;

    int minSpeed;
    int maxSpeed;
} Spaceship;

typedef struct {
    int winWidth, winHeight;

    Position startingPoint;
    Position arrivalPoint;
    Spaceship spaceship;

    int nbStars;
    int nbSolarSystems;
    SolarSystem * solarSystems;

    int score;
} Configuration;

/* --------- */
/* Functions */
/* --------- */

void loadConfigFile();

void startTime();
void startGame();

void rotatePlanets();

void moveSpaceship();

void startTurningLeft();
void startTurningRight();
void stopTurningLeft();
void stopTurningRight();

bool isGameWin();
bool isGameLost();

void freeAllocations();

Configuration getConfiguration();

#endif