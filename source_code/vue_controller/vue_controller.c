#include <stdbool.h>

#include <SDL2/SDL2_gfxPrimitives.h>

#include "vue_controller.h"
#include "../model/model.h"

#define OUTER_BORDER_EDGE   10
#define STARTSHIP_SIZE      10
#define STARTING_POINT_SIZE 10
#define ARRIVAL_POINT_SIZE  10

#define OUTER_BORDER_COLOR   0xFFFFFFFF
#define STARTING_POINT_COLOR 0xFFFFFFFF
#define ARRIVAL_POINT_COLOR  0xFFFFFFFF
#define SUNS_COLOR           0xFF00FFFF
#define PLANETS_COLOR        0xFFFFFF00
#define SPACESHIP_COLOR      0xFF0000FF
#define ORBITS_COLOR         0xFFFFFFFF
#define TRAJECTORY_COLOR     0xFFFFFFFF
#define GRAVITY_COLOR        0xFF8080FF

SDL_Window * window;
SDL_Renderer * renderer;

bool showTrajectories = false;

/**
 * Calculate the R, G, B and A attributes of a hexadecimal color
 * 
 * @param hexColor Color in hexadecimal format
 * 
 * @return Array of int representing the R, G, B and A attributes of the hexadecimal color
*/
int * convertHexToRGBA(Uint32 hexColor) {
    int * tab = malloc(4 * sizeof(int));
    
    tab[0] = ((hexColor >> 24) & 0xFF);
    tab[1] = ((hexColor >> 16) & 0xFF);
    tab[2] = ((hexColor >>  8) & 0xFF);
    tab[3] = ((hexColor >>  0) & 0xFF);

    return tab; 
}

/**
 * Draw a empty rectangle
 * 
 * @param x Upper left rectangle's corner abscissa
 * @param y Upper left rectangle's corner ordinate
 * @param width Rectangle's width
 * @param height Rectangle's height
 * @param hexColor Rectangle's color in hexadecimal format
*/
void drawEmptyRectangle(int x, int y, int width, int height, Uint32 hexColor) {
    int * rgbaColor = convertHexToRGBA(hexColor);

    SDL_SetRenderDrawColor(renderer, rgbaColor[0], rgbaColor[1], rgbaColor[2], rgbaColor[3]);

    free(rgbaColor);

    SDL_Rect rect = {x, y, width, height};

    SDL_RenderDrawRect(renderer, &rect);
}

/**
 * Draw a filled square based on its center
 * 
 * @param centerPosition Coordinates of the center of the square
 * @param hexColor Square's color in hexadecimal format
*/
void drawCenteredFilledSquare(Position centerPosition, int size, Uint32 hexColor) {
    int * rgbaColor = convertHexToRGBA(hexColor);

    SDL_SetRenderDrawColor(renderer, rgbaColor[0], rgbaColor[1], rgbaColor[2], rgbaColor[3]);

    free(rgbaColor);

    SDL_Rect rect = {centerPosition.posX - size / 2, centerPosition.posY - size / 2, size, size};

    SDL_RenderFillRect(renderer, &rect);
}

/**
 * Draw a empty square based on its center
 * 
 * @param centerPosition Coordinates of the center of the square
 * @param hexColor Square's color in hexadecimal format
*/
void drawCenteredEmptySquare(Position centerPosition, int size, Uint32 hexColor) {
    int * rgbaColor = convertHexToRGBA(hexColor);

    SDL_SetRenderDrawColor(renderer, rgbaColor[0], rgbaColor[1], rgbaColor[2], rgbaColor[3]);

    free(rgbaColor);

    SDL_Rect rect = {centerPosition.posX - size / 2, centerPosition.posY - size / 2, size, size};

    SDL_RenderDrawRect(renderer, &rect);
}

/**
 * Draw a empty circle
 * 
 * @param centerPosition Coordinates of the center of the circle
 * @param hexColor Circle's color in hexadecimal format
*/
void drawEmptyCircle(Position centerPosition, int radius, Uint32 hexColor) {
    int * rgbaColor = convertHexToRGBA(hexColor);

    circleRGBA(renderer, centerPosition.posX, centerPosition.posY, radius, rgbaColor[0], rgbaColor[1], rgbaColor[2], rgbaColor[3]);

    free(rgbaColor);
}

/**
 * Draw a filled circle
 * 
 * @param centerPosition Coordinates of the center of the circle
 * @param hexColor Circle's color in hexadecimal format
*/
void drawFilledCircle(Position centerPosition, int radius, Uint32 hexColor) {
    filledCircleColor(renderer, centerPosition.posX, centerPosition.posY, radius, hexColor);
}

/**
 * Draw a vector
 * 
 * @param startingPosition Coordinates of the starting point of the vector
 * @param vector Vector's angle and length
 * @param hexColor Vector's color in hexadecimal format
*/
void drawVector(Position startPosition, Vector vector, Uint32 hexColor) {
    int * rgbaColor = convertHexToRGBA(hexColor);

    SDL_SetRenderDrawColor(renderer, rgbaColor[0], rgbaColor[1], rgbaColor[2], rgbaColor[3]);

    free(rgbaColor);

    SDL_RenderDrawLine(renderer, startPosition.posX, startPosition.posY, startPosition.posX + 10 * vector.strength * cos(vector.angle), startPosition.posY + 10 * vector.strength * sin(vector.angle));
}

/**
 * Initialize all the prerequisites for visual management
*/
void initializeRenderer() {
    Configuration configuration = getConfiguration();

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_Log("SDL cant start %s", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow("SDL example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, configuration.winWidth, configuration.winHeight, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        SDL_Log("Window cant be generated %s", SDL_GetError());
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        SDL_Log("Renderer cannot be generated %s", SDL_GetError());
        exit(1);
    }
}

/**
 * Reset the window and display the updated state of the universe
*/
void printActualState() {
    Configuration configuration = getConfiguration();

    // Resetting the display
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Window borders display
    drawEmptyRectangle(
        OUTER_BORDER_EDGE,
        OUTER_BORDER_EDGE,
        configuration.winWidth  - 2 * OUTER_BORDER_EDGE,
        configuration.winHeight - 2 * OUTER_BORDER_EDGE,
        OUTER_BORDER_COLOR
    );

    // Starting point display
    drawCenteredEmptySquare(configuration.startingPoint, STARTING_POINT_SIZE, ARRIVAL_POINT_COLOR);

    // Arrival point display
    drawCenteredEmptySquare(configuration.arrivalPoint, ARRIVAL_POINT_SIZE, ARRIVAL_POINT_COLOR);

    // Spaceship display
    drawCenteredFilledSquare(configuration.spaceship.position, STARTSHIP_SIZE, SPACESHIP_COLOR);

    // Vectors influencing the trajectory of the spaceship display (if activated)
    if (showTrajectories) {
        drawVector(configuration.spaceship.position, configuration.spaceship.trajectoryAngle, TRAJECTORY_COLOR);
        drawVector(configuration.spaceship.position, configuration.spaceship.gravityAngle,    GRAVITY_COLOR);
    }

    // Iterate through all the solar systems
    for (int i = 0; i < configuration.nbSolarSystems; i++) {
        SolarSystem solarSystem = configuration.solarSystems[i];

        Sun sun = solarSystem.sun;

        // Sun display
        drawFilledCircle(sun.position, sun.radius, SUNS_COLOR);

        // Iterate through all the planets
        for (int j = 0; j < solarSystem.nbPlanets; j++) {
            Planet planet = solarSystem.planets[j];

            // Planet orbit display
            drawEmptyCircle(sun.position, abs(planet.orbit), ORBITS_COLOR);

            // Planet display
            drawFilledCircle(planet.position, planet.radius, PLANETS_COLOR);
        }
    }

    SDL_RenderPresent(renderer);
}

/**
 * Closing SDL objects and freeing dynamically allocated memories
*/
void freeSDL() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

/**
 * Changes the state of the variable managing the display of vectors
*/
void permuteTrajectoriesShowing() {
    showTrajectories = !showTrajectories;
}

/**
 * Updates the title of the window
 * 
 * @param currentTime Actual timestamp
 * @param lastTime Timestamp of the last update
*/
void updateTitle(float currentTime, float lastTime) {
    char title[50];
    sprintf(title, "ProjetC_oleil | FPS: %.1f | Score: %d", 1000 / (currentTime - lastTime), getConfiguration().score);
    SDL_SetWindowTitle(window, title);
}