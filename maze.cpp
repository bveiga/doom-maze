#include "maze.h"

using namespace std;

int win_width = 800;
int win_height = 800;

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
	if(mode == PLAYER_MODE) {
		gluLookAt(eyex + modifyx, 7.5, eyez+modifyz,
			5.0 + modifyx, 5.3, -5.0 + modifyz,
			0.0, 1.0, 0.0);
	} else if(mode == BIRD_MODE) {
		gluLookAt(45.0, 50.0, -45.0,
			5.0 + modifyx, 5.3, -5.0 + modifyz,
			0.0, 1.0, 0.0);
	} else if(mode == TOP_DOWN_MODE) {
		gluLookAt(5.0+modifyx, 50.0, -1.0+modifyz,
			5.0 + modifyx, 5.3, -5.0 + modifyz,
			0.0, 1.0, 0.0);
	}
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

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);



	// Light Position
	glPushMatrix();
		glRotatef(-90.0, 0.0f, 1.0f, 0.0f);
		GLfloat lightpos[] = {45.0f, 40.0f, -45.0f, 0.0f};
		// GLfloat lightspot[] = {0.0, 1.0, 0.0};
		glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
		// glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightspot);
	glPopMatrix();

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
		case 'p':
			mode = PLAYER_MODE;
			break;
		case 'b':
			mode = BIRD_MODE;
			break;
		case 't':
			mode = TOP_DOWN_MODE;
			break;
		default:
			break;
	}

	glutPostRedisplay();
}

void keyboard2(int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_LEFT:
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

	glutPostRedisplay();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize( 800, 800);
	glutInitWindowPosition(100, 100);
	glutCreateWindow( "MAZE" );
	Init();

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