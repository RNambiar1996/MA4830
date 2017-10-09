#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "printTrajectory.h"

DuoValues XYscalling(float yMax, float dMax){
	DuoValues vals;
	vals.deltaY = yMax/50;
	vals.deltaX = dMax/150;

	if (vals.deltaY < 2*vals.deltaX)
		vals.deltaY = 2*vals.deltaX;

	else
		vals.deltaX = vals.deltaY/2;
	
	return vals;
}

float Downfunc(float y){

	float sqrtEq = 1 +  ((2*G_ACC*(h -y))/((pow(sin(theta),2)*pow(v,2))));
	float d = (pow(v,2)/(2*G_ACC))  *  (1 +  sqrt(sqrtEq) )  *  sin(2*theta);
	return d;

}

float Upfunc(float y){

	float sqrtEq = 1 +  ((2*G_ACC*(h - y))/((pow(sin(theta),2)*pow(v,2))));
	float d = (pow(v,2)/(2*G_ACC))  *  (1 -  sqrt(sqrtEq) )  *  sin(2*theta);
	return d;

}

//print spaces
int printSpaces(int num){

	char spaces[200];
	// char * spaces;    
	//   // allocate memory for the new string.
	//   spaces = malloc(num+1);
	//   // check that the allocation was successful
	//   if (spaces==NULL)
	//     return NULL;
	int i;
	for (i=0; i<num; i++)
		spaces[i] = ' ';

	spaces[num] = '\0';

	printf("%s",spaces);
	// printf("x");
	
	return 0;

}

float PlotAboveH(float y, float yMax, float deltaX, float deltaY, int Yindent){

	char str[200], yStr[20];
	int idx;

	while(y >= h && y<= yMax){
		int upX = ceil(Upfunc(y-0.0001)/deltaX);
		int downX = ceil(Downfunc(y-0.0001)/deltaX);

		// printf("%d,%d\n",upX, downX);
		//front unit 
		sprintf(yStr, "%d", (int)y);
		printf("%.1f",y);
		printSpaces(Yindent - (unsigned)strlen(yStr));
		// printf("|");
		for (idx = 0; idx<180; idx++){

			if (idx == upX || idx == downX)
				str[idx] = 'x';
			else
				str[idx] = ' ';

		}
		str[180] = '\0';    
		printf("|%s\n",str);
		y = y - deltaY;
	}

	return y;

}

float PlotBelowH(float y, float deltaX, float deltaY, int Yindent){

	char str[200], yStr[20];
	int idx;
	while(y >= 0 && y<= h){

		int downX = ceil(Downfunc(y)/deltaX);

		// printf("%d,%d\n",upX, downX);
		//front unit 
		sprintf(yStr, "%d", (int)y);
		printf("%.1f",y);
		printSpaces(Yindent - (unsigned)strlen(yStr));
		// printf("|");

		for (idx = 0; idx<180; idx++){

			if (idx ==  downX)
				str[idx] = 'x';
			else
				str[idx] = ' ';

		}

		str[180] = '\0';    
		printf("|%s\n",str);
		// printf("this indent %d, %d ", Yindent, (unsigned)strlen(yStr));
		y = y - deltaY;

	}
	return y;

}

int PlotXaxis(int Yindent, float deltaX, float dMax){
	//print "_"
	char str[200], xDigit[20];
	int i, idx=1, interval = 10;

	for (i=0; i<180; i++)
		str[i] = '_';

	str[180] = '\0';
	printf("%s\n",str);

	// strcpy( str, "(0,0)");
	printf("(0,0)");
	printSpaces(Yindent-2);

	while(idx<180-10){
		// for (idx = 1; idx < 180; idx++){
		if (idx%interval == 0){
			sprintf(xDigit, "%d", (int)(idx*deltaX));
			printf("%.3f",idx*deltaX);
			idx+= (unsigned)strlen(xDigit) + 3;
		}
		else{
			printf(" ");
			idx++;
		}
	}
	printf("\n");

	return 0;
}

int compute_trajectory(float v_input, float h_input, float theta_input) {

	char str[20] = "hello there";

	float dMax, yMax;
	DuoValues deltaXY;
	float deltaX;
	float deltaY; 
	int Yindent;
	float y;	
	//predefine num is 180
	
	v = v_input;
	h = h_input;
	theta = theta_input;
	
	theta = theta*PI/180;

	printf("\tTRAJECTORY MOTION OF: v = %.5f, h = %.5f, theta = %.5f\n\n",v, h, theta);

	dMax = Downfunc(0);
	yMax = h + pow(v,2)*pow(sin(theta),2)/(2*G_ACC);
	// printf("This is dMax num>> %f\n", dMax);
	printf("This is yMax num>> %f\n", yMax);


	//convert int to str of initial y indentation
	sprintf(str, "%d", (int)yMax);
	Yindent = (unsigned)strlen(str);
	// printf("Yindent> %d \n", Yindent);

	//auto scalling
	deltaXY = XYscalling(yMax, dMax); 
	

	//plotting
	y = yMax;
	y = PlotAboveH(y, yMax, deltaX, deltaY, Yindent);
	y = PlotBelowH(y, deltaX, deltaY, Yindent);
	PlotXaxis(Yindent, deltaX, dMax);

	printf("This is dMax num>> %f\n", dMax);

	return 0;
    
}
