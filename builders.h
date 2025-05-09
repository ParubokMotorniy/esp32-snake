#include "widgets/lv_img.h"
#ifndef BUILDER_H
#define BUILDER_H

#include "widgets/lv_line.h"
#include <cstddef>
#include "misc/lv_area.h"
#include "core/lv_obj_pos.h"
#include "font/lv_symbol_def.h"
#include "core/lv_obj_style.h"
#include "misc/lv_color.h"
#include "misc/lv_style.h"
#include "widgets/lv_label.h"
#include "core/lv_obj.h"
#include "core/lv_event.h"
#include "lvgl.h"

inline lv_obj_t *createGameButtons(int xPos, int yPos, const char *labelText, void (*touchHandler)(lv_event_t *)) {
  lv_obj_t *label;
  lv_obj_t *btn = lv_btn_create(lv_scr_act());
  lv_obj_add_event_cb(btn, touchHandler, LV_EVENT_PRESSED, NULL);
  lv_obj_align(btn, LV_ALIGN_CENTER, xPos, yPos);
  label = lv_label_create(btn);
  lv_label_set_text(label, labelText);
  lv_obj_center(label);

  return btn;
}

inline void createGameScene(const lv_point_t frameCorners[5], const lv_point_t *gridEndpoints, size_t numPoints, lv_obj_t **gameFrame, lv_obj_t **gameGrid) {
  static lv_style_t styleLine;
  lv_style_init(&styleLine);
  lv_style_set_line_width(&styleLine, 4);
  lv_style_set_line_color(&styleLine, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_line_rounded(&styleLine, true);

  *gameFrame = lv_line_create(lv_scr_act());
  lv_line_set_points(*gameFrame, frameCorners, 5); 
  lv_obj_add_style(*gameFrame, &styleLine, 0);

  static lv_style_t gridStyle;
  lv_style_init(&gridStyle);
  lv_style_set_line_width(&gridStyle, 2);
  lv_style_set_line_color(&gridStyle, lv_palette_main(LV_PALETTE_BLUE));

  for(size_t i = 0; i < numPoints; i += 2)
  {
    lv_obj_t *gridLine;
    gridLine = lv_line_create(lv_scr_act());
    lv_line_set_points(gridLine, gridEndpoints + i, 2);
    lv_obj_add_style(gridLine, &gridStyle, 0);
  }
}

inline lv_obj_t *createGameLogo(int xPos, int yPos) {
  static const char *logo = "Snake";
  // " _____             _    _____ \n"
  // "/  ___|           | |  |  ___|\n"
  // "\ `--. _ __   __ _| | _| |__  \n"
  // " `--. \ '_ \ / _` | |/ /  __| \n"
  // "/\__/ / | | | (_| |   <| |___ \n"
  // "\____/|_| |_|\__,_|_|\_\____/ \n";

  // static lv_style_t style_shadow;
  // lv_style_init(&style_shadow);
  // lv_style_set_text_opa(&style_shadow, LV_OPA_30);
  // lv_style_set_text_color(&style_shadow, lv_color_black());

  static lv_style_t style_main;
  lv_style_init(&style_main);
  lv_style_set_text_color(&style_main, lv_color_hex(0x06c723));
  lv_style_set_opa(&style_main, LV_OPA_100);
  lv_style_set_text_align(&style_main, LV_TEXT_ALIGN_CENTER);

  /*Create a label for the shadow first (it's in the background)*/
  // lv_obj_t *shadow_label = lv_label_create(lv_scr_act());
  // lv_obj_add_style(shadow_label, &style_shadow, 0);

  /*Create the main label*/
  lv_obj_t *main_label = lv_label_create(lv_scr_act());
  lv_label_set_text_static(main_label, LV_SYMBOL_LEFT "SnakE" LV_SYMBOL_RIGHT);
  lv_obj_add_style(main_label, &style_main, 0);
  lv_obj_set_size(main_label, 80, 40);

  /*Set the same text for the shadow label*/
  // lv_label_set_text_static(shadow_label, logo);

  /*Position the main label*/
  lv_obj_align(main_label, LV_ALIGN_CENTER, xPos, yPos);

  /*Shift the second label down and to the right by 2 pixel*/
  // lv_obj_align_to(shadow_label, main_label, LV_ALIGN_TOP_LEFT, 2, 2);

  return main_label;
}

inline lv_obj_t *createApple(int size)
{
  // static lv_style_t appleStyle;
  // lv_style_init(&appleStyle);
  // lv_style_set_line_width(&appleStyle, 5);
  // lv_style_set_line_color(&appleStyle, lv_palette_main(LV_PALETTE_RED));
  // lv_style_set_line_rounded(&appleStyle, true);

  // lv_obj_t *apple = lv_line_create(lv_scr_act());
  // lv_obj_add_style(apple, &appleStyle, 0);
  LV_IMG_DECLARE(apple_img);
  lv_obj_t *appleObj = lv_img_create(lv_scr_act());
  lv_img_set_src(appleObj, &apple_img);
  lv_obj_align(appleObj, LV_ALIGN_CENTER, 0, 0);
  lv_img_set_antialias(appleObj, true);
  lv_img_set_zoom(appleObj, size);
  
  return appleObj;
}

inline lv_obj_t *createSnake()
{
  static lv_style_t snakeStyle;
  lv_style_init(&snakeStyle);
  lv_style_set_line_width(&snakeStyle, 5);
  lv_style_set_line_color(&snakeStyle, lv_color_hex(0x06c723));
  lv_style_set_line_rounded(&snakeStyle, false);

  lv_obj_t *snake = lv_line_create(lv_scr_act());
  lv_obj_add_style(snake, &snakeStyle, 0);

  return snake;
}

#endif