#include "LVGL_Example.h"
#include <math.h>

/* ---------- CONFIG ---------- */
#define MAX_TARGETS 10
#define CENTER_X 206
#define CENTER_Y 206
#define RADAR_RADIUS 180
#define MAX_RANGE_M 30.0f

/* ---------- OBJECTS ---------- */
static lv_obj_t *radar;
static lv_obj_t *center_dot;
static lv_obj_t *targets[MAX_TARGETS];
static lv_obj_t *pulse[MAX_TARGETS];
static lv_obj_t *rings[3];
static lv_obj_t *sweep_line;

/* ---------- SWEEP ---------- */
static lv_point_t sweep_points[2];
static float sweep_angle = 0;

/* ---------- PULSE STATE ---------- */
static float pulse_radius[MAX_TARGETS] = {0};


/* ---------- PAGES ---------- */
PageType currentPage = PAGE_RADAR;

/* ---------- MESSAGE UI ---------- */
static lv_obj_t *msg_label;

/* ---------- STATUS UI ---------- */
static lv_obj_t *status_container;
/* ---------- INIT ---------- */
void Radar_UI()
{
    lv_obj_clean(lv_scr_act());

    radar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(radar, 412, 412);
    lv_obj_center(radar);

    lv_obj_set_style_bg_color(radar, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(radar, 0, 0);
    lv_obj_clear_flag(radar, LV_OBJ_FLAG_SCROLLABLE);

    /* CENTER */
    center_dot = lv_obj_create(radar);
    lv_obj_set_size(center_dot, 14, 14);
    lv_obj_set_style_radius(center_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(center_dot, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_border_width(center_dot, 0, 0);
    lv_obj_align(center_dot, LV_ALIGN_CENTER, 0, 0);

    /* RINGS */
    for(int i=0;i<3;i++)
    {
        rings[i] = lv_obj_create(radar);
        int size = 120 + i*100;

        lv_obj_set_size(rings[i], size, size);
        lv_obj_center(rings[i]);

        lv_obj_set_style_radius(rings[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_opa(rings[i], LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_color(rings[i], lv_color_hex(0x003300), 0);
        lv_obj_set_style_border_width(rings[i], 1, 0);
    }

    /* TARGETS + PULSE */
    for(int i=0;i<MAX_TARGETS;i++)
    {
        /* DOT */
        targets[i] = lv_obj_create(radar);
        lv_obj_set_size(targets[i], 14, 14);
        lv_obj_set_style_radius(targets[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(targets[i], lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_border_width(targets[i], 0, 0);
        lv_obj_add_flag(targets[i], LV_OBJ_FLAG_HIDDEN);

        /* PULSE */
        pulse[i] = lv_obj_create(radar);

        lv_obj_set_size(pulse[i], 10, 10);

        /* 🔥 FULL CIRCLE */
        lv_obj_set_style_radius(pulse[i], LV_RADIUS_CIRCLE, 0);

        /* 🔥 WHITE */
        lv_obj_set_style_bg_color(pulse[i], lv_color_hex(0xFFFFFF), 0);

        /* 🔥 NO BORDER (IMPORTANT → removes V shape bug) */
        lv_obj_set_style_border_width(pulse[i], 0, 0);

        /* 🔥 SOFT GLOW BASE */
        lv_obj_set_style_bg_opa(pulse[i], LV_OPA_30, 0);
        lv_obj_add_flag(pulse[i], LV_OBJ_FLAG_HIDDEN);
    }

    /* SWEEP */
    sweep_line = lv_line_create(radar);

    lv_obj_set_size(sweep_line, 412, 412);
    lv_obj_center(sweep_line);

    lv_obj_set_style_line_width(sweep_line, 2, 0);
    lv_obj_set_style_line_color(sweep_line, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_line_opa(sweep_line, LV_OPA_70, 0);

    sweep_points[0].x = CENTER_X;
    sweep_points[0].y = CENTER_Y;
    sweep_points[1].x = CENTER_X;
    sweep_points[1].y = CENTER_Y;

    lv_line_set_points(sweep_line, sweep_points, 2);
}

/* ---------- UPDATE ---------- */
void Radar_Update(RadarData *targets_data, int count, float myHeading)
{
    for(int i=0;i<MAX_TARGETS;i++)
    {
        if(i >= count)
        {
            lv_obj_add_flag(targets[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(pulse[i], LV_OBJ_FLAG_HIDDEN);
            continue;
        }

        float rel = targets_data[i].heading - myHeading;

        while(rel < -180) rel += 360;
        while(rel > 180) rel -= 360;

        float rad = rel * 3.14159 / 180.0;

        float dist = targets_data[i].distance;
        float scale = dist / MAX_RANGE_M;
        if(scale > 1.0f) scale = 1.0f;

        int r = scale * RADAR_RADIUS;

        int x = r * sin(rad);
        int y = -r * cos(rad);

        /* DOT */
        lv_obj_clear_flag(targets[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(targets[i], LV_ALIGN_CENTER, x, y);

        /* PULSE ANIMATION */
        pulse_radius[i] += 3;
        if(pulse_radius[i] > 30) pulse_radius[i] = 0;

        int size = 10 + pulse_radius[i];

        lv_obj_clear_flag(pulse[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_size(pulse[i], size, size);
        lv_obj_align(pulse[i], LV_ALIGN_CENTER, x, y);

        /* FADE */
        /* FADE (GLOW EFFECT) */
        int opa = 120 - (pulse_radius[i] * 4);
        if(opa < 0) opa = 0;

        lv_obj_set_style_bg_opa(pulse[i], opa, 0);
    }

    /* SWEEP */
    sweep_angle += 8;   // faster sweep
    if(sweep_angle > 360) sweep_angle = 0;

    float rad = sweep_angle * 3.14159 / 180.0;

    sweep_points[0].x = CENTER_X;
    sweep_points[0].y = CENTER_Y;

    sweep_points[1].x = CENTER_X + cos(rad) * RADAR_RADIUS;
    sweep_points[1].y = CENTER_Y + sin(rad) * RADAR_RADIUS;

    lv_line_set_points(sweep_line, sweep_points, 2);
}

void Messages_UI()
{
    lv_obj_clean(lv_scr_act());

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    msg_label = lv_label_create(scr);

    lv_label_set_text(msg_label, "NO MESSAGES");
    lv_obj_center(msg_label);
}

void Status_UI()
{
    lv_obj_clean(lv_scr_act());

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    status_container = lv_obj_create(scr);
    lv_obj_set_size(status_container, 400, 380);
    lv_obj_center(status_container);

    lv_obj_set_style_bg_opa(status_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(status_container, 0, 0);

    lv_obj_set_flex_flow(status_container, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *title = lv_label_create(status_container);
    lv_label_set_text(title, "SELF STATUS");

    lv_obj_t *self = lv_label_create(status_container);
    lv_label_set_text(self,
        "HR: 0\nHEALTH: DEAD\nLAT: 22.82\nLON: 75.94");

    lv_obj_t *others = lv_label_create(status_container);
    lv_label_set_text(others,
        "\nOTHERS:\nNo data");
}


void Switch_Page(int dir)
{
    if(dir > 0) // right swipe
    {
        currentPage = (PageType)((currentPage + 1) % 3);
    }
    else // left swipe
    {
        currentPage = (PageType)((currentPage + 2) % 3);
    }

    if(currentPage == PAGE_RADAR)
        Radar_UI();
    else if(currentPage == PAGE_MESSAGES)
        Messages_UI();
    else if(currentPage == PAGE_STATUS)
        Status_UI();
}