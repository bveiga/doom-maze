#include <stdio.h>
#include <cstdlib>
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

#define PI 3.14159265
using namespace std;

int win_width = 800;
int win_height = 800;

unsigned int texture[2];

float camera_x = 0.0;
float camera_y = 0.0;
float camera_z = 0.0;
float camera_zoom = 0.0;
float camera_r = 0.0;

// Man Globals.
static float highlightColor[3] = {0.0, 0.0, 0.0}; // Emphasize color.
static float lowlightColor[3] = {0.7, 0.7, 0.7}; // De-emphasize color.
static float partSelectColor[3] = {1.0, 0.0, 0.0}; // Selection indicate color.
static long font = (long)GLUT_BITMAP_8_BY_13; // Font selection.
static int animateMode = 0; // In animation mode?
static int animationPeriod = 1000; // Time interval between frames.
static ofstream outFile; // File to write configurations data.

// g++ -o maze maze.cpp -lglut -lGL -lGLU
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

   void incrementUpMove() { upMove += 0.1; }
   void decrementUpMove() { upMove -= 0.1; }
   void setUpMove(float move) { upMove = move; }

   void incrementForwardMove() { forwardMove += 0.1; }
   void decrementForwardMove() { forwardMove -= 0.1; }
   void setForwardMove(float move) { forwardMove = move; }

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

   float upMove, forwardMove; // Up and forward translation components - both lie
                                // on the plane parallel to which parts rotate -
                                // therefore all translations and part rotations
                                // are along one fixed plane.
        
   int selectedPart; // Selected part number - this part can be interactively rotated
                       // in the develop mode.

   int highlight; // If man is currently selected.
};

// Global vector of man configurations.
vector<Man> manVector;

// Global iterators to traverse manVector.
vector<Man>::iterator manVectorIterator;
vector<Man>::iterator manVectorAnimationIterator;

/*******************************************************************************
 *  Loading Textures (with error checking)
 *******************************************************************************/
void fatal(const char* format , ...)
{
   va_list args;
   va_start(args,format);
   vfprintf(stderr,format,args);
   va_end(args);
   exit(1);
}
unsigned int LoadTexBMP(char* file)
{
   unsigned int   texture;     // Texture name
   FILE*          f;           // File pointer
   char           magic[2];    // Image magic
   unsigned int   dx,dy,size;  // Image dimensions
   unsigned short nbp,bpp;     // Planes and bits per pixel
   unsigned char* image;       // Image data
   int            k;           // Counter

   //  Open file
   f = fopen(file,"rb");
   if (!f) fatal("Cannot open file %s\n",file);
   //  Check image magic
   if (fread(magic,2,1,f)!=1) fatal("Cannot read magic from %s\n",file);
   if (strncmp(magic,"BM",2)) fatal("Image magic not BMP in %s\n",file);
   //  Seek to and read header
   fseek(f,16,SEEK_CUR);
   if (fread(&dx ,4,1,f)!=1 || fread(&dy ,4,1,f)!=1 || fread(&nbp,2,1,f)!=1 || fread(&bpp,2,1,f)!=1)
     fatal("Cannot read header from %s\n",file);
   //  Check image parameters
   if (nbp!=1) fatal("%s bit planes is not 1: %d\n",file,nbp);
   if (bpp!=24) fatal("%s bits per pixel is not 24: %d\n",file,bpp);
#ifndef GL_VERSION_2_0
   //  OpenGL 2.0 lifts the restriction that texture size must be a power of two
   for (k=1;k<dx;k++);
   if (k!=dx) fatal("%s image width not a power of two: %d\n",file,dx);
   for (k=1;k<dy;k++);
   if (k!=dy) fatal("%s image height not a power of two: %d\n",file,dy);
#endif

   //  Allocate image memory
   size = 3*dx*dy;
   image = (unsigned char*) malloc(size);
   if (!image) fatal("Cannot allocate %d bytes of memory for image %s\n",size,file);
   //  Seek to and read image
   fseek(f,24,SEEK_CUR);
   if (fread(image,size,1,f)!=1) fatal("Error reading data from image %s\n",file);
   fclose(f);
   //  Reverse colors (BGR -> RGB)
   for (k=0;k<size;k+=3)
   {
      unsigned char temp = image[k];
      image[k]   = image[k+2];
      image[k+2] = temp;
   }

   //  Generate 2D texture
   glGenTextures(1,&texture);
   glBindTexture(GL_TEXTURE_2D,texture);
   //  Copy image
   glTexImage2D(GL_TEXTURE_2D,0,3,dx,dy,0,GL_RGB,GL_UNSIGNED_BYTE,image);
   if (glGetError()) fatal("Error in glTexImage2D %s %dx%d\n",file,dx,dy);
   //  Scale linearly when image size doesn't match
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

   //  Free image memory
   free(image);
   //  Return texture name
   return texture;
}

/*******************************************************************************
 *  DRAWING STUFF
 *******************************************************************************/
// Man constructor.
Man::Man()
{
   for (int i=0; i<9; i++) partAngles[i] = 0.0;
   upMove = 0.0;
   forwardMove = 0.0;
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
	glTranslatef(-5.0+forwardMove, 4.0, -5.0);
	glRotatef(-90.0, 0.0, 1.0, 0.0);

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

// Timer function.
void animate(int value)
{
   if (animateMode)
   {
      manVectorAnimationIterator++;
	  if (manVectorAnimationIterator == manVector.end() ) 
		  manVectorAnimationIterator = manVector.begin();
   }
   glutTimerFunc(animationPeriod, animate, 1);
   glutPostRedisplay();
}

static void Init() {
	glClearColor( 0.50, 0.50, 0.50, 0.0 );
	glEnable(GL_DEPTH_TEST);

	// Initialize global manVector with single configuration.
	manVector.push_back(Man() );

	// Initialize global iterators for manVector.
	manVectorIterator = manVector.begin();
	manVectorAnimationIterator = manVector.begin();
}

void set_projection() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60.0, (GLfloat)win_width / (GLfloat)win_height, 1.00, 130.0);

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void set_viewpoint() {
	gluLookAt(5.0+camera_x, 7.5+camera_y, -1.0+camera_zoom,
		5.0+camera_x, 4.5+camera_y, -7.0+camera_zoom,
		0.0, 1.0, 0.0);
}

static void reshape(int width, int height) {
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	set_projection();
}

static void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	set_viewpoint();

	draw_maze();
	// Local iterator to traverse manVector.
	vector<Man>::iterator localManVectorIterator;

	localManVectorIterator = manVector.begin();
	while(localManVectorIterator != manVector.end() )
	{
		localManVectorIterator->draw();
		localManVectorIterator++;
	}

	glFlush();			      // Flush drawing routines
	glutSwapBuffers();		      // Swap buffers
}

void keyboard(unsigned char key, int x, int y) {
	switch(key) {
		case 's':
			camera_zoom += 2;
			break;
		case 'w':
			camera_zoom -= 2;
			break;
		case 'r':
			camera_r += 2;
			break;
		case 't':
			camera_r -= 2;
			break;
		default:
			break;
	}

	glutPostRedisplay();
}

void keyboard2(int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_LEFT:
			camera_x -= 1;
			break;
		case GLUT_KEY_RIGHT:
			camera_x += 1;
			break;
		case GLUT_KEY_UP:
			camera_y += 1;
			break;
		case GLUT_KEY_DOWN:
			camera_y -= 1;
			break;
		default:
			break;
	}

	glutPostRedisplay();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize( 800, 800);
	glutInitWindowPosition(100, 100);
	glutCreateWindow( "MAZE" );
	Init();

	string name = "textures/wall_texture800x800.bmp";
	texture[0] = LoadTexBMP(strdup(name.c_str()));

	// Set Callback Functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard2);

	set_projection();
	glutTimerFunc(5, animate, 1);

	glutMainLoop();
	return 0;
}