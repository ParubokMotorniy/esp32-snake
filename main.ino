#include "initialization.h"
#include "builders.h"

namespace {

//constants

const char *leftLabel = "Left";
const char *rightLabel = "Right";
const char *pauseLabel = "Pause";

constexpr uint16_t sceneXStart = 10;
constexpr uint16_t frameXEnd = 310;

constexpr uint16_t sceneYStart = 40;
constexpr uint16_t frameYEnd = 180;

constexpr uint16_t cellSize = 10;  //pixels

constexpr lv_point_t sceneCorners[] = { { sceneXStart, sceneYStart }, { sceneXStart, frameYEnd }, { frameXEnd, frameYEnd }, { frameXEnd, sceneYStart }, { sceneXStart, sceneYStart } };

constexpr uint16_t xSceneSize = (((frameXEnd - sceneXStart) / cellSize) - 1);
constexpr uint16_t ySceneSize = (((frameYEnd - sceneYStart) / cellSize) - 1);

constexpr uint16_t numXGridPoints = xSceneSize * 2;
constexpr uint16_t numYGridPoints = ySceneSize * 2;

constexpr uint16_t fieldSize = xSceneSize * ySceneSize;

//end constants

lv_point_t *gridEndpoints = []() {
  static lv_point_t gridEndpoints[numXGridPoints + numYGridPoints];

  for (uint16_t i = 0; i < numXGridPoints; i += 2) {
    const auto xPos = (sceneXStart + cellSize) + cellSize * (i / 2);
    gridEndpoints[i] = { xPos, sceneYStart };
    gridEndpoints[i + 1] = { xPos, frameYEnd };
  }

  for (uint16_t i = 0; i < numYGridPoints; i += 2) {
    const auto yPos = (sceneYStart + cellSize) + cellSize * (i / 2);
    gridEndpoints[i + numXGridPoints] = { sceneXStart, yPos };
    gridEndpoints[i + numXGridPoints + 1] = { frameXEnd, yPos };
  }

  return gridEndpoints;
}();


enum SnakeDirection : uint8_t {
  UP,
  RIGHT,
  DOWN,
  LEFT,
  NONE
};

SnakeDirection currentSnakeDirection = NONE;
SnakeDirection directionField[ySceneSize][xSceneSize];
volatile SnakeDirection nextTurn = NONE;

lv_point_t snakeBodyPoints[fieldSize];
uint16_t snakeSize = 0;

lv_point_t applePosition;

lv_obj_t *snake = nullptr;
lv_obj_t *apple = nullptr;
bool gameIsPaused = false;
}

lv_point_t gridCoordinatesToGlobal(const lv_point_t& gridCoords)
{
  auto [y,x] = gridCoords;
  return { sceneXStart + cellSize + x * cellSize, sceneYStart + cellSize + y * cellSize };
}

void dropApple()
{
  do
  {
    applePosition.x = random(0, ySceneSize);
    applePosition.y = random(0, xSceneSize);
  }while(directionField[applePosition.x][applePosition.y] != NONE);

  Serial.println("Apple position: ");
  Serial.println(applePosition.x);
  Serial.println(applePosition.y);

  const auto appleGlobalCoordinates = gridCoordinatesToGlobal(applePosition); 
  lv_obj_align(apple, LV_ALIGN_CENTER, appleGlobalCoordinates.x - (screenWidth / 2), appleGlobalCoordinates.y - (screenHeight / 2));
}

void redrawSnake() {
  static lv_point_t sceneCoordinates[fieldSize];

  for (size_t j = 0; j < snakeSize; ++j) {
    sceneCoordinates[j] = gridCoordinatesToGlobal(snakeBodyPoints[j]);
  }

  lv_line_set_points(snake, sceneCoordinates, snakeSize);
}

void updateSnake() {
  auto [headY, headX] = snakeBodyPoints[snakeSize - 1];
  auto [tailY, tailX] = snakeBodyPoints[0];

  if (headX == xSceneSize || headY == ySceneSize || headX == -1 || headY == -1) {
    gameIsPaused = true;
    return;
  }

  SnakeDirection directionToSet = NONE;
  if (nextTurn != NONE) {
    Serial.println("Turning");
    auto appliedTurn = static_cast<int>(currentSnakeDirection) + (nextTurn == RIGHT ? 1 : -1);
    directionToSet = static_cast<SnakeDirection>( appliedTurn > 3 ? 0 : appliedTurn < 0 ? 3 : appliedTurn  );
    currentSnakeDirection = directionToSet;
    nextTurn = NONE;
  } else {
    directionToSet = currentSnakeDirection;
  }
  directionField[headY][headX] = directionToSet;

  if(headY == applePosition.x && headX == applePosition.y)
  {
    Serial.println("Ate apple!");

    int16_t growX = headX + (directionToSet == RIGHT ? 1 : (directionToSet == LEFT ? -1 : 0));
    uint16_t growY = headY + (directionToSet == DOWN ? 1 : (directionToSet == UP ? -1 : 0));

    snakeBodyPoints[snakeSize] = {growY, growX};
    directionField[growY][growX] = directionToSet;
    snakeSize += 1;
    dropApple();
  }

  for (size_t j = 0; j < snakeSize; ++j) {
    auto [sectionY, sectionX] = snakeBodyPoints[j];

    switch (directionField[sectionY][sectionX]) {
      case UP:
        sectionY -= 1;
        break;
      case RIGHT:
        sectionX += 1;
        break;
      case DOWN:
        sectionY += 1;
        break;
      case LEFT:
        sectionX -= 1;
        break;
    }

    snakeBodyPoints[j] = { sectionY, sectionX };
  }

  directionField[tailY][tailX] = NONE;
}

static void leftEventHandler(lv_event_t *e) {
  if (nextTurn == NONE && !gameIsPaused) {
    nextTurn = LEFT;
  }
}
static void rightEventHandler(lv_event_t *e) {
  if (nextTurn == NONE && !gameIsPaused) {
    nextTurn = RIGHT;
  }
}
static void pauseEventHandler(lv_event_t *e) {
  Serial.println("Pause pressed!");
}

void setup() {
  Serial.begin(115200);

  initHardware();

  memset(directionField, NONE, sizeof(directionField));

  auto leftButton = createGameButtons(-100, 90, leftLabel, leftEventHandler);
  auto rightButton = createGameButtons(100, 90, rightLabel, rightEventHandler);
  auto pauseButton = createGameButtons(0, 90, pauseLabel, pauseEventHandler);
  auto gameLogo = createGameLogo(0, -90);

  lv_obj_t *gameFrame = nullptr;
  lv_obj_t *gameGrid = nullptr;
  createGameScene(sceneCorners, gridEndpoints, numXGridPoints + numYGridPoints, &gameFrame, &gameGrid);

  snakeBodyPoints[0] = { 0, 0 };
  snakeBodyPoints[1] = { 0, 1 };
  snakeBodyPoints[2] = { 0, 2 };

  directionField[0][0] = RIGHT;
  directionField[0][1] = RIGHT;
  directionField[0][2] = RIGHT;

  snake = createSnake();
  currentSnakeDirection = RIGHT;
  snakeSize = 3;

  redrawSnake();

  apple = createApple(15);
  dropApple();

  Serial.println("Initialization successful!");
}

void loop() {
  updateUi();
  if (!gameIsPaused) {
    updateSnake();
    redrawSnake();
  }
  delay(50);
}