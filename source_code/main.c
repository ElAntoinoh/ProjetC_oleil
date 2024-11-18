#include <stdbool.h>

#include <SDL2/SDL2_gfxPrimitives.h>

#include "model/model.h"
#include "vue_controller/vue_controller.h"

#define FPS 1000/60

/**
 * Main function of the main program
*/
int main(int argc, char * argv[]) {
    // Configuration file reading
    loadConfigFile(argv[1]);

    // Window initialization
    initializeRenderer();

    // Initial universe display
    printActualState();

    // Time starting
    startTime();

    int lastUpdateTime = SDL_GetTicks();

    bool stopGame = false;
    bool gameStarted = false;

    // Game loop
    while (!stopGame) {
        SDL_Event event;

        // Keyboard events listening
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT : {
                    gameStarted = true;
                    stopGame = true;
                    break;
                }

                case SDL_KEYDOWN : {
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE : {
                            gameStarted = true;
                            stopGame = true;
                            break;
                        }

                        case SDLK_SPACE : {
                            if (!gameStarted) {
                                gameStarted = true;
                                startGame();
                            }
                            
                            break;
                        }

                        case SDLK_v : {
                            permuteTrajectoriesShowing();
                            break;
                        }

                        case SDLK_LEFT : {
                            startTurningLeft();
                            break;
                        }

                        case SDLK_RIGHT : {
                            startTurningRight();
                            break;
                        }
                    }
                    break;
                }

                case SDL_KEYUP : {
                    switch (event.key.keysym.sym) {
                        case SDLK_LEFT : {
                            stopTurningLeft();
                            break;
                        }

                        case SDLK_RIGHT : {
                            stopTurningRight();
                            break;
                        }
                    }
                    break;
                }
            }
        }

        int currentTime = SDL_GetTicks();

        // If we are in the updating delay
        if (currentTime - lastUpdateTime >= FPS) {
            // Actual state of the universe display
            printActualState();

            // Planets rotations
            rotatePlanets();

            // Spaceship moving
            moveSpaceship();

            //mise à jour du titre et des FPS
            updateTitle(currentTime, lastUpdateTime);

            // Winning and losing conditions checking
            if (isGameWin()) {
                printf("Well played ! Score : %d\n", getConfiguration().score);
                stopGame = true;
            } else if (isGameLost()) {
                stopGame = true;
            }

            lastUpdateTime = currentTime;
        }
    }

    // Dynamically allocated memories freeing
    freeAllocations();

    // SDL objects freeing
    freeSDL();

    return 0;
}