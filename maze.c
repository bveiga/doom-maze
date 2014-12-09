#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef __APPLE__  // include Mac OS X verions of headers
#  include <OpenGL/OpenGL.h>
#  include <GLUT/glut.h>
#else // non-Mac OS X operating systems
#  include <GL/glew.h>
#  include <GL/freeglut.h>
#  include <GL/freeglut_ext.h>
#endif  // __APPLE__

int win_width = 800;
int win_height = 800;

unsigned int texture[2];

float camera_x = 0.0;
float camera_y = 0.0;
float camera_z = 0.0;
float camera_zoom = 60.0;

// g++ -o maze maze.cpp -lglut -lGL -lGLU

/*
 *  Print message to stderr and exit
 */
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
static void Init() {
	glClearColor( 0.50, 0.50, 0.50, 0.0 );
	glEnable(GL_DEPTH_TEST);
	
}

void set_projection() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(camera_zoom, (GLfloat)win_width / (GLfloat)win_height, 1.00, 100.0);

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void set_viewpoint() {
	gluLookAt(0.0+camera_x, 2.5+camera_y, 5.0+camera_z,
		0.0+camera_x, -0.5+camera_y, -1.0+camera_z,
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

	glFlush();			      // Flush drawing routines
	glutSwapBuffers();		      // Swap buffers
}

void keyboard(unsigned char key, int x, int y) {
	switch(key) {
		case 's':
			camera_z += 2;
			break;
		case 'w':
			camera_z -= 2;
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
	texture[0] = LoadTexBMP("textures/wall_texture800x800.bmp");

	// Set Callback Functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard2);

	set_projection();

	glutMainLoop();
	return 0;
}