#include "drivers.h"
#include "builders.h"
#include "music.h"
#include <Arduino.h>

namespace {

//constants
constexpr uint16_t sceneXStart = 10;
constexpr uint16_t frameXEnd = 310;

constexpr uint16_t sceneYStart = 40;
constexpr uint16_t frameYEnd = 180;

constexpr uint16_t cellSize = 10;  //pixels

//global coordinates for UI of the scene frame
constexpr lv_point_t sceneCorners[] = { { sceneXStart, sceneYStart }, { sceneXStart, frameYEnd }, { frameXEnd, frameYEnd }, { frameXEnd, sceneYStart }, { sceneXStart, sceneYStart } };

constexpr uint16_t xSceneSize = (((frameXEnd - sceneXStart) / cellSize) - 1);
constexpr uint16_t ySceneSize = (((frameYEnd - sceneYStart) / cellSize) - 1);

constexpr uint16_t numXGridPoints = xSceneSize * 2;
constexpr uint16_t numYGridPoints = ySceneSize * 2;

constexpr uint16_t fieldSize = xSceneSize * ySceneSize;

constexpr uint16_t snakeSpeed = 5;  //cells per second
constexpr uint16_t snakeUpdateInterval = 1000 / snakeSpeed;

//end constants


//this thing computes the endpoints for the grid
const lv_point_t *gridEndpoints = []() {
  static lv_point_t endpoints[numXGridPoints + numYGridPoints];

  for (uint16_t i = 0; i < numXGridPoints; i += 2) {
    const auto xPos = (sceneXStart + cellSize) + cellSize * (i / 2);
    endpoints[i] = { xPos, sceneYStart };
    endpoints[i + 1] = { xPos, frameYEnd };
  }

  for (uint16_t i = 0; i < numYGridPoints; i += 2) {
    const auto yPos = (sceneYStart + cellSize) + cellSize * (i / 2);
    endpoints[i + numXGridPoints] = { sceneXStart, yPos };
    endpoints[i + numXGridPoints + 1] = { frameXEnd, yPos };
  }

  return endpoints;
}();


enum SnakeDirection : uint8_t {
  UP,
  RIGHT,
  DOWN,
  LEFT,
  NONE
};

volatile SnakeDirection currentSnakeDirection = NONE;
//this "vector" field records previous directions of the head so that the rest of the body can follow up
SnakeDirection directionField[ySceneSize][xSceneSize];
volatile SnakeDirection nextTurn = NONE;

//just lists the positions of the body points in grid coordinates -> head is last
lv_point_t snakeBodyPoints[fieldSize];
uint16_t snakeSize = 0;

lv_point_t applePosition;

lv_obj_t *snake = nullptr;
lv_obj_t *apple = nullptr;
lv_obj_t *appleCounterLabel = nullptr;

volatile size_t applesCollected = 0;

volatile bool gameIsSuspended = false;
TaskHandle_t gameLoopHandle{};
TaskHandle_t musicLoopHandle{};
}

namespace Utility {
//maps the grid coordinates to the global UI coordinates
lv_point_t gridCoordinatesToGlobal(const lv_point_t &gridCoords) {
  auto [y, x] = gridCoords;
  return { sceneXStart + cellSize + x * cellSize, sceneYStart + cellSize + y * cellSize };
}
}

namespace Apple {
void updateAppleCounter(size_t newCount) {
  lv_label_set_text_fmt(appleCounterLabel, "#%d", newCount);
}

//randomly drops apple to a new postion
void dropApple() {
  do {
    applePosition.x = random(0, ySceneSize);
    applePosition.y = random(0, xSceneSize);
  } while (directionField[applePosition.x][applePosition.y] != NONE);

  const auto appleGlobalCoordinates = Utility::gridCoordinatesToGlobal(applePosition);
  lv_obj_align(apple, LV_ALIGN_CENTER, appleGlobalCoordinates.x - (screenWidth / 2), appleGlobalCoordinates.y - (screenHeight / 2));
}
void resetApple() {
  if (apple == nullptr) {
    apple = createApple(64);
  }
  dropApple();
  applesCollected = 0;
  updateAppleCounter(0);
}
}

namespace Game {
void showRestartPrompt();
}

namespace Snake {

//updates the graphics to the snake motion
void redrawSnake() {
  static lv_point_t sceneCoordinates[fieldSize];

  if (gameIsSuspended) {
    return;
  }

  for (size_t j = 0; j < snakeSize; ++j) {
    sceneCoordinates[j] = Utility::gridCoordinatesToGlobal(snakeBodyPoints[j]);
  }

  lv_line_set_points(snake, sceneCoordinates, snakeSize);
}

//moves the snake
void updateSnake() {
  if (gameIsSuspended) {
    return;
  }

  auto [headY, headX] = snakeBodyPoints[snakeSize - 1];
  auto [tailY, tailX] = snakeBodyPoints[0];

  //snake has either hit the wall or itself
  if ((headX >= xSceneSize || headY >= ySceneSize || headX < 0 || headY < 0) || directionField[headY][headX] != NONE) {
    gameIsSuspended = true;
    Game::showRestartPrompt();
    return;
  }

  //below we detrimen where the head goes this frame
  SnakeDirection directionAtHead = NONE;
  if (nextTurn != NONE) {
    auto appliedTurn = static_cast<int>(currentSnakeDirection) + (nextTurn == RIGHT ? 1 : -1);
    directionAtHead = static_cast<SnakeDirection>(appliedTurn > 3 ? 0 : appliedTurn < 0 ? 3
                                                                                        : appliedTurn);

    currentSnakeDirection = directionAtHead;
    nextTurn = NONE;
  } else {
    directionAtHead = currentSnakeDirection;
  }
  directionField[headY][headX] = directionAtHead;

  //if the head is currently on the apple position -> grow the snake by 1
  if (headY == applePosition.x && headX == applePosition.y) {
    //in what direction to grow the snake
    int16_t growX = headX + (directionAtHead == RIGHT ? 1 : (directionAtHead == LEFT ? -1 : 0));
    uint16_t growY = headY + (directionAtHead == DOWN ? 1 : (directionAtHead == UP ? -1 : 0));

    //grows the snake
    snakeBodyPoints[snakeSize] = { growY, growX };
    directionField[growY][growX] = directionAtHead;
    snakeSize += 1;

    Apple::dropApple();
    Apple::updateAppleCounter(++applesCollected);
  }


  //all body parts are translated depending on the direction specified in the direction field
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

  //just to make sure all unoccupied cells are marked with NONE
  directionField[tailY][tailX] = NONE;
}

void resetSnake() {
  memset(directionField, NONE, sizeof(directionField));

  snakeBodyPoints[0] = { 0, 0 };
  snakeBodyPoints[1] = { 0, 1 };
  snakeBodyPoints[2] = { 0, 2 };

  directionField[0][0] = RIGHT;
  directionField[0][1] = RIGHT;
  directionField[0][2] = NONE;

  if (snake == nullptr) {
    snake = createSnake();
  }
  currentSnakeDirection = RIGHT;
  snakeSize = 3;

  redrawSnake();
}

}

namespace Game {

void resetGameState() {
  Snake::resetSnake();
  Apple::resetApple();
  gameIsSuspended = false;
}

void restartPromptHandler(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_current_target(e);
  lv_msgbox_close(obj);

  resetGameState();
}

void showRestartPrompt() {
  static const char *btns[] = { LV_SYMBOL_REFRESH " Restart?", "" };

  lv_obj_t *mbox1 = lv_msgbox_create(NULL, "Game over", "No worries, you\ncan have another go!", btns, true);
  lv_obj_add_event_cb(mbox1, restartPromptHandler, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_center(mbox1);
}
}

namespace UiCallbacks {
static void leftEventHandler(lv_event_t *e) {
  if (nextTurn == NONE && !gameIsSuspended) {
    nextTurn = LEFT;
  }
}
static void rightEventHandler(lv_event_t *e) {
  if (nextTurn == NONE && !gameIsSuspended) {
    nextTurn = RIGHT;
  }
}
static void pauseEventHandler(lv_event_t *e) {
  const static auto pausedColor = lv_color_hex(0xC70202);
  const static auto unpausedColor = lv_palette_main(LV_PALETTE_BLUE);

  lv_obj_t *pauseBtn = lv_event_get_current_target(e);

  gameIsSuspended = !gameIsSuspended;

  if (gameIsSuspended) {
    lv_obj_set_style_bg_color(pauseBtn, pausedColor, 0);
    vTaskSuspend(gameLoopHandle);
    vTaskSuspend(musicLoopHandle);
  } else {
    lv_obj_set_style_bg_color(pauseBtn, unpausedColor, 0);
    vTaskResume(gameLoopHandle);
    vTaskResume(musicLoopHandle);
  }
}

static void volumeSliderEventHandler(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target(e);
  int val = lv_slider_get_value(slider);

  Music::setVolume(val);
}
}

namespace Ui {
void initUi() {
  auto leftButton = createGameButtons(-100, 90, "Left", UiCallbacks::leftEventHandler);
  auto rightButton = createGameButtons(100, 90, "Right", UiCallbacks::rightEventHandler);
  auto pauseButton = createGameButtons(0, 90, "Pause", UiCallbacks::pauseEventHandler);
  auto gameLogo = createGameLogo(0, -80);

  auto volumeSlider = createVolumeSlider(0, 21, UiCallbacks::volumeSliderEventHandler);
  lv_obj_align(volumeSlider, LV_ALIGN_CENTER, -100, -100);

  auto appleCounterImg = createApple(96);
  lv_obj_align(appleCounterImg, LV_ALIGN_CENTER, 100, -100);

  appleCounterLabel = createAppleCounterLabel();
  lv_obj_align(appleCounterLabel, LV_ALIGN_CENTER, 120, -80);

  lv_obj_t *gameFrame = nullptr;
  lv_obj_t *gameGrid = nullptr;
  createGameScene(sceneCorners, gridEndpoints, numXGridPoints + numYGridPoints, &gameFrame, &gameGrid);
}
}


void inGameLoop(void *pvParameters) {
  for (;;) {
    Snake::updateSnake();
    Snake::redrawSnake();
    vTaskDelay(snakeUpdateInterval / portTICK_PERIOD_MS);
  }
}

void uiLoop(void *pvParameters) {
  for (;;) {
    Drivers::update();
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void musicLoop(void *pvParameters) {
  for (;;) {
    Music::updateMusic();
    vTaskDelay(1);
  }
}

void setup() {
  Serial.begin(115200);

  Drivers::initHardware();
  Music::initAudio(20);

  Ui::initUi();
  Game::resetGameState();

  //important: arduino runs on core 1 while events run on core 0 to guarantee smooth music playback
  auto inGameLoopTask = xTaskCreatePinnedToCore(inGameLoop, "Game loop", 8192, nullptr, 1, &gameLoopHandle, 0);
  auto musicLoopTask = xTaskCreatePinnedToCore(musicLoop, "Music loop", 8192, nullptr, 1, &musicLoopHandle, 0);
  auto uiLoopTask = xTaskCreatePinnedToCore(uiLoop, "UI loop", 8192, nullptr, 1, nullptr, 0);

  if (inGameLoopTask != pdPASS || uiLoopTask != pdPASS || musicLoopTask != pdPASS) {
    Serial.println("Failed to create tasks!");
    abort();
  }
}

void loop() {
}