// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.4
// LVGL version: 8.3.6
// Project name: SquareLine_Project

#include "../ui.h"

void ui_Screen1_screen_init(void)
{
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_background = lv_img_create(ui_Screen1);
    lv_img_set_src(ui_background, &ui_img_background_png);
    lv_obj_set_width(ui_background, LV_SIZE_CONTENT);   /// 480
    lv_obj_set_height(ui_background, LV_SIZE_CONTENT);    /// 480
    lv_obj_set_align(ui_background, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_background, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_background, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_indices = lv_img_create(ui_Screen1);
    lv_img_set_src(ui_indices, &ui_img_numbers_png);
    lv_obj_set_width(ui_indices, LV_SIZE_CONTENT);   /// 480
    lv_obj_set_height(ui_indices, LV_SIZE_CONTENT);    /// 480
    lv_obj_set_align(ui_indices, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_indices, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_indices, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_hand = lv_img_create(ui_Screen1);
    lv_img_set_src(ui_hand, &ui_img_hand_png);
    lv_obj_set_width(ui_hand, LV_SIZE_CONTENT);   /// 480
    lv_obj_set_height(ui_hand, LV_SIZE_CONTENT);    /// 480
    lv_obj_set_align(ui_hand, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_hand, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_hand, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

}