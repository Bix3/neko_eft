#include "settings.hpp"

void settings_struct::config_load(nlohmann::json& js, bool out)
{
    char    setting_name[4] = { 0 };
    int64_t setting_indexer = 0;

    /*Visuals*/
    setting(visuals);
    setting(crosshair);
    setting(player_esp);
    setting(scav_esp);
    setting(esp_players_box);
    setting(esp_players_show_name);
    setting(esp_players_show_distance);
    setting(esp_players_weapon);
    setting(esp_skeleton);

    setting(esp_scav_distance);
    setting(esp_players_distance);

    setting_imcolor(m_player_color);
    setting_imcolor(m_color_crosshair);

    /*Aimbot*/
    setting(aimbot);
    setting(aimbot_max_fov);
    setting(ricochet);
    setting(aimbot_aimspot);
    setting(aimbot_key);

    /*Misc*/
    //setting(no_fall_damage);
    setting(infinite_stamine);
    setting(set_automatic);
    setting(fast_firerate);
    setting(no_sway);
    setting(no_recoil);
    setting(no_spread);
    //setting(unlock_all_doors);
    //setting(unlock_all_doors_key);
    //setting(slow_fall);
    setting(run_damaged);
    setting(day_time_modifier);
    setting(instant_bullet);

    //setting(timescale);
    //setting(timescale_key);
    //setting(timescale_speed);

    //setting(fly);
    //setting(fly_key);
    //setting(fly_speed);

    setting(day_time);
}