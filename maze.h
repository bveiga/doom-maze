#ifndef maze_h
#define maze_h

#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#ifdef __APPLE__  // include Mac OS X verions of headers
#  include <OpenGL/OpenGL.h>
#  include <GLUT/glut.h>
#else // non-Mac OS X operating systems
#  include <GL/glew.h>
#  include <GL/freeglut.h>
#  include <GL/freeglut_ext.h>
#endif  // __APPLE__

using namespace std;

#define PI 3.14159265
#define CAMERA_MODE 1
#define PLAYER_MODE 2

#define UP 3
#define RIGHT 4
#define DOWN 5
#define LEFT 6
#define SPEED 0.2

// Man Globals.
static float highlightColor[3] = {0.0, 0.0, 0.0}; // Emphasize color.
static float lowlightColor[3] = {0.7, 0.7, 0.7}; // De-emphasize color.
static float partSelectColor[3] = {1.0, 0.0, 0.0}; // Selection indicate color.
static long font = (long)GLUT_BITMAP_8_BY_13; // Font selection.
static int animateMode = 0; // In animation mode?
static int animationPeriod = 1000; // Time interval between frames.
static ofstream outFile; // File to write configurations data.


/*******************************************************************
 *	CAMERA STUFF
********************************************************************/
class Camera
{
public:
   Camera();
   void incrementViewDirection();
   void decrementViewDirection();
   void incrementZoomDistance() { zoomDistance += 1.0; }
   void decrementZoomDistance() { zoomDistance -= 1.0; }

   float getViewDirection() const { return viewDirection; }
   float getZoomDistance() const { return zoomDistance; }

   float getX() { return xValue; }
   float getZ() { return zValue; }

   void turnRight() { 
		if(direction == UP) {
			xValue = -4.0;
			zValue = -4.0;
			direction = RIGHT;
		} else if (direction == RIGHT) {
			xValue = 4.0;
			zValue = -4.0;
			direction = DOWN;
		} else if (direction == DOWN) {
			xValue = 4.0; 
			zValue = 4.0;
			direction = LEFT;
		} else {
			xValue = -4.0;
			zValue = 4.0;
			direction = UP;
		}
	}

	void turnLeft() {
		if(direction == UP) {
			xValue = 4.0; 
			zValue = -4.0;
			direction = LEFT;
		} else if (direction == RIGHT) {
			xValue = 4.0; 
			zValue = 4.0;
			direction = UP;
		} else if (direction == DOWN) {
			xValue = -4.0; 
			zValue = 4.0;
			direction = RIGHT;
		} else {
			xValue = -4.0; 
			zValue = -4.0;
			direction = DOWN;
		}
	}

private:
   float viewDirection;
   float zoomDistance;
   float direction;
   float xValue;
   float zValue;
};

// Global camera.
Camera camera;

// Camera constructor.
Camera::Camera()
{
   viewDirection = 0.0;
   zoomDistance = 10.0;
}

// Function to increment camera viewing angle.
void Camera::incrementViewDirection()
{
   viewDirection += 5.0;
   if (viewDirection > 360.0) viewDirection -= 360.0;
}

// Function to decrement camera viewing angle.
void Camera::decrementViewDirection()
{
   viewDirection -= 5.0;
   if (viewDirection < 0.0) viewDirection += 360.0;
}

/*******************************************************************
 *	MAN STUFF
********************************************************************/
class Man
{
public:
	Man();
	void incrementSelectedPart();

	void incrementPartAngle();
	void decrementPartAngle();
	void setPartAngle(float angle) { partAngles[selectedPart] = angle; }

	void incrementForwardMove() { forwardMove -= SPEED; }
	void decrementForwardMove() { forwardMove += SPEED; }
	void incrementSideMove() { sideMove -= SPEED; }
	void decrementSideMove() { sideMove += SPEED; }
	void setForwardMove(float move) { forwardMove = move; }

	int turnRight() { 
		rotateAngle -= 90.0;
		if(direction == UP) {
			direction = RIGHT;
		} else if (direction == RIGHT) {
			direction = DOWN;
		} else if (direction == DOWN) {
			direction = LEFT;
		} else {
			direction = UP;
		}

		return direction;
	}
	int turnLeft() {
		rotateAngle += 90.0;
		if(direction == UP) {
			direction = LEFT;
		} else if (direction == RIGHT) {
			direction = UP;
		} else if (direction == DOWN) {
			direction = RIGHT;
		} else {
			direction = DOWN;
		}

		return direction;
	}

	int incrementMove(){
		if (direction == UP)
			incrementForwardMove();
		else if(direction == RIGHT)
			incrementSideMove();
		else if(direction == DOWN)
			decrementForwardMove();
		else
			decrementSideMove();
		return direction;
	}
	int decrementMove(){
		if (direction == UP)
			decrementForwardMove();
		else if(direction == RIGHT)
			decrementSideMove();
		else if(direction == DOWN)
			incrementForwardMove();
		else
			incrementSideMove();
		return direction;
	}
	void setHighlight(int inputHighlight) { highlight = inputHighlight; }

	void draw();
	void outputData();
	void writeData();

private:
   // Man configuration values.
   float partAngles[9]; // Angles from 0 to 360 of 9 body parts - torso, left and right
                          // upper arms, left and right lower arms, left and right upper
                          // legs, left and right lower legs. 
                          // All parts move parallel to the same plane.

   float upMove, forwardMove, sideMove; // Up and forward translation components - both lie
                                // on the plane parallel to which parts rotate -
                                // therefore all translations and part rotations
                                // are along one fixed plane.

   float rotateAngle;

   int direction;
        
   int selectedPart; // Selected part number - this part can be interactively rotated
                       // in the develop mode.

   int highlight; // If man is currently selected.
};

// Man constructor.
Man::Man()
{
   for (int i=0; i<9; i++) partAngles[i] = 0.0;
   upMove = 0.0;
   forwardMove = 0.0;
   sideMove = 0.0;
   rotateAngle = 0.0;
   direction = UP;
   selectedPart = 0;
   highlight = 1;
}

// Function to incremented selected part..
void Man::incrementSelectedPart()
{
   if (selectedPart < 8) selectedPart++;
   else selectedPart = 0;
}

// Function to increment angle of selected part.
void Man::incrementPartAngle()
{
   partAngles[selectedPart] += 5.0;
   if (partAngles[selectedPart] > 360.0) partAngles[selectedPart] -= 360.0;
}

// Function to decrement angle of selected part.
void Man::decrementPartAngle()
{
   partAngles[selectedPart] -= 5.0;
   if (partAngles[selectedPart] < 0.0) partAngles[selectedPart] += 360.0; 
}

// Function to draw man.
void Man::draw()
{
	glPushMatrix(); 

	// Up and forward translations.
	glTranslatef(-5.0+forwardMove, 4.0, -5.0+sideMove);
	glRotatef(-90.0+rotateAngle, 0.0, 1.0, 0.0);

	// Torso begin.
	glRotatef(partAngles[0], 1.0, 0.0, 0.0);
	glPushMatrix();
		glColor3f(0.1, 0.1, 0.1);
		glScalef(1.0, 2.0, 1.0);
		glutSolidCube(1.0);
	glPopMatrix();
	if (highlight && !animateMode) glColor3fv(highlightColor);
	// Torso end.

	// Head begin.
	glPushMatrix();

	glTranslatef(0.0, 2.0, 0.0);
	glPushMatrix();
		glColor3f(0.2, 0.2, 0.2);
		glScalef(0.7, 1.0, 0.7);
		glutSolidSphere(1.0, 10, 8);
	glPopMatrix();

	glPopMatrix();
	// Head end.

	// Left upper and lower arm begin.
	glPushMatrix();
		// Left upper arm begin.
		glTranslatef(0.7, 1.0, 0.0);
		glRotatef(180.0 + partAngles[1], 1.0, 0.0, 0.0);
		glTranslatef(0.0, 0.5, 0.0);
		glPushMatrix();
			glColor3f(0.2, 0.2, 0.2);
			glScalef(0.3, 0.8, 0.3);
			glutSolidCube(1.0);
		glPopMatrix(); 

		// Left lower arm begin.
		glTranslatef(0.0, 0.4, 0.0);
		glRotatef(partAngles[2], 1.0, 0.0, 0.0);
		glTranslatef(0.0, 0.4, 0.0);
		glPushMatrix();
			glColor3f(1.0, 0.0, 0.0);
			glScalef(0.3, 0.8, 0.3);
			glutSolidCube(1.0);
		glPopMatrix();
	glPopMatrix();

	// Right upper and lower arm begin.
	glPushMatrix();
		// Right upper arm begin.
		glTranslatef(-0.7, 1.0, 0.0);
		glRotatef(180.0 + partAngles[3], 1.0, 0.0, 0.0);
		glTranslatef(0.0, 0.5, 0.0);
		glPushMatrix();
			glColor3f(0.2, 0.2, 0.2);
			glScalef(0.3, 0.8, 0.3);
			glutSolidCube(1.0);
		glPopMatrix();

		// Right lower arm begin.
		glTranslatef(0.0, 0.4, 0.0);
		glRotatef(partAngles[4], 1.0, 0.0, 0.0);
		glTranslatef(0.0, 0.4, 0.0);
		glPushMatrix();
			glColor3f(1.0, 0.0, 0.0);
			glScalef(0.3, 0.8, 0.3);
			glutSolidCube(1.0);
		glPopMatrix();
	glPopMatrix();
	// Right upper and lower arm end.

	// Left upper and lower leg with foot begin.
	glPushMatrix();
		// Left upper leg begin.
		glTranslatef(0.3, -1.0, 0.0);
		glRotatef(partAngles[5], 1.0, 0.0, 0.0);
		glTranslatef(0.0, -0.75, 0.0);
		glPushMatrix(); 
			glColor3f(1.0, 0.0, 0.0);
			glScalef(0.3, 1.5, 0.3);
			glutSolidCube(1.0);
		glPopMatrix();

		// Left lower leg with foot begin.
		glTranslatef(0.0, -0.75, 0.0);
		glRotatef(partAngles[6], 1.0, 0.0, 0.0);
		glTranslatef(0.0, -0.75, 0.0);

		// Lower leg.
		glPushMatrix();
			glColor3f(0.2, 0.2, 0.2);
			glScalef(0.3, 1.5, 0.3);
			glutSolidCube(1.0);
		glPopMatrix();

		// Foot.
		glTranslatef(0.0, -0.8, 0.3);
		glPushMatrix();
			glScalef(0.3, 0.25, 1.0);
			glutSolidCube(1.0);
		glPopMatrix();
	glPopMatrix();
	// Left upper and lower leg with foot end.

	// Right upper and lower leg with foot begin.
	glPushMatrix();
		// Right upper leg begin.
		glTranslatef(-0.3, -1.0, 0.0);
		glRotatef(partAngles[7], 1.0, 0.0, 0.0);
		glTranslatef(0.0, -0.75, 0.0);
		glPushMatrix();
			glColor3f(1.0, 0.0, 0.0);
			glScalef(0.3, 1.5, 0.3);
			glutSolidCube(1.0);
		glPopMatrix();

		// Right lower leg with foot begin.
		glTranslatef(0.0, -0.75, 0.0);
		glRotatef(partAngles[8], 1.0, 0.0, 0.0);
		glTranslatef(0.0, -0.75, 0.0);

		// Lower leg.
		glPushMatrix();
			glColor3f(0.2, 0.2, 0.2);
			glScalef(0.3, 1.5, 0.3);
			glutSolidCube(1.0);
		glPopMatrix();

		// Foot.
		glTranslatef(0.0, -0.8, 0.3);
		glPushMatrix();
			glScalef(0.3, 0.25, 1.0);
			glutSolidCube(1.0);
		glPopMatrix();
	glPopMatrix();
	// Right upper and lower leg with foot end.

	glPopMatrix(); 
}

// Function to output configurations data to file.
void Man::outputData()
{
	int i;
	for (i = 0; i < 9; i++) outFile << partAngles[i] << " ";
	outFile << upMove << " " << forwardMove << endl;
}

// Routine to draw a bitmap character string.
void writeBitmapString(void *font, char *string)
{  
   char *c;

   for (c = string; *c != '\0'; c++) glutBitmapCharacter(font, *c);
}  

// Routine to convert floating point to char string.
void floatToString(char * destStr, int precision, float val) 
{
	sprintf(destStr,"%f",val);
	destStr[precision] = '\0';
}

// Routine to write configurations data.
void Man::writeData()
{
   char buffer [33];
   
   floatToString(buffer, 4, partAngles[0]);	
   glRasterPos3f(-28.0, 10.0, 0.0);
   writeBitmapString((void*)font, "torso = "); 
   glRasterPos3f(-11.0, 10.0, 0.0);
   writeBitmapString((void*)font, buffer);

   floatToString(buffer, 4, partAngles[1]);	
   glRasterPos3f(-28.0, 8.0, 0.0);
   writeBitmapString((void*)font, "left upper arm = "); 
   glRasterPos3f(-11.0, 8.0, 0.0);
   writeBitmapString((void*)font, buffer);

   floatToString(buffer, 4, partAngles[2]);	
   glRasterPos3f(-28.0, 6.0, 0.0);
   writeBitmapString((void*)font, "left lower arm = "); 
   glRasterPos3f(-11.0, 6.0, 0.0);
   writeBitmapString((void*)font, buffer);

   floatToString(buffer, 4, partAngles[3]);	
   glRasterPos3f(-28.0, 4.0, 0.0);
   writeBitmapString((void*)font, "right upper arm = "); 
   glRasterPos3f(-11.0, 4.0, 0.0);
   writeBitmapString((void*)font, buffer);

   floatToString(buffer, 4, partAngles[4]);	
   glRasterPos3f(-28.0, 2.0, 0.0);
   writeBitmapString((void*)font, "right lower arm = "); 
   glRasterPos3f(-11.0, 2.0, 0.0);
   writeBitmapString((void*)font, buffer);

   floatToString(buffer, 4, partAngles[5]);	
   glRasterPos3f(-28.0, 0.0, 0.0);
   writeBitmapString((void*)font, "left uppper leg = "); 
   glRasterPos3f(-11.0, 0.0, 0.0);
   writeBitmapString((void*)font, buffer);

   floatToString(buffer, 4, partAngles[6]);	
   glRasterPos3f(-28.0, -2.0, 0.0);
   writeBitmapString((void*)font, "left lower leg = "); 
   glRasterPos3f(-11.0, -2.0, 0.0);
   writeBitmapString((void*)font, buffer);

   floatToString(buffer, 4, partAngles[7]);	
   glRasterPos3f(-28.0, -4.0, 0.0);
   writeBitmapString((void*)font, "right upper leg = "); 
   glRasterPos3f(-11.0, -4.0, 0.0);
   writeBitmapString((void*)font, buffer);

   floatToString(buffer, 4, partAngles[8]);	
   glRasterPos3f(-28.0, -6.0, 0.0);
   writeBitmapString((void*)font, "right lower leg = "); 
   glRasterPos3f(-11.0, -6.0, 0.0);
   writeBitmapString((void*)font, buffer);

   floatToString(buffer, 4, upMove);	
   glRasterPos3f(-28.0, -8.0, 0.0);
   writeBitmapString((void*)font, "upMove = "); 
   glRasterPos3f(-11.0, -8.0, 0.0);
   writeBitmapString((void*)font, buffer);

   floatToString(buffer, 4, forwardMove);	
   glRasterPos3f(-28.0, -10.0, 0.0);
   writeBitmapString((void*)font, "forwardMove = "); 
   glRasterPos3f(-11.0, -10.0, 0.0);
   writeBitmapString((void*)font, buffer);
}


/*******************************************************************************
 *  DRAWING STUFF
 *******************************************************************************/
void draw_wall(){
	glEnable(GL_TEXTURE_2D);
	glBegin( GL_QUADS );
		glColor3f(0.6, 0.0, 0.0);
		//FRONTFACE
		glTexCoord2f(0.0, 0.0); glVertex3f( 0.0, 0.0, 0.5);
		glTexCoord2f(1.0, 0.0); glVertex3f( 10.0, 0.0, 0.5);
		glTexCoord2f(1.0, 1.0); glVertex3f( 10.0, 10.0, 0.5);
		glTexCoord2f(0.0, 1.0); glVertex3f( 0.0, 10.0, 0.5);
		//BACKFACE
		glTexCoord2f(1.0, 0.0); glVertex3f( 0.0, 0.0, -0.5);
		glTexCoord2f(1.0, 1.0); glVertex3f( 0.0, 10.0, -0.5);
		glTexCoord2f(0.0, 1.0); glVertex3f( 10.0, 10.0, -0.5);
		glTexCoord2f(0.0, 0.0); glVertex3f( 10.0, 0.0, -0.5);

		glColor3f(0.3, 0.0, 0.0);
		//TOP FACE
		glTexCoord2f(0.0, 0.9); glVertex3f( 0.0, 10.0, 0.5);
		glTexCoord2f(1.0, 0.9); glVertex3f( 10.0, 10.0, 0.5);
		glTexCoord2f(1.0, 1.0); glVertex3f( 10.0, 10.0, -0.5);
		glTexCoord2f(0.0, 1.0); glVertex3f( 0.0, 10.0, -0.5);
		//LEFT FACE
		glTexCoord2f(0.1, 0.0); glVertex3f( 0.0, 0.0, 0.5);
		glTexCoord2f(0.1, 1.0); glVertex3f( 0.0, 10.0, 0.5);
		glTexCoord2f(0.0, 1.0); glVertex3f( 0.0, 10.0, -0.5);
		glTexCoord2f(0.0, 0.0); glVertex3f( 0.0, 0.0, -0.5);
		//RIGHT FACE
		glTexCoord2f(0.0, 0.0); glVertex3f( 10.0, 0.0, -0.5);
		glTexCoord2f(0.0, 1.0); glVertex3f( 10.0, 10.0, -0.5);
		glTexCoord2f(0.1, 1.0); glVertex3f( 10.0, 10.0, 0.5);
		glTexCoord2f(0.1, 0.0); glVertex3f( 10.0, 0.0, 0.5);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void draw_maze() {
	int i;
	glPushMatrix();
		glPushMatrix();
			glTranslatef(10.0, 0.0, 0.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(10.0, 0.0, -10.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(10.0, 0.0, -30.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(0.0, 0.0, -40.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(20.0, 0.0, -10.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(20.0, 0.0, -30.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(20.0, 0.0, -40.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(20.0, 0.0, -70.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(30.0, 0.0, -10.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(30.0, 0.0, -30.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(30.0, 0.0, -50.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(30.0, 0.0, -70.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(40.0, 0.0, 0.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(40.0, 0.0, -20.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(40.0, 0.0, -60.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(50.0, 0.0, -10.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(50.0, 0.0, -80.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(60.0, 0.0, -30.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(60.0, 0.0, -50.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(70.0, 0.0, -30.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(70.0, 0.0, -50.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(70.0, 0.0, -60.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(70.0, 0.0, -70.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(80.0, 0.0, -10.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(80.0, 0.0, -40.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(80.0, 0.0, -70.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();

		glPushMatrix();
			glTranslatef(80.0, 0.0, -80.0);
			glRotatef(90.0, 0.0, 1.0, 0.0);
			draw_wall();
		glPopMatrix();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0, 0.0, -30.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(0.0, 0.0, -70.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(10.0, 0.0, -20.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(10.0, 0.0, -60.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(10.0, 0.0, -70.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(10.0, 0.0, -80.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(20.0, 0.0, -60.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(20.0, 0.0, -30.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(30.0, 0.0, -20.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(30.0, 0.0, -30.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(30.0, 0.0, -60.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(30.0, 0.0, -80.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(40.0, 0.0, -20.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(40.0, 0.0, -70.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(40.0, 0.0, -80.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(50.0, 0.0, -20.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(50.0, 0.0, -30.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(50.0, 0.0, -60.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(50.0, 0.0, -70.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(50.0, 0.0, -10.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(60.0, 0.0, -10.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(60.0, 0.0, -20.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(60.0, 0.0, -80.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(70.0, 0.0, -20.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(70.0, 0.0, -30.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(70.0, 0.0, -50.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(70.0, 0.0, -60.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(80.0, 0.0, -30.0);
		draw_wall();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(80.0, 0.0, -60.0);
		draw_wall();
	glPopMatrix();

	// Drawing Maze Floor 
	glBegin(GL_QUADS);
		glColor3f(0.0, 0.4, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
	   	glVertex3f(90.0, 0.0, 0.0);
	   	glVertex3f(90.0, 0.0, -90.0);
	   	glVertex3f(0.0, 0.0, -90.0);
	glEnd();

	// Drawing Maze Walls
	for(i = 0; i < 9; ++i) {
		draw_wall();
		glTranslatef(10.0, 0.0, 0.0);
	}
	glRotatef(90.0, 0.0, 1.0, 0.0);
	for(i = 0; i < 9; ++i) {
		draw_wall();
		glTranslatef(10.0, 0.0, 0.0);
	}
	glRotatef(90.0, 0.0, 1.0, 0.0);
	for(i = 0; i < 9; ++i) {
		draw_wall();
		glTranslatef(10.0, 0.0, 0.0);
	}
	glRotatef(90.0, 0.0, 1.0, 0.0);
	for(i = 0; i < 9; ++i) {
		draw_wall();
		glTranslatef(10.0, 0.0, 0.0);
	}
}

#endif