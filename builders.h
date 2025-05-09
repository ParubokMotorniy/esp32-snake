#ifndef BUILDER_H
#define BUILDER_H

#include "lvgl.h"
#include "core/lv_disp.h"
#include "widgets/lv_img.h"
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

inline lv_obj_t *createGameButtons(int xPos, int yPos, const char *labelText, void (*touchHandler)(lv_event_t *)) {
  lv_obj_t *label;
  lv_obj_t *btn = lv_btn_create(lv_scr_act());
  lv_obj_set_width(btn, 70);
  lv_obj_set_height(btn, 30);
  lv_obj_add_event_cb(btn, touchHandler, LV_EVENT_PRESSED, NULL);
  lv_obj_align(btn, LV_ALIGN_CENTER, xPos, yPos);
  label = lv_label_create(btn);
  lv_label_set_text(label, labelText);
  lv_obj_center(label);

  return btn;
}

inline lv_obj_t *createVolumeSlider(int min, int max, lv_event_cb_t on_change_cb) {
  lv_obj_t *slider = lv_slider_create(lv_scr_act());

  lv_slider_set_range(slider, min, max);

  lv_obj_set_width(slider, 80);
  lv_obj_set_height(slider, 10);
  lv_obj_center(slider);

  static lv_style_t sliderStyle;
  lv_style_init(&sliderStyle);
  lv_style_set_bg_color(&sliderStyle, lv_color_hex(0x2196F3));
  lv_style_set_bg_opa(&sliderStyle, LV_OPA_COVER);
  lv_style_set_pad_all(&sliderStyle, 0);
  lv_style_set_border_width(&sliderStyle, 0);
  lv_style_set_outline_width(&sliderStyle, 0);
  lv_style_set_radius(&sliderStyle, 3);

  lv_obj_add_style(slider, &sliderStyle, LV_PART_MAIN);
  lv_obj_add_style(slider, &sliderStyle, LV_PART_INDICATOR);

  static lv_style_t style_knob;
  lv_style_init(&style_knob);

  lv_style_set_bg_color(&style_knob, lv_color_hex(0x64B5F6));  
  lv_style_set_bg_opa(&style_knob, LV_OPA_COVER);

  lv_style_set_width(&style_knob, 10); 
  lv_style_set_height(&style_knob, 10); 

  lv_style_set_radius(&style_knob, LV_RADIUS_CIRCLE);
  lv_obj_add_style(slider, &style_knob, LV_PART_KNOB);

  if (on_change_cb) {
    lv_obj_add_event_cb(slider, on_change_cb, LV_EVENT_VALUE_CHANGED, NULL);
  }

  return slider;
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

  for (size_t i = 0; i < numPoints; i += 2) {
    lv_obj_t *gridLine;
    gridLine = lv_line_create(lv_scr_act());
    lv_line_set_points(gridLine, gridEndpoints + i, 2);
    lv_obj_add_style(gridLine, &gridStyle, 0);
  }
}

inline lv_obj_t *createGameLogo(int xPos, int yPos) {
  static const char *logo = "Snake";

  static lv_style_t style_main;
  lv_style_init(&style_main);
  lv_style_set_text_color(&style_main, lv_color_hex(0x06c723));
  lv_style_set_opa(&style_main, LV_OPA_100);
  lv_style_set_text_align(&style_main, LV_TEXT_ALIGN_CENTER);

  lv_obj_t *main_label = lv_label_create(lv_scr_act());
  lv_label_set_text_static(main_label, LV_SYMBOL_LEFT "SnakE" LV_SYMBOL_RIGHT);
  lv_obj_add_style(main_label, &style_main, 0);
  lv_obj_set_size(main_label, 80, 50);

  lv_obj_align(main_label, LV_ALIGN_CENTER, xPos, yPos);

  return main_label;
}

inline lv_obj_t *createAppleCounterLabel() {
  static lv_style_t counterStyle;
  lv_style_init(&counterStyle);
  lv_style_set_text_color(&counterStyle, lv_color_black());
  lv_style_set_opa(&counterStyle, LV_OPA_100);
  lv_style_set_text_align(&counterStyle, LV_TEXT_ALIGN_CENTER);

  lv_obj_t *counterLabel = lv_label_create(lv_scr_act());
  lv_obj_add_style(counterLabel, &counterStyle, 0);
  lv_obj_set_size(counterLabel, 80, 50);

  lv_obj_align(counterLabel, LV_ALIGN_CENTER, 0, 0);

  return counterLabel;
}

inline lv_obj_t *createApple(int size) {
  LV_IMG_DECLARE(apple_img);
  lv_obj_t *appleObj = lv_img_create(lv_scr_act());
  lv_img_set_src(appleObj, &apple_img);
  lv_obj_align(appleObj, LV_ALIGN_CENTER, 0, 0);
  lv_img_set_antialias(appleObj, true);
  lv_img_set_zoom(appleObj, size);

  return appleObj;
}

inline lv_obj_t *createSnake() {
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