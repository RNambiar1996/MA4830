#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "printTrajectory.h"

// change ScreenDisplayScale between 0 to 1 according to the resolution of the computer display 
int Xscale, Yscale, col;
float ScreenDisplayScale = 0.5;


//auto scalling of x y axis according to max display
//scale length of x and y axis is equal, helps in visulizing projectile motion
DuoValues XYscalling(float yMax, float dMax){
	DuoValues vals;
	vals.deltaY = yMax/Xscale;
	vals.deltaX = dMax/Yscale;

	if (vals.deltaY < 1*vals.deltaX)
		vals.deltaY = 1*vals.deltaX;
	else
		vals.deltaX = vals.deltaY/1;
	return vals;
}

//trajectory xy position while object moving upwards (vy >0)
float Downfunc(float y){
	float sqrtEq = 1 +  ((2*G_ACC*(h -y))/((pow(sin(theta),2)*pow(v,2))));
	float d = (pow(v,2)/(2*G_ACC))  *  (1 +  sqrt(sqrtEq) )  *  sin(2*theta);
	return d;

}

//trajectory xy position while object droping downwards (vy <0)
float Upfunc(float y){
	float sqrtEq = 1 +  ((2*G_ACC*(h - y))/((pow(sin(theta),2)*pow(v,2))));
	float d = (pow(v,2)/(2*G_ACC))  *  (1 -  sqrt(sqrtEq) )  *  sin(2*theta);
	return d;

}

//print empty spaces 
int printSpaces(int num){
	
	char spaces[200];
	int i;
	for (i=0; i<num; i++)
		spaces[i] = ' ';

	spaces[num] = '\0';
	printf("%s",spaces);
	
	return 0;
}

//plot the graph which y is above h, initial height of the object. plot from  y=h to y=yMax
float PlotAboveH(float y, float yMax, float deltaX, float deltaY, int Yindent){

	char str[col], yStr[20];
	int idx;

	while(y >= h && y<= yMax){
		int upX = ceil(Upfunc(y-0.001)/deltaX);
		int downX = ceil(Downfunc(y-0.001)/deltaX);

		//print front unit of y scale
		sprintf(yStr, "%d", (int)y);
		printf("%.1f",y);
		printSpaces(Yindent - (unsigned)strlen(yStr));

		//plot xy position of the point on graph
		for (idx = 0; idx<col; idx++){
			if (idx == upX || idx == downX)
				str[idx] = 'x';
			else
				str[idx] = ' ';
		}
		str[col] = '\0';    
		printf("|%s\n",str);
		y = y - deltaY;
	}
	return y;
}

//plot the graph which y is below h, initial height of the object. plot from  y=h to y=0
float PlotBelowH(float y, float deltaX, float deltaY, int Yindent){

	char str[200], yStr[20];
	int idx;
	while(y >= 0 && y<= h){

		int downX = ceil(Downfunc(y)/deltaX);

		//print front unit of y scale
		sprintf(yStr, "%d", (int)y);
		printf("%.1f",y);
		printSpaces(Yindent - (unsigned)strlen(yStr));

		//plot xy position of the point on graph
		for (idx = 0; idx<col; idx++){
			if (idx ==  downX)
				str[idx] = 'x';
			else
				str[idx] = ' ';
		}

		str[col] = '\0';    
		printf("|%s\n",str);
		y = y - deltaY;
	}
	return y;
}

//plot all scale on the x axis, consist of 2 lines of printing
int PlotXaxis(int Yindent, float deltaX, float dMax){
	
	char str[200], xDigit[20];
	int i, idx=1, interval = 10;

	//print "_" seperation line
	for (i=0; i<col; i++)
		str[i] = '_';
	str[col] = '\0';
	printf("%s\n",str);

	//initial indentation
	printf("(0,0)");
	printSpaces(Yindent-2);

	while(idx<col){ //-20){
		//print unit of x scale on x-axis
		if (idx%interval == 0){
			sprintf(xDigit, "%d", (int)(idx*deltaX));
			printf("%.3f",idx*deltaX);
			idx+= (unsigned)strlen(xDigit) + 3;
		}
		//spacing between each unit of x-scale
		else{
			printf(" ");
			idx++;
		}
	}
	printf("\n");
	return 0;
}

//main function of compute the trajectory
int compute_trajectory(float v_input, float h_input, float theta_input) {

	char str[20] = "hello there";

	float dMax, yMax;
	DuoValues deltaXY;
	float deltaX;
	float deltaY; 
	int Yindent;
	float y;	

	//display scalling according to global ScreenDisplayScale
	Xscale = 150*ScreenDisplayScale; //50;
	Yscale = 150*ScreenDisplayScale; //150;
	col = 160*ScreenDisplayScale; //180
	//printf("New Scale factor: %d %d %d\n", Xscale, Yscale, col);
	
	v = v_input;
	h = h_input;
	theta = theta_input;
	theta = theta*PI/180;

	printf("\tTRAJECTORY MOTION OF: v = %.5f m/s, h = %.5f m, theta = %.5f radians\n\n",v, h, theta);

	//get max x position and y position
	dMax = Downfunc(0);
	yMax = h + pow(v,2)*pow(sin(theta),2)/(2*G_ACC);
	
	printf("\n\n\nMaximum height to which the projectile rises : %f\n\n\n\n", yMax);


	//convert int to str of initial y indentation
	sprintf(str, "%d", (int)yMax);
	Yindent = (unsigned)strlen(str);
	// printf("Yindent> %d \n", Yindent);

	//auto scalling
	deltaXY = XYscalling(yMax, dMax); 

	deltaX = deltaXY.deltaX;
	deltaY = deltaXY.deltaY;

	//plotting graph
	y = yMax;
	y = PlotAboveH(y, yMax, deltaX, deltaY, Yindent);
	y = PlotBelowH(y, deltaX, deltaY, Yindent);
	PlotXaxis(Yindent, deltaX, dMax);

	printf("\n\n\nMaximum horizontal distance of the projectile : %f\n\n", dMax);

	return 0;
}
