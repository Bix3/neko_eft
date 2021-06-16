#ifndef settings_hpp
#define settings_hpp
#pragma once
#include <nlohmann/json.hpp>

#include <imgui.h>


#define next_setting_name                                  \
    setting_name[0] = 'a' + setting_indexer / ('z' - 'a'); \
    setting_name[1] = 'a' + setting_indexer % ('z' - 'a'); \
    setting_indexer++;

#define setting(var)                     \
    if(out)                              \
        js[setting_name] = var;          \
    else if(!js[setting_name].is_null()) \
        var = js[setting_name];          \
    next_setting_name

#define setting_imcolor(var)                                                  \
    if(out) {                                                                 \
        js[setting_name]                   = var.color.Value.x;               \
        next_setting_name js[setting_name] = var.color.Value.y;               \
        next_setting_name js[setting_name] = var.color.Value.z;               \
        next_setting_name js[setting_name] = var.color.Value.w;               \
        next_setting_name js[setting_name] = var.dropoff_type;                \
    }                                                                         \
    else {                                                                    \
        if(!js[setting_name].is_null())                                       \
            var.color.Value.x = js[setting_name];                             \
        next_setting_name if(!js[setting_name].is_null()) var.color.Value.y = \
            js[setting_name];                                                 \
        next_setting_name if(!js[setting_name].is_null()) var.color.Value.z = \
            js[setting_name];                                                 \
        next_setting_name if(!js[setting_name].is_null()) var.color.Value.w = \
            js[setting_name];                                                 \
        next_setting_name if(!js[setting_name].is_null()) var.dropoff_type =  \
            js[setting_name];                                                 \
    }                                                                         \
    next_setting_name

#define rgba(r,g,b,a) ImColor(r, g, b, a * 255)

#include <math.h>

inline struct settings_struct {
    struct color_var_t {
        color_var_t() = default;
        color_var_t(ImColor col) { color = col; }

        ImColor get_color_bydist(float curdist, float maxdist)
        {
            switch (dropoff_type) {
            case 0: // Constant
                return color;
            case 1: // Linear
                return ImColor(color.Value.x,
                    color.Value.y,
                    color.Value.z,
                    color.Value.w * (curdist / maxdist));
            case 2: // Non-linear
            {
                // Limit to 0.f -> 25.f range for a nice root drop-off
                // We're using a simple sqrt(x) to 25x for a cleaner drop-off
                return ImColor(
                    color.Value.x,
                    color.Value.y,
                    color.Value.z,
                    color.Value.w *
                    sqrt(25.f - curdist * 25.f / maxdist) /
                    5.f);
            }
            }
        }

        ImColor color;
        int     dropoff_type = 2;
    };

    struct esp_item_t {
        bool  enabled = true;
        bool  name = true;
        bool  distance = false;
        bool  marker = false;
        float draw_distance = 120.f;
    };

    /*Visuals*/
    bool visuals = true;
    bool player_esp = true;
    bool esp_players_box = true;
    bool esp_players_show_name = true;
    bool esp_players_show_distance = true;
    bool esp_skeleton;

    esp_item_t esp_corpses;


    //not working as of rn
    bool esp_players_weapon;

    bool scav_esp = true;
    bool crosshair = true;

    float esp_scav_distance = 300;
    float esp_players_distance = 300;

    color_var_t color_corpses = color_var_t(ImColor(45, 45, 45));
    color_var_t m_player_color = color_var_t(rgba(255,255,255, 1.0));
    color_var_t m_color_crosshair = color_var_t(rgba(230, 126, 34, 1.0));
    color_var_t m_color_fov_circle = color_var_t(rgba(230, 126, 34, 1.0));

    /*Aimbot*/
    bool aimbot;
    bool ricochet;
    int aimbot_key;

    int   aimbot_aimspot = 0;

    float aimbot_max_fov = 100.f;

    /*Misc*/
    bool no_fall_damage;
    bool timescale;
    bool infinite_stamine;
    bool no_sway;
    bool no_recoil;
    bool no_spread;
    bool instant_bullet;

    bool unlock_all_doors;
    bool slow_fall;
    bool run_damaged;
    bool fly;
    bool set_automatic;
    bool fast_firerate;
    bool day_time_modifier;

    int fly_key = 'B';
    int timescale_key = 'X';
    int unlock_all_doors_key = 'F';

    float fly_speed = 1.5f;
    float timescale_speed = 1.5f;
    float fire_rate = 100.f;
    float day_time = 12.f;

    void config_load(nlohmann::json& js, bool out);
}settings;
#endif