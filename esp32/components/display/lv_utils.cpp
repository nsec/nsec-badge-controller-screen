#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "lv_conf.h"
#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"

#include "lv_utils.h"

lv_style_t style_box, style_row_container, style_no_top_margin;

lv_obj_t *create_switch_with_label(lv_obj_t *parent, const char *text, bool enabled)
{
    lv_obj_t * h = lv_cont_create(parent, NULL);
    lv_cont_set_layout(h, LV_LAYOUT_ROW_MID);
    lv_obj_set_drag_parent(h, true);
    lv_obj_set_auto_realign(h, true);
    lv_obj_add_style(h, LV_CONT_PART_MAIN, &style_row_container);
    lv_cont_set_fit2(h, LV_FIT_MAX, LV_FIT_TIGHT);

    lv_obj_t *sw = lv_switch_create(h, NULL);
    if(enabled) {
        lv_switch_on(sw, LV_ANIM_OFF);
    } else {
        lv_switch_off(sw, LV_ANIM_OFF);
    }

    lv_obj_t *label = lv_label_create(h, NULL);
    lv_label_set_text(label, text);
    lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);

    return sw;
}

lv_obj_t *create_container(lv_obj_t *parent, const char *title, lv_layout_t layout)
{
    lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);
    lv_coord_t grid_w = lv_page_get_width_grid(parent, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1) - 10;

    lv_obj_t *h = lv_cont_create(parent, NULL);
    lv_cont_set_layout(h, layout);
    lv_obj_add_style(h, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_drag_parent(h, true);
    lv_obj_set_auto_realign(h, true);

    if(title) {
        lv_obj_set_style_local_value_str(h, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, title);
    } else {
        lv_obj_add_style(h, LV_CONT_PART_MAIN, &style_no_top_margin);
    }

    lv_cont_set_fit2(h, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_width(h, grid_w);

    return h;
}

void util_styles_init()
{
    lv_style_init(&style_box);
    lv_style_set_value_align(&style_box, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    lv_style_set_value_ofs_y(&style_box, LV_STATE_DEFAULT, - LV_DPX(10));
    lv_style_set_margin_top(&style_box, LV_STATE_DEFAULT, LV_DPX(30));

    lv_style_init(&style_no_top_margin);
    lv_style_set_margin_top(&style_no_top_margin, LV_STATE_DEFAULT, 0);

    lv_style_init(&style_row_container);
    lv_style_set_border_width(&style_row_container, LV_STATE_DEFAULT, 0);
    lv_style_set_margin_top(&style_row_container, LV_STATE_DEFAULT, 0);
    lv_style_set_margin_bottom(&style_row_container, LV_STATE_DEFAULT, 0);
    lv_style_set_margin_left(&style_row_container, LV_STATE_DEFAULT, 0);
    lv_style_set_margin_right(&style_row_container, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_top(&style_row_container, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&style_row_container, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_left(&style_row_container, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_right(&style_row_container, LV_STATE_DEFAULT, 0);
    return;
}
