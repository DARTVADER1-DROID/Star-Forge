#include "game.h"

typedef struct {
    const char *name;
    int planet_count;
    int moon_count;
    bool has_asteroids;
    float station_x;
    float station_y;
} SfSystemDefinition;

static const SfSystemDefinition systems[SF_MAX_SYSTEMS] = {
    { "Solace", 3, 5, true, 220.0f, 180.0f },
    { "Vesper", 2, 3, false, 360.0f, 260.0f },
    { "Kestrel", 4, 7, true, 180.0f, 420.0f },
    { "Ironwake", 2, 2, false, 480.0f, 340.0f }
};

const char *sf_game_system_name(int system_index)
{
    if (system_index < 0 || system_index >= SF_MAX_SYSTEMS) {
        return "Unknown";
    }

    return systems[system_index].name;
}

int sf_game_system_planet_count(int system_index)
{
    if (system_index < 0 || system_index >= SF_MAX_SYSTEMS) {
        return 0;
    }

    return systems[system_index].planet_count;
}

int sf_game_system_moon_count(int system_index)
{
    if (system_index < 0 || system_index >= SF_MAX_SYSTEMS) {
        return 0;
    }

    return systems[system_index].moon_count;
}

bool sf_game_system_has_asteroids(int system_index)
{
    if (system_index < 0 || system_index >= SF_MAX_SYSTEMS) {
        return false;
    }

    return systems[system_index].has_asteroids;
}



float sf_game_system_station_x(int system_index)
{
    if (system_index < 0 || system_index >= SF_MAX_SYSTEMS) {
        return 0.0f;
    }

    return systems[system_index].station_x;
}

float sf_game_system_station_y(int system_index)
{
    if (system_index < 0 || system_index >= SF_MAX_SYSTEMS) {
        return 0.0f;
    }

    return systems[system_index].station_y;
}
