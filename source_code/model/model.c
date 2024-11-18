#include <time.h>
#include <stdbool.h>

#include <SDL2/SDL2_gfxPrimitives.h>

#include "model.h"

#define OUTER_BORDER_EDGE 10
#define SPACESHIP_WEIGHT 2
#define SPACESHIP_INITIAL_SPEED 4
#define G 1000

Configuration configuration;
clock_t startingTime;

bool leftKeyPressed  = false;
bool rightKeyPressed = false;

typedef struct {
    char * keyword;
    int nbParameters;
    bool parametersCanBeNegative;

    char ** previousKeyWords;
    int previousKeyWordsLength;

    char ** followingKeyWords;
    int followingKeyWordsLength;
} Information;

char * globalPreviousWords[9][2] = {{""}, {""}, {"START"}, {""}, {""}, {"STAR_POS"}, {"STAR_RADIUS"}, {"NB_PLANET", "PLANET_ORBIT"}, {"PLANET_RADIUS"}};
char * globalFollowingWords[9][2] = {{""},{"END"},{""},{""},{"STAR_RADIUS"},{"NB_PLANET"},{"PLANET_RADIUS", ""},{"PLANET_ORBIT"},{"PLANET_RADIUS", ""}};

Information informationDictionary[] = {
    {"WIN_SIZE"       , 2, false, globalPreviousWords[0], 1, globalFollowingWords[0], 1},
    {"START"          , 2, false, globalPreviousWords[1], 1, globalFollowingWords[1], 1},
    {"END"            , 2, false, globalPreviousWords[2], 1, globalFollowingWords[2], 1},
    {"NB_SOLAR_SYSTEM", 1, false, globalPreviousWords[3], 1, globalFollowingWords[3], 1},
    {"STAR_POS"       , 2, false, globalPreviousWords[4], 1, globalFollowingWords[4], 1},
    {"STAR_RADIUS"    , 1, false, globalPreviousWords[5], 1, globalFollowingWords[5], 1},
    {"NB_PLANET"      , 1, false, globalPreviousWords[6], 1, globalFollowingWords[6], 2},
    {"PLANET_RADIUS"  , 1, false, globalPreviousWords[7], 2, globalFollowingWords[7], 1},
    {"PLANET_ORBIT"   , 1, true , globalPreviousWords[8], 1, globalFollowingWords[8], 2}
};

/**
 * Search for an Information using its key word
 * 
 * @param keyword Keyword of the attribute we are looking for
 * 
 * @return The Information found or an Information with -1 parameters if no Information was found
*/
Information findInformation(char * keyword) {
    // Iterate through all the attributes
    for (int i = 0; i < sizeof(informationDictionary) / sizeof(Information); i++) {
        // Compares the keyword of the searched Information with the keyword of the observed Information
        if (!strcmp(informationDictionary[i].keyword, keyword)) {
            return informationDictionary[i];
        }
    }

    // Creation of an Information with -1 parameters
    Information nullInformation;
    nullInformation.nbParameters = -1;

    return nullInformation;
}

/**
 * Return the open file
 * 
 * @param path File path
 * 
 * @return Open file
*/
FILE * getOpenFile(char * path) {
    return fopen(path, "r");
}

/**
 * Check the number of successive occurrences of a character in a file
 * 
 * @param path File path
 * @param character Character to check
 * @param nbMax Max number of successive occurences of the character
 * 
 * @return True if everything is fine or false otherwise
*/
bool checkNbCharacterOccurencies(char * path, char character, int nbMax) {
    FILE * file = getOpenFile(path);

    char currentChar;
    int nbCharacterOccurences = 0;

    // Iterate through all the characters of the file
    while ((currentChar = fgetc(file)) != EOF) {
        if (currentChar == character) {
            nbCharacterOccurences++;

            // Checking the current number of occurences
            if (nbCharacterOccurences > nbMax - 1) {
                fclose(file);
                return false;
            }
        } else {
            nbCharacterOccurences = 0;
        }
    }

    fclose(file);
    return true;
}

/**
 * Returns the first word of a string
 * 
 * @param input The string
 * 
 * @return The first word of the string
*/
char * getFirstWord(char * input) {
    int len = strlen(input);
    char * output = (char *) malloc(len + 1);

    int i;

    // Copy the input string into the output string character by character until you come across a space or a line break
    for(i = 0; i < len && input[i] != ' ' && input[i] != '\n'; i++) {
        output[i] = input[i];
    }

    // Ending the output string
    output[i] = '\0';

    return output;
}

/**
 * Check that a word is in a word list
 * 
 * @param word Word tested
 * @param list Word list
 * @param listLength Length of the word list
 * 
 * @return True if everything is fine or false otherwise
*/
bool checkWordIsInList(char * word, char ** list, int listLength) {
    for (int i = 0; i < listLength; i++) {
        if (strcmp(word, list[i]) == 0) {
            return true;
        }
    }

    return false;
}

/**
 * Checks that the lines contents are OK
 * 
 * @param path File path
 * 
 * @return True if everything is fine or false otherwise
*/
bool areLinesOK(char * path) {
    FILE * file = getOpenFile(path);

    char line[256];

    // Iterate through all the lines of the file
    while (!feof(file)) {
        if (fgets(line, sizeof(line), file) != NULL) {
            // Ignoring the empty lines
            if (line[0] == '\n') continue;

            // Find the first space character in the line
            int i = 0;
            while (line[i] != '\n' && line[i] != '\0') {
                if (line[i] == ' ') break;

                i++;
            }

            i++;

            int nbSpace = 1;

            // Cutting out of the first word
            char * firstWord = getFirstWord(line);

            // Checking if the first word is known
            if (findInformation(firstWord).nbParameters == -1) {
                printf("Some first words of lines are unknown\n");
                fclose(file);
                return false;
            }

            // Stores in a variable whether a row's parameters can be negative or not
            bool parametersCanBeNegative = findInformation(getFirstWord(line)).parametersCanBeNegative;

            // Iterate through all the characters of the line folowing the first word
            while (line[i] != '\n' && line[i] != '\0') {
                // Incrementing of the nbSpace value;
                if (line[i] == ' ') nbSpace++;

                // Checking if the parameters are allowed to be negative or not
                if (line[i] == '-' && !parametersCanBeNegative) {
                    printf("Some lines have negative parameters when they should'nt\n");
                    fclose(file);
                    return false;
                }

                // Checking if the parameters are integers
                if (!(isdigit(line[i]) || line[i] == '-' || line[i] == ' ')) {
                    printf("Some lines have non-integers parameters\n");
                    fclose(file);
                    return false;
                }

                i++;
            }

            // Checking if we found the correct number of parameters
            if (nbSpace != findInformation(getFirstWord(line)).nbParameters) {
                printf("Some lines do not have the correct number of parameters\n");
                fclose(file);
                return false;
            }
        }
    }

    fclose(file);
    return true;
}

/**
 * Check that all the lines of a file are in a consistent order
 * 
 * @param path File path
 * 
 * @return True if everything is fine or false otherwise
*/
bool isLinesOrderOK(char * path) {
    FILE * file = getOpenFile(path);

    char previousFirstWord [256] = "";
    char currentFirstWord  [256] = "";
    char followingFirstWord[256] = "";

    char newLine[256] = "";

    // Iterate through all the lines of the file
    while (fgets(newLine, sizeof(newLine), file) != NULL) {
        strcpy(newLine, getFirstWord(newLine));

        strcpy(previousFirstWord, currentFirstWord);
        strcpy(currentFirstWord, followingFirstWord);
        strcpy(followingFirstWord, newLine);

        Information attribute = findInformation(getFirstWord(currentFirstWord));

        if (attribute.nbParameters == -1) continue;

        // Checks that the previous and following words are valids
        if (!checkWordIsInList(previousFirstWord, attribute.previousKeyWords, attribute.previousKeyWordsLength) || !checkWordIsInList(followingFirstWord, attribute.followingKeyWords, attribute.followingKeyWordsLength)) {
            printf("Problem near a line starting with \"%s\"\n", currentFirstWord);
            fclose(file);
            return false;
        }
    }

    fclose(file);
    return true;
}

/**
 * Check if the configuration file is OK
 * 
 * @param path The file path
 * 
 * @return True if OK or false otherwise
*/
bool isConfigurationFileOK(char * path) {
    // Checking that there are no double line breaks
    if (!checkNbCharacterOccurencies(path, '\n', 3)) {
        printf("Too many lines skipped consecutively\n");
        return false;
    }

    // Checking that there are no double spaces
    if (!checkNbCharacterOccurencies(path, ' ', 2)) {
        printf("Successive spaces are not allowed\n");
        return false;
    }

    // Check that lines content is OK
    if (!areLinesOK(path)) {
        printf("Some line's not first words are'nt valid\n");
        return false;
    }

    // Check that all the lines are in a consistent order
    if (!isLinesOrderOK(path)) {
        printf("Some lines are misplaced\n");
        return false;
    }

    return true;
}

/**
 * Checks that a position is in the frame
 * 
 * @param position Checked Position
 * 
 * @return True if everything is OK, false otherwise
*/
bool checkPositionInFrame(Position position) {
    return !(
        position.posX < 0 ||
        position.posX > configuration.winWidth ||
        position.posY < 0 ||
        position.posY > configuration.winHeight
    );
}

/**
 * Checks that a planet is in the frame
 * 
 * @param sun Checked sun
 * 
 * @return True if everything is OK, false otherwise
*/
bool checkPlanetPosition(Sun sun, Planet planet) {
    return !(sun.position.posY - abs(planet.orbit) < 0);
}

/**
 * Read the configuration file to create the configuration of the game
 * 
 * @param path Path of the configuration file
*/
void loadConfigFile(char * path) {
    // Configuration file opening
    FILE * configFile = getOpenFile(path);

    // Ensure configuration file's openability
    if (configFile == NULL) {
        printf("Can't open configuration file (%s).\n", path);
        exit(1);
    }

    // Ensure configuration file's structure
    if (!isConfigurationFileOK(path)) {
        printf("Configuration file error (%s).\n", path);
        exit(1);
    }

    // Window dimensions
    fscanf(configFile, "WIN_SIZE %d %d\n", &configuration.winWidth, &configuration.winHeight);

    // Starting point
    fscanf(configFile, "START %le %le\n", &configuration.startingPoint.posX, &configuration.startingPoint.posY);

    if (!checkPositionInFrame((Position) {configuration.startingPoint.posX, configuration.startingPoint.posY})) {
        printf("The starting point is outside of the frame\n");
        fclose(configFile);
        exit(1);
    }

    // Arrival point
    fscanf(configFile, "END %le %le\n", &configuration.arrivalPoint.posX, &configuration.arrivalPoint.posY);

    if (!checkPositionInFrame((Position) {configuration.arrivalPoint.posX, configuration.arrivalPoint.posY})) {
        printf("The arrival point is outside of the frame\n");
        fclose(configFile);
        exit(1);
    }

    Spaceship spaceship = {0};
    configuration.spaceship = spaceship;

    // Initial spaceship position (on the starting point)
    configuration.spaceship.position = configuration.startingPoint;

    // Spaceship's weight
    configuration.spaceship.weight = SPACESHIP_WEIGHT;

    // Number of solar systemes reading
    fscanf(configFile, "NB_SOLAR_SYSTEM %d\n", &configuration.nbSolarSystems);

    // Solar systems list
    configuration.solarSystems = malloc(configuration.nbSolarSystems * sizeof(SolarSystem));

    // Iterate through all the solar systems
    for (int i = 0; i < configuration.nbSolarSystems; i++) {
        // Including the sun in the spacial objects counter
        configuration.nbStars++;

        // Sun's attributes
        if (fscanf(configFile, "STAR_POS %le %le\n", &configuration.solarSystems[i].sun.position.posX, &configuration.solarSystems[i].sun.position.posY) != 2) {
            printf("Waited for %d solar systems but only got %d\n", configuration.nbSolarSystems, i);
            fclose(configFile);
            exit(1);
        }
        fscanf(configFile, "STAR_RADIUS %d\n", &configuration.solarSystems[i].sun.radius);

        if (!checkPositionInFrame(configuration.solarSystems[i].sun.position)) {
            printf("A sun is outside of the frame\n");
            fclose(configFile);
            exit(1);
        }

        // Number of planets reading
        fscanf(configFile, "NB_PLANET %d\n", &configuration.solarSystems[i].nbPlanets);

        // Planets list
        configuration.solarSystems[i].planets = malloc(configuration.solarSystems[i].nbPlanets * sizeof(Planet));

        // Iterate through all the planets
        for (int j = 0; j < configuration.solarSystems[i].nbPlanets; j++) {
            // Including the planet in the spacial objects counter
            configuration.nbStars++;

            // Planet's attributes
            if (fscanf(configFile, "PLANET_RADIUS %d PLANET_ORBIT %d\n", &configuration.solarSystems[i].planets[j].radius, &configuration.solarSystems[i].planets[j].orbit) != 2) {
                printf("Waited for %d planets in the n°%d solar system but only got %d\n", configuration.solarSystems[i].nbPlanets, i + 1, j);
                fclose(configFile);
                exit(1);
            }
        
            if (!checkPlanetPosition(configuration.solarSystems[i].sun, configuration.solarSystems[i].planets[j])) {
                printf("A planet is gonna leave the frame\n");
                fclose(configFile);
                exit(1);
            }
        }
    }

    // Configuration file closing
    fclose(configFile);
}

/**
 * Start the timer used in the game
*/
void startTime() {
    startingTime = SDL_GetTicks();
}

/**
 * Initialize the spaceship's attributes and launch it
*/
void launchSpaceship() {
    srand(time(NULL));

    double randRadianMin = 0;
    double randRadianMax = 2 * M_PI;

    double radian = randRadianMin + (double) rand() / RAND_MAX * randRadianMax;

    configuration.spaceship.trajectoryAngle.angle   = radian;
    configuration.spaceship.instructionsAngle.angle = radian;
    configuration.spaceship.gravityAngle.angle      = radian;
}

/**
 * Start the game
*/
void startGame() {
    configuration.spaceship.position = configuration.startingPoint;

    configuration.spaceship.trajectoryAngle  .strength = SPACESHIP_INITIAL_SPEED;
    configuration.spaceship.instructionsAngle.strength = 0;
    configuration.spaceship.gravityAngle     .strength = 0;

    configuration.spaceship.minSpeed = configuration.spaceship.trajectoryAngle.strength / 2;
    configuration.spaceship.maxSpeed = configuration.spaceship.trajectoryAngle.strength * 2;

    launchSpaceship();
}

/**
 * Rotate a planet
 * 
 * @param sun Sun around which the planet orbits
 * @param planet Planet to rotate
 * 
 * @return New position of the planet
*/
Position rotatePlanet(Sun sun, Planet planet) {
    // Angle of the planet relative to the sun
    double radian = (SDL_GetTicks() - startingTime) % (planet.radius * 1000) * 2 * M_PI / (planet.radius * 1000) - 0.5 * M_PI;

    // Adaptation of the angle relative to its direction of rotation
    if (planet.orbit < 0) {
        radian *= -1;
    }

    return (Position) {sun.position.posX + planet.orbit * cos(radian), sun.position.posY + planet.orbit * sin(radian)};
}

/**
 * Rotates all the planets
*/
void rotatePlanets() {
    // Iterate through all the solar systems
    for (int i = 0; i < configuration.nbSolarSystems; i++) {
        SolarSystem solarSystem = configuration.solarSystems[i];

        // Iterate through all the planets
        for (int j = 0; j < solarSystem.nbPlanets; j++) {
            // Calculation of the new planet's position
            solarSystem.planets[j].position = rotatePlanet(solarSystem.sun, solarSystem.planets[j]);
        }
    }
}

/**
 * Changes the variable managing the left rotation of the spaceship to a true state
*/
void startTurningLeft() {
    leftKeyPressed = true;
}

/**
 * Changes the variable managing the right rotation of the spaceship to a true state
*/
void startTurningRight() {
    rightKeyPressed = true;
}

/**
 * Changes the variable managing the left rotation of the spaceship to a false state
*/
void stopTurningLeft() {
    leftKeyPressed = false;
}

/**
 * Changes the variable managing the right rotation of the spaceship to a false state
*/
void stopTurningRight() {
    rightKeyPressed = false;
}

/**
 * Apply the toricity to given coordinates
 * 
 * @param position The coordinates to which we want to apply the toricity
 * 
 * @return The new coordinates with toricity applied
*/
Position applyToricity(Position position) {
    int width = configuration.winWidth;
    int height = configuration.winHeight;

    // Managing of the horizontal toricity
    position.posX = fmod(position.posX + width, width);

    // Managing of the vertical toricity
    position.posY = fmod(position.posY + height, height);

    return position;
}

/**
 * Calculates a toroidal distance between to positions
 * 
 * @param p1 First position
 * @param p2 Second position
 * 
 * @return Toroidal distance
*/
double toroidalDistance(Position p1, Position p2) {
    int width = configuration.winWidth;
    int height = configuration.winHeight;

    // Standardization of lengths according to toricity dimensions
    double dX = fmod(fabs(p1.posX - p2.posX), width);
    double dY = fmod(fabs(p1.posY - p2.posY), height);

    // Managing of the horizontal toricity
    if (dX > width / 2) {
        dX = width - dX;
    }

    // Managing of the vertical toricity
    if (dY > height / 2) {
        dY = height - dY;
    }

    return sqrt(pow(dX, 2) + pow(dY, 2));
}

/**
 * Calculates a toroidal radian angle between to positions
 * 
 * @param p1 First position
 * @param p2 Second position
 * 
 * @return Radian angle
*/
double toroidalRadian(Position p1, Position p2) {
    int height = configuration.winWidth;
    int width = configuration.winHeight;

    // Standardization of lengths according to toricity dimensions
    double dx = fmod(p2.posX - p1.posX + width  / 2.0, width ) - width  / 2.0;
    double dy = fmod(p2.posY - p1.posY + height / 2.0, height) - height / 2.0;

    // Angle calculation
    double radian = atan2(dy, dx);

    // Angle standardization between 0 and 2*PI
    if (radian < 0) {
        radian += 2.0 * M_PI;
    }

    return radian;
}

/**
 * Calculate the vectors's angle weighted average
 * 
 * @param vectors Vectors used in the average
 * @param distances Distances that weight the average
 * @param nbVectors Number of vectors
 * 
 * @return Angles weighted average
*/
double calculateWeightedAngleAverage(Vector * vectors, double * distances, int nbVectors) {
    double totalX = 0.0;
    double totalY = 0.0;

    // Iterate through all the vectors
    for (int i = 0; i < nbVectors; i++) {
        double strength = vectors[i].strength * distances[i];
        totalX += strength * cos(vectors[i].angle);
        totalY += strength * sin(vectors[i].angle);
    }

    double angle = atan2(totalY, totalX);

    // Normalization of the average angle between 0 and 2*PI
    if (angle < 0.0) {
        angle += 2.0 * M_PI;
    }

    return angle;
}

/**
 * Calculate the vectors's forces weighted average
 * 
 * @param vectors Vectors used in the average
 * @param distances Distances that weight the average
 * @param nbVectors Number of vectors
 * 
 * @return Forces weighted average
*/
double calculateWeightedForceAverage(Vector * vectors, double * distances, int nbVectors) {
    double weightedSum = 0.0;
    double totalDistance = 0.0;

    // Iterate through all the vectors
    for (int i = 0; i < nbVectors; i++) {
        weightedSum += vectors[i].strength * distances[i] * vectors[i].strength;
        totalDistance += distances[i];
    }

    // Weighted average calculation
    return weightedSum / totalDistance;
}

/**
 * Move the spaceship based on his directionnal vectors
*/
void moveSpaceship() {
    Spaceship spaceship = configuration.spaceship;

    Vector vectors[configuration.nbStars];

    if (configuration.nbStars == 0) {
        spaceship.gravityAngle.strength = 0;
        spaceship.gravityAngle.angle = 0;
    } else {
        double starsDistances[configuration.nbStars];

        int index = 0;

        // Iterate through all the solar systems
        for (int i = 0; i < configuration.nbSolarSystems; i++) {
            SolarSystem solarSystem = configuration.solarSystems[i];

            Sun sun = solarSystem.sun;

            starsDistances[index] = toroidalDistance(spaceship.position, sun.position);

            vectors[index].strength = (G * sun.radius * spaceship.weight) / pow(starsDistances[index], 2);

            vectors[index].angle = toroidalRadian(spaceship.position, sun.position);

            index++;

            // Iterate through all the planets
            for (int j = 0; j < solarSystem.nbPlanets; j++) {
                Planet planet = solarSystem.planets[j];

                starsDistances[index] = toroidalDistance(spaceship.position, planet.position);

                vectors[index].strength = (G * planet.radius * spaceship.weight) / pow(starsDistances[index], 2);

                vectors[index].angle = toroidalRadian(spaceship.position, planet.position);

                index++;
            }
        }

        // Calculation of the weighted average of the angles's forces and gravities
        spaceship.gravityAngle.strength = calculateWeightedForceAverage(vectors, starsDistances, configuration.nbStars);
        spaceship.gravityAngle.angle    = calculateWeightedAngleAverage(vectors, starsDistances, configuration.nbStars);
    }

    if (spaceship.gravityAngle.strength > spaceship.maxSpeed) spaceship.gravityAngle.strength = spaceship.maxSpeed;

    double angle = atan2(
        spaceship.trajectoryAngle.strength * sin(spaceship.trajectoryAngle.angle) + spaceship.gravityAngle.strength * sin(spaceship.gravityAngle.angle),
        spaceship.trajectoryAngle.strength * cos(spaceship.trajectoryAngle.angle) + spaceship.gravityAngle.strength * cos(spaceship.gravityAngle.angle)
    );

    // Normalization of the angle (between 0 and 2*PI)
    if (angle < 0.0) {
        angle += 2.0 * M_PI;
    }

    spaceship.trajectoryAngle.angle = angle;

    // Considering of the users inputs regarding direction
    if (leftKeyPressed ) spaceship.trajectoryAngle.angle -= M_PI / 60;
    if (rightKeyPressed) spaceship.trajectoryAngle.angle += M_PI / 60;

    configuration.score += leftKeyPressed || rightKeyPressed;

    double speed = spaceship.trajectoryAngle.strength;

    // Checking speed compliance
    if (speed < spaceship.minSpeed) speed = spaceship.minSpeed;
    if (speed > spaceship.maxSpeed) speed = spaceship.maxSpeed;

    // New value of the trajectory angle's force
    spaceship.trajectoryAngle.strength = speed;

    // Calculation of the new position of the spaceship
    spaceship.position.posX = spaceship.position.posX + spaceship.trajectoryAngle.strength * cos(spaceship.trajectoryAngle.angle);
    spaceship.position.posY = spaceship.position.posY + spaceship.trajectoryAngle.strength * sin(spaceship.trajectoryAngle.angle);

    configuration.spaceship = spaceship;

    // Toricity applying
    configuration.spaceship.position = applyToricity(configuration.spaceship.position);
}

/**
 * Determine if the game is win
 * 
 * @return True if the game is lost, false else
*/
bool isGameWin() {
    Position spaceshipPosition = configuration.spaceship.position;
    Position arrivalPointPosition = configuration.arrivalPoint;

    return abs(spaceshipPosition.posX - arrivalPointPosition.posX) < 5 && abs(spaceshipPosition.posY - arrivalPointPosition.posY) < 5;
}

/**
 * Determine if the game is lost
 * 
 * @return True if the game is lost, false else
*/
bool isGameLost() {
    Position spaceshipPosition = configuration.spaceship.position;

    // Iterate through all the solar systems
    for (int i = 0; i < configuration.nbSolarSystems; i++) {
        SolarSystem solarSystem = configuration.solarSystems[i];

        Sun sun = solarSystem.sun;

        // Managing possible sun collisions
        if (sqrt(pow(sun.position.posX - spaceshipPosition.posX, 2) + pow(sun.position.posY - spaceshipPosition.posY, 2)) < abs(sun.radius)) {
            return true;
        }

        // Iterate through all the planets
        for (int j = 0; j < solarSystem.nbPlanets; j++) {
            Planet planet = solarSystem.planets[j];

            // Managing possible planet collisions
            if (sqrt(pow(planet.position.posX - spaceshipPosition.posX, 2) + pow(planet.position.posY - spaceshipPosition.posY, 2)) < abs(planet.radius)) {
                return true;
            }
        }
    }

    return false;
}

/**
 * Frees dynamically allocated memories
*/
void freeAllocations() {
    // Iterate through all the solar systems
    for (int i = 0; i < configuration.nbSolarSystems; i++) {
        free(configuration.solarSystems[i].planets);
    }

    free(configuration.solarSystems);
}

/**
 * Game's configuration accessor
*/
Configuration getConfiguration() {
    return configuration;
}
