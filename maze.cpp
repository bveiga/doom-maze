#include "maze.h"

using namespace std;

int win_width = 800;
int win_height = 800;

unsigned int texture[2];

float person_x = 0.0;
float person_y = -2.0;
float person_z = -30.0;
float person_r = 0.0;
int mode = PLAYER_MODE;

float eyex = 5.0;
float eyez = -1.0;

float modifyx = 0.0;
float modifyz = 0.0;
int direction = UP;

// g++ -o maze maze.cpp -lglut -lGL -lGLU

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
	glClearColor( 0.5, 0.7, 1.0, 0.0 );
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
	gluLookAt(eyex + modifyx, 7.5, eyez+modifyz,
		5.0 + modifyx, 5.3, -5.0 + modifyz,
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
			person_z += 2;
			break;
		case 'w':
			person_z -= 2;
			break;
		case 'a':
			camera.incrementViewDirection();
			break;
		case 'd':
			camera.decrementViewDirection();
			break;
		case 'p':
			if(mode == CAMERA_MODE) {
				mode = PLAYER_MODE;
			} else {
				mode = CAMERA_MODE;
			}
			break;
		default:
			break;
	}

	glutPostRedisplay();
}

void keyboard2(int key, int x, int y) {
	if(mode == PLAYER_MODE) {
		switch(key) {
			case GLUT_KEY_LEFT:
				camera.turnLeft();
				direction = manVectorIterator->turnLeft();
				if(direction == LEFT){
					eyex = 9.0;
					eyez = -5.0;
				}
				else if(direction == DOWN){
					eyex = 5.0;
					eyez = -9.0;
				}
				else if(direction == RIGHT){
					eyex = 1.0;
					eyez = -5.0;
				}
				else{
					eyex = 5.0;
					eyez = -1.0;
				}
				break;
			case GLUT_KEY_RIGHT:
				camera.turnRight();
				direction = manVectorIterator->turnRight();
				if(direction == RIGHT){
					eyex = 1.0;
					eyez = -5.0;
				}
				else if(direction == DOWN){
					eyex = 5.0;
					eyez = -9.0;
				}
				else if(direction == LEFT){
					eyex = 9.0;
					eyez = -5.0;
				}
				else{
					eyex = 5.0;
					eyez = -1.0;
				}
				break;
			case GLUT_KEY_UP:
				direction = manVectorIterator->incrementMove();
				if(direction == LEFT)
					modifyx -= SPEED;
				else if(direction == DOWN)
					modifyz += SPEED;
				else if(direction == RIGHT)
					modifyx += SPEED;
				else
					modifyz -= SPEED;
				break;
			case GLUT_KEY_DOWN:
				direction = manVectorIterator->decrementMove();
				if(direction == LEFT)
					modifyx += SPEED;
				else if(direction == DOWN)
					modifyz -= SPEED;
				else if(direction == RIGHT)
					modifyx -= SPEED;
				else
					modifyz += SPEED;
				break;
			default:
				break;
		}
	} else {
		switch(key) {
			case GLUT_KEY_LEFT:
				person_x -= 1;
				break;
			case GLUT_KEY_RIGHT:
				person_x += 1;
				break;
			case GLUT_KEY_UP:
				person_y += 1;
				break;
			case GLUT_KEY_DOWN:
				person_y -= 1;
				break;
			default:
				break;
		}
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