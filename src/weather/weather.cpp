#include "../../include/weather/weather.h"
#include "../../include/weather/fog.h"

void initWeather()
{
    initFog();
}

void updateWeather()
{
    updateFog();
}

void transferWeatherUniforms()
{
    transferFogUniforms();
}