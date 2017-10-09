#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define PI 3.14159265

#define G_ACC 9.81 // gravitational acceleration
float v, h, theta;

typedef struct DuoValues{
    float deltaX;
    float deltaY;
}DuoValues;

DuoValues XYscalling(float yMax, float dMax);

float Downfunc(float y);

float Upfunc(float y);

int printSpaces(int num);

float PlotAboveH(float y, float yMax, float deltaX, float deltaY, int Yindent);

float PlotBelowH(float y, float deltaX, float deltaY, int Yindent);

int PlotXaxis(int Yindent, float deltaX, float dMax);

int compute_trajectory(float v, float h, float theta); 
