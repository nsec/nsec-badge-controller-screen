#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "esp_vfs.h"

#include "FX.h"
#include "lv_conf.h"
#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"

#include "badge/mesh/main.h"
#include "badge/mesh/ops.h"
#include "screens/debug.h"
#include "lv_utils.h"
#include "save.h"
#include "neopixel.h"
#include "disk.h"
#include "badge/mesh/ops/neopixel.h"
#include "badge/mesh/ops/set_name.h"
#include "badge/mesh/ops/partyline.h"

static const char *TAG = "display";

static lv_obj_t *tab_view;
static lv_obj_t *kb;

static lv_obj_t *wifi_info_container;
static lv_obj_t *disk_info_container;
static lv_obj_t *mood_controls_container;
static lv_obj_t *chat_container;

static lv_obj_t *chat_history, *chat_message;
static time_t chat_last_received_message = 0;

typedef struct debug_tabs debug_tabs_t;

typedef lv_obj_t *(* tab_init_cb_t)(debug_tabs_t *tab);

struct debug_tabs {
    int id;
    const char *name;
    lv_obj_t *tab, *enable_switch;
    bool enabled;
    tab_init_cb_t init;
};

static lv_obj_t *tab_mesh_init(debug_tabs_t *tab);
static lv_obj_t *tab_wifi_init(debug_tabs_t *tab);
static lv_obj_t *tab_disk_init(debug_tabs_t *tab);
static lv_obj_t *tab_mood_init(debug_tabs_t *tab);
static lv_obj_t *tab_chat_init(debug_tabs_t *tab);

debug_tabs_t debug_tabs[debug_tab::count] = {
    { .name = "Mesh", .init = &tab_mesh_init },
    { .name = "Wifi", .init = tab_wifi_init },
    { .name = "SD", .init = tab_disk_init },
    { .name = "Mood", .init = tab_mood_init },
    { .name = "Chat", .init = tab_chat_init },
};

struct mesh_info_table {
    const char *name;
    lv_obj_t *value;
} mesh_info_table[mesh_info_rows::count] = {
    { .name = "Name", .value = NULL },
    { .name = "Address", .value = NULL },
    { .name = "Msg sent", .value = NULL },
    { .name = "Network time", .value = NULL },
    { .name = "Company ID", .value = NULL },
    { .name = "Group address", .value = NULL },
    { .name = "Netkey index", .value = NULL },
    { .name = "Appkey index", .value = NULL },
};

struct sd_info_table {
    const char *name;
    lv_obj_t *value;
} sd_info_table[sd_info_rows::count] = {
    { .name = "Inserted?", .value = NULL },
    { .name = "Name", .value = NULL },
    { .name = "Capacity", .value = NULL },
    { .name = "Mount point", .value = NULL },
};

static const char *FX_mode_names[] = {
    "Solid","Blink","Breathe","Wipe","Wipe Random","Random Colors","Sweep","Dynamic","Colorloop","Rainbow",
    "Scan","Scan Dual","Fade","Theater","Theater Rainbow","Running","Saw","Twinkle","Dissolve","Dissolve Rnd",
    "Sparkle","Sparkle Dark","Sparkle+","Strobe","Strobe Rainbow","Strobe Mega","Blink Rainbow","Android","Chase","Chase Random",
    "Chase Rainbow","Chase Flash","Chase Flash Rnd","Rainbow Runner","Colorful","Traffic Light","Sweep Random","Running 2","Red & Blue","Stream",
    "Scanner","Lighthouse","Fireworks","Rain","Merry Christmas","Fire Flicker","Gradient","Loading","Police","Police All",
    "Two Dots","Two Areas","Circus","Halloween","Tri Chase","Tri Wipe","Tri Fade","Lightning","ICU","Multi Comet",
    "Scanner Dual","Stream 2","Oscillate","Pride 2015","Juggle","Palette","Fire 2012","Colorwaves","Bpm","Fill Noise",
    "Noise 1","Noise 2","Noise 3","Noise 4","Colortwinkles","Lake","Meteor","Meteor Smooth","Railway","Ripple",
    "Twinklefox","Twinklecat","Halloween Eyes","Solid Pattern","Solid Pattern Tri","Spots","Spots Fade","Glitter","Candle","Fireworks Starburst",
    "Fireworks 1D","Bouncing Balls","Sinelon","Sinelon Dual","Sinelon Rainbow","Popcorn","Drip","Plasma","Percent","Ripple Rainbow",
    "Heartbeat","Pacifica","Candle Multi", "Solid Glitter","Sunrise","Phased","Twinkleup","Noise Pal", "Sine","Phased Noise",
    "Flow","Chunchun","Dancing Shadows",
};

#define MOOD_BEACON_INTERVAL 10 /* in seconds */
static TickType_t last_mood_beacon_at = 0; // used to keep sending beacons every 10 seconds
bool mood_changed; // used to refresh mood imediately when UI is interacted with

/*
    Create a row container with two labels, left is the name, right is the value, and return the value label;
*/
static lv_obj_t *create_kv_row_labels(lv_obj_t *parent, const char *name)
{
    lv_obj_t * h = lv_cont_create(parent, NULL);
    lv_cont_set_layout(h, LV_LAYOUT_PRETTY_MID);
    lv_obj_set_drag_parent(h, true);
    lv_obj_set_auto_realign(h, true);
    lv_obj_add_style(h, LV_CONT_PART_MAIN, &style_row_container);
    lv_cont_set_fit2(h, LV_FIT_MAX, LV_FIT_TIGHT);

    lv_obj_t *left = lv_label_create(h, NULL);
    lv_obj_t *right = lv_label_create(h, NULL);
    lv_label_set_text(left, name);

    return right;
}

static void toggle_tab(debug_tabs_t *tab, bool enabled)
{
    tab->enabled = enabled;
    if(Save::save_data.debug_feature_enabled[tab->id] != enabled) {
        Save::save_data.debug_feature_enabled[tab->id] = enabled;
        Save::write_save();
    }

    return;
}

static lv_obj_t *tab_mesh_init(debug_tabs_t *tab)
{
    lv_obj_t *h, *sw;
    lv_obj_t *parent = tab->tab = lv_tabview_add_tab(tab_view, tab->name);

    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_MID);

    // Mesh info container
    h = create_container(parent);
    for(int i=0; i<mesh_info_rows::count; i++) {
        mesh_info_table[i].value = create_kv_row_labels(h, mesh_info_table[i].name);
    }

    /* set values for that never changes */
    lv_label_set_text_fmt(mesh_info_table[mesh_info_rows::addr].value, "0x%04x", badge_network_info.unicast_addr);
    lv_label_set_text_fmt(mesh_info_table[mesh_info_rows::company_id].value, "0x%04x", NSEC_COMPANY_ID);
    lv_label_set_text_fmt(mesh_info_table[mesh_info_rows::group_addr].value, "0x%04x", badge_network_info.group_addr);
    lv_label_set_text_fmt(mesh_info_table[mesh_info_rows::net_idx].value, "%d", badge_network_info.net_idx);
    lv_label_set_text_fmt(mesh_info_table[mesh_info_rows::app_idx].value, "%d", badge_network_info.app_idx);

    h = create_container(parent, "BLE mesh operations");
    for(int i=0; mesh_callbacks[i].cb != NULL; i++) {
        if(mesh_callbacks[i].op == OP_VND_SET_NAME)
            /* dont show this command to delay chaos a little further */
            continue;

        lv_obj_t *value = create_kv_row_labels(h, mesh_callbacks[i].name);
        lv_label_set_text_fmt(value, "0x%04lx", (mesh_callbacks[i].op & ~(0xC00000 | NSEC_COMPANY_ID)) >> 16);
    }

    return parent;
}

static void wifi_enable_event(lv_obj_t *sw, lv_event_t event)
{
    switch(event) {
        case LV_EVENT_VALUE_CHANGED:
        {
            bool enabled = lv_switch_get_state(sw);
            toggle_tab(&debug_tabs[debug_tab::wifi], enabled);
            lv_obj_set_hidden(wifi_info_container, !debug_tabs[debug_tab::wifi].enabled);
            break;
        }
    }

    return;
}

static lv_obj_t *tab_wifi_init(debug_tabs_t *tab)
{
    lv_obj_t *h, *sw;
    lv_obj_t *parent = tab->tab = lv_tabview_add_tab(tab_view, tab->name);

    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_MID);

    // Wifi container
    h = create_container(parent);

    // Enable switch
    sw = tab->enable_switch = create_switch_with_label(h, "Enabled", tab->enabled);
    lv_obj_set_event_cb(sw, wifi_enable_event);

    // Information container
    h = wifi_info_container = create_container(parent);
    lv_obj_set_hidden(h, !tab->enabled);

    create_kv_row_labels(h, "SSID");
    create_kv_row_labels(h, "Password");

    return parent;
}

bool disk_info_displayed = false;
static char disk_current_path[1024];
static lv_obj_t *disk_list, *disk_explorer;

static void disk_enable_event(lv_obj_t *sw, lv_event_t event)
{
    switch(event) {
        case LV_EVENT_VALUE_CHANGED:
        {
            bool enabled = lv_switch_get_state(sw);
            toggle_tab(&debug_tabs[debug_tab::disk], enabled);
            lv_obj_set_hidden(disk_info_container, !debug_tabs[debug_tab::disk].enabled);

            if(enabled) {
                Disk::getInstance().enable();
            } else {
                Disk::getInstance().disable();
                disk_info_displayed = false;

                for(int i=0; i<sd_info_rows::count; i++) {
                    lv_label_set_text(sd_info_table[i].value, "-");
                }
            }
            break;
        }
    }

    return;
}

static void disk_refresh_files();

static void disk_list_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        const char *name = lv_list_get_btn_text(obj);
        bool dir = name[strlen(name)-1] == '/';

        if(!strcmp("..", name)) {
            for(int i=strlen(disk_current_path)-2;i>0 && disk_current_path[i] != '/';i--)
                disk_current_path[i] = '\0';
            disk_refresh_files();
        } else if(dir) {
            if(strlen(disk_current_path) + strlen(name) + 1 > sizeof(disk_current_path)) {
                printf("can't open dir because path is too long");
                return;
            }
            strcat(disk_current_path, name);
            disk_refresh_files();
        }
    }
}

bool disk_iter_cb(dirent *entry, void *param)
{
    char fmt[257];
    bool dir = entry->d_type == DT_DIR;
    if(dir) {
        snprintf((char *)&fmt, sizeof(fmt), "%s/", entry->d_name);
    } else {
        snprintf((char *)&fmt, sizeof(fmt), "%s", entry->d_name);
    }
    lv_obj_t *list_btn = lv_list_add_btn(disk_list, dir ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_FILE, fmt);
    lv_obj_set_event_cb(list_btn, disk_list_event_handler);

    return true;
}

static void disk_refresh_files()
{
    char dir[32];
    lv_list_clean(disk_list);

    snprintf((char *)&dir, sizeof(dir), "%s/", Disk::getInstance().getMountPoint());

    lv_obj_t *list_btn = lv_list_add_btn(disk_list, LV_SYMBOL_DIRECTORY, "..");
    lv_obj_set_event_cb(list_btn, disk_list_event_handler);
    if(!strcasecmp(disk_current_path, dir)) {
        lv_obj_set_click(list_btn, false);
        lv_btn_set_state(list_btn, LV_BTN_STATE_DISABLED);
    }

    Disk::getInstance().iterPath(disk_current_path, (disk_iter_cb_t)disk_iter_cb, NULL);
}

static lv_obj_t *tab_disk_init(debug_tabs_t *tab)
{
    lv_obj_t *h, *sw;
    lv_obj_t *parent = tab->tab = lv_tabview_add_tab(tab_view, tab->name);

    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_MID);

    // SD container
    h = create_container(parent);

    // Enable switch
    sw = tab->enable_switch = create_switch_with_label(h, "Enabled", tab->enabled);
    lv_obj_set_event_cb(sw, disk_enable_event);

    // Information container
    h = disk_info_container = create_container(parent);
    lv_obj_set_hidden(h, !tab->enabled);
    for(int i=0; i<sd_info_rows::count; i++) {
        sd_info_table[i].value = create_kv_row_labels(h, sd_info_table[i].name);
        lv_label_set_text(sd_info_table[i].value, "-");
    }

    // File explorer container
    h = disk_explorer = create_container(parent, "Explorer");
    lv_obj_set_hidden(h, true);

    disk_list = lv_list_create(h, NULL);
    lv_obj_add_style(disk_list, LV_CONT_PART_MAIN, &style_row_container);
    lv_obj_set_size(disk_list, 260, 160);
    lv_obj_align(disk_list, NULL, LV_ALIGN_CENTER, 0, 0);

    return parent;
}

static void mood_enable_event(lv_obj_t *sw, lv_event_t event)
{
    switch(event) {
        case LV_EVENT_VALUE_CHANGED:
        {
            bool enabled = lv_switch_get_state(sw);
            toggle_tab(&debug_tabs[debug_tab::mood], enabled);
            lv_obj_set_hidden(mood_controls_container, !debug_tabs[debug_tab::mood].enabled);
            break;
        }
    }

    return;
}

static void mood_brightness_event(lv_obj_t *slider, lv_event_t event)
{
    switch(event) {
        case LV_EVENT_VALUE_CHANGED:
        {
            uint8_t brightness = (uint8_t)lv_slider_get_value(slider);
            if(Save::save_data.mood_brightness != brightness) {
                Save::save_data.mood_brightness = brightness;
                Save::write_save();
            }
            mood_changed = true;
            break;
        }
    }

    return;
}

static void mood_mode_event(lv_obj_t *roller, lv_event_t event)
{
    switch(event) {
        case LV_EVENT_VALUE_CHANGED:
        {
            int selected = lv_roller_get_selected(roller);
            if(Save::save_data.mood_mode != selected) {
                Save::save_data.mood_mode = selected;
                Save::write_save();
            }
            mood_changed = true;
            break;
        }
    }

    return;
}

static void mood_color_event(lv_obj_t *cpicker, lv_event_t event)
{
    switch(event) {
        case LV_EVENT_VALUE_CHANGED:
        {
            lv_color_t color = lv_cpicker_get_color(cpicker);
            if(Save::save_data.mood_color.full != color.full) {
                Save::save_data.mood_color = color;
                Save::write_save();
            }
            mood_changed = true;
            break;
        }
    }

    return;
}

static lv_obj_t *tab_mood_init(debug_tabs_t *tab)
{
    lv_obj_t *h, *sw, *cpicker;
    lv_obj_t *parent = tab->tab = lv_tabview_add_tab(tab_view, tab->name);

    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_MID);

    // Mood container
    h = create_container(parent);

    // Enable switch
    sw = tab->enable_switch = create_switch_with_label(h, "Enabled", tab->enabled);
    lv_obj_set_event_cb(sw, mood_enable_event);

    // Controls container
    h = mood_controls_container = create_container(parent, NULL, LV_LAYOUT_PRETTY_MID);
    lv_obj_set_hidden(h, !tab->enabled);

    lv_obj_t * roller = lv_roller_create(h, NULL);
    lv_obj_add_style(roller, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_style_local_value_str(roller, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Choose ambiant mood");
    lv_roller_set_auto_fit(roller, false);
    lv_roller_set_align(roller, LV_LABEL_ALIGN_CENTER);
    lv_roller_set_visible_row_count(roller, 4);
    lv_obj_set_width(roller, 110);
    lv_obj_set_style_local_text_font(roller, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_12);
    lv_obj_set_event_cb(roller, mood_mode_event);

    char choices[1024];
    memset(choices, 0, sizeof(choices));
    int count = sizeof(NeoPixel::unlocked_mode) / sizeof(NeoPixel::unlocked_mode[0]);
    for(int i=0; i<count; i++) {
        strcat((char *)&choices, FX_mode_names[NeoPixel::unlocked_mode[i]]);
        if(i != count - 1)
            strcat((char *)&choices, "\n");
    }

    lv_roller_set_options(roller, (char *)&choices, LV_ROLLER_MODE_INIFINITE);
    lv_roller_set_selected(roller, Save::save_data.mood_mode, LV_ANIM_OFF);

    cpicker = lv_cpicker_create(h, NULL);
    lv_obj_set_size(cpicker, 130, 150);
    lv_obj_align(cpicker, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(cpicker, mood_color_event);
    lv_cpicker_set_color(cpicker, Save::save_data.mood_color);

    lv_obj_t * slider = lv_slider_create(h, NULL);
    lv_obj_set_width(slider, 250);
    lv_obj_align(slider, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(slider, mood_brightness_event);
    lv_slider_set_range(slider, 0, 128);
    lv_slider_set_value(slider, Save::save_data.mood_brightness, LV_ANIM_OFF);

    return parent;
}

static void chat_enable_event(lv_obj_t *sw, lv_event_t event)
{
    switch(event) {
        case LV_EVENT_VALUE_CHANGED:
        {
            bool enabled = lv_switch_get_state(sw);
            toggle_tab(&debug_tabs[debug_tab::chat], enabled);
            lv_obj_set_hidden(chat_container, !debug_tabs[debug_tab::chat].enabled);
            break;
        }
    }

    return;
}

static void kb_event_cb(lv_obj_t * _kb, lv_event_t e)
{
    lv_keyboard_def_event_cb(kb, e);

    if(e == LV_EVENT_APPLY) {
        const char *msg = lv_textarea_get_text(chat_message);

        if(debug_tabs[debug_tab::mood].enabled) {
            char fmt[PARTYLINE_MAX_MESSAGE_LENGTH];
            lv_color_t color = Save::save_data.mood_color;
            // lv_color_t is 16 bits, we lose some precision at the bottom of each nibble
            uint32_t rgb = (color.ch.red << (16+3)) | ((color.ch.green_l | (color.ch.green_h << 3)) << (8+2)) | (color.ch.blue << 3);

            snprintf((char *)fmt, sizeof(fmt), "#%06lx %s", rgb, msg);
            send_partyline(fmt);
        } else {
            send_partyline(msg);
        }

        lv_textarea_set_text(chat_message, "");
    }

    if(e == LV_EVENT_APPLY || e == LV_EVENT_CANCEL) {
        if(kb) {
            lv_obj_set_height(tab_view, LV_VER_RES);
            lv_obj_del(kb);
            kb = NULL;
        }
    }
}

static void chat_message_event_cb(lv_obj_t * ta, lv_event_t e)
{
    if(e == LV_EVENT_FOCUSED) {
        if(kb == NULL) {
            lv_obj_set_height(tab_view, LV_VER_RES / 2);
            kb = lv_keyboard_create(lv_scr_act(), NULL);
            lv_obj_set_event_cb(kb, kb_event_cb);

            lv_indev_wait_release(lv_indev_get_act());
        }
        lv_textarea_set_cursor_hidden(ta, false);
        lv_page_focus(debug_tabs[debug_tab::chat].tab, lv_textarea_get_label(ta), LV_ANIM_ON);
        lv_keyboard_set_textarea(kb, ta);
    } else if(e == LV_EVENT_DEFOCUSED) {
        lv_textarea_set_cursor_hidden(ta, true);
    }
}

static lv_obj_t *tab_chat_init(debug_tabs_t *tab)
{
    lv_obj_t *h, *sw, *ta;
    lv_obj_t *parent = tab->tab = lv_tabview_add_tab(tab_view, tab->name);

    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_MID);

    // Chat container
    h = create_container(parent);

    // Enable switch
    sw = tab->enable_switch = create_switch_with_label(h, "Enabled", tab->enabled);
    lv_obj_set_event_cb(sw, chat_enable_event);

    // Messages container
    h = chat_container = create_container(parent);
    lv_obj_set_hidden(h, !tab->enabled);
    lv_obj_add_style(h, LV_CONT_PART_MAIN, &style_row_container);

    // input
    ta = chat_message = lv_textarea_create(h, NULL);
    lv_cont_set_fit2(ta, LV_FIT_PARENT, LV_FIT_NONE);
    lv_textarea_set_text(ta, "");
    lv_textarea_set_placeholder_text(ta, "Message");
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_cursor_hidden(ta, true);
    lv_obj_set_event_cb(ta, chat_message_event_cb);

    lv_obj_t *label = chat_history = lv_label_create(h, NULL);
    lv_label_set_recolor(label, true);
    lv_label_set_text(label, "This is the start of chat history");
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);
    lv_obj_set_width(label, 270);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_12);

    return parent;
}

void screen_debug_init()
{
    lv_obj_clean(lv_scr_act());

    tab_view = lv_tabview_create(lv_scr_act(), NULL);

    debug_tabs[0].enabled = true;
    for(int i=1 /* skip mesh tab, always active */; i<debug_tab::count; i++) {
        debug_tabs[i].id = i;
        debug_tabs[i].enabled = Save::save_data.debug_feature_enabled[i];
    }

    for(int i=0; i<debug_tab::count; i++) {
        debug_tabs[i].init(&debug_tabs[i]);
    }

    if(debug_tabs[debug_tab::disk].enabled) {
        Disk::getInstance().enable();
    }

    return;
}

typedef struct chat_history_buffer {
    time_t most_recent;
    char *data;
    unsigned int size;
} chat_history_buffer_t;

bool partyline_each_cb(partyline_msg_t *msg, chat_history_buffer_t *buf)
{
    char fmt[PARTYLINE_MAX_MESSAGE_LENGTH + BADGE_NAME_LEN + 32];
    struct tm tm;

    gmtime_r(&msg->recv_at, &tm);

    snprintf((char *)&fmt, sizeof(fmt), "#0000ff (%02d:%02d)# #ff0000 %s#> %s\n", tm.tm_hour, tm.tm_min, msg->from, msg->msg);

    unsigned int fmt_len = strlen(fmt);
    unsigned int data_len = strlen(buf->data);
    if((data_len + fmt_len + 1) > buf->size) {
        buf->data = (char *)realloc(buf->data, buf->size + 1024);
        buf->size += 1024;
    }

    strcpy((char *)&buf->data[data_len], (char *)&fmt);

    if(buf->most_recent == 0) {
        /* collect timestamp from very first (most recent) message */
        buf->most_recent = msg->recv_at;
    }

    return true;
}

void screen_debug_loop()
{
    // update mesh info
    lv_label_set_text(mesh_info_table[mesh_info_rows::name].value, (char *)&badge_network_info.name);
    lv_label_set_text_fmt(mesh_info_table[mesh_info_rows::seq_num].value, "%lu", bt_mesh.seq);

    if(BadgeMesh::getInstance().networkTimeIsValid()) {
        struct tm tm;
        time_t t;

        BadgeMesh::getInstance().networkTimeGet(&t);
        gmtime_r(&t, &tm);
        lv_label_set_text_fmt(mesh_info_table[mesh_info_rows::network_time].value, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
    } else {
        lv_label_set_text(mesh_info_table[mesh_info_rows::network_time].value, "(not available)");
    }

    if(debug_tabs[debug_tab::mood].enabled) {
        TickType_t now = xTaskGetTickCount();
        TickType_t elapsed_ticks = now - last_mood_beacon_at;
        uint32_t elapsed_time_ms = (uint32_t)((elapsed_ticks * 1000) / configTICK_RATE_HZ);

        if(last_mood_beacon_at == 0 || elapsed_time_ms > (MOOD_BEACON_INTERVAL * 1000) || mood_changed) {
            lv_color_t color = Save::save_data.mood_color;
            uint8_t mode = Save::save_data.mood_mode;
            uint8_t brightness = Save::save_data.mood_brightness;
            // lv_color_t is 16 bits, we lose some precision at the bottom of each nibble
            uint32_t rgb = (color.ch.red << (16+3)) | ((color.ch.green_l | (color.ch.green_h << 3)) << (8+2)) | (color.ch.blue << 3);

            send_neopixel_set(MOOD_BEACON_INTERVAL, mode, brightness, rgb, 0, 1);

            mood_changed = false;
            last_mood_beacon_at = now;
        }
    }

    if(debug_tabs[debug_tab::chat].enabled) {
        if(partyline_received_since(chat_last_received_message)) {
            chat_history_buffer_t buf = {
                .most_recent = 0,
                .data = (char *)malloc(1024),
                .size = 1024,
            };
            memset(buf.data, 0, buf.size);

            partyline_each((partyline_each_cb_t)&partyline_each_cb, &buf);
            chat_last_received_message = buf.most_recent;

            lv_label_set_text(chat_history, buf.data);
            free(buf.data);
        }
    }

    if(debug_tabs[debug_tab::disk].enabled) {
        if(!disk_info_displayed && Disk::getInstance().getCardState() == Disk::CardState::Present) {
            sdmmc_card_t *card = Disk::getInstance().getCardInfo();
            char name[10];

            snprintf((char *)&name, sizeof(name), card->cid.name);

            lv_label_set_text(sd_info_table[sd_info_rows::inserted].value, "Yes");
            lv_label_set_text(sd_info_table[sd_info_rows::name].value, name);
            lv_label_set_text_fmt(sd_info_table[sd_info_rows::capacity].value, "%lluMB",
                ((uint64_t) card->csd.capacity) * card->csd.sector_size / (1024 * 1024));

            snprintf((char *)&disk_current_path, sizeof(disk_current_path), "%s/", Disk::getInstance().getMountPoint());
            lv_label_set_text(sd_info_table[sd_info_rows::mount].value, Disk::getInstance().getMountPoint());

            disk_refresh_files();
            lv_obj_set_hidden(disk_explorer, false);

            disk_info_displayed = true;
        }

        if(disk_info_displayed && Disk::getInstance().getCardState() != Disk::CardState::Present) {
            sdmmc_card_t *card = Disk::getInstance().getCardInfo();

            lv_label_set_text(sd_info_table[sd_info_rows::inserted].value, "No");
            lv_label_set_text(sd_info_table[sd_info_rows::name].value, "-");
            lv_label_set_text(sd_info_table[sd_info_rows::capacity].value, "-");
            lv_label_set_text(sd_info_table[sd_info_rows::mount].value, "-");

            lv_obj_set_hidden(disk_explorer, true);

            disk_info_displayed = false;
        }
    }

    return;
}
