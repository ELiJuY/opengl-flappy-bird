#pragma once

extern bool keyA;
extern bool keyD;

extern float currentTime;
extern float deltaTime;
extern float lastTime;
extern float nightFactor;
extern float simulatedHour;
extern float dayDuration;

bool isNight();
bool isDay();
bool isSunset();
bool isDawn();
float getNightFactor();
void updateGameTime();