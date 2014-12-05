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

float camera_x = 0.0;
float camera_y = 0.0;
float camera_z = 0.0;
float camera_zoom = 60.0;

// g++ -o maze maze.cpp -lglut -lGL -lGLU

void draw_wall(){
	glColor3f(0.0, 0.0, 0.5);
	glBegin( GL_QUADS );
		//FRONTFACE
		glVertex3f( 0.0, 0.0, 0.5);
		glVertex3f( 10.0, 0.0, 0.5);
		glVertex3f( 10.0, 10.0, 0.5);
		glVertex3f( 0.0, 10.0, 0.5);
		//BACKFACE
		glVertex3f( 0.0, 0.0, -0.5);
		glVertex3f( 0.0, 10.0, -0.5);
		glVertex3f( 10.0, 10.0, -0.5);
		glVertex3f( 10.0, 0.0, -0.5);

		glColor3f(0.0, 0.0, 1.0);
		//TOP FACE
		glVertex3f( 0.0, 10.0, -0.5);
		glVertex3f( 10.0, 10.0, -0.5);
		glVertex3f( 10.0, 10.0, 0.5);
		glVertex3f( 0.0, 10.0, 0.5);
		//LEFT FACE
		glVertex3f( 0.0, 0.0, 0.5);
		glVertex3f( 0.0, 10.0, 0.5);
		glVertex3f( 0.0, 10.0, -0.5);
		glVertex3f( 0.0, 0.0, -0.5);
		//RIGHT FACE
		glVertex3f( 10.0, 0.0, -0.5);
		glVertex3f( 10.0, 10.0, -0.5);
		glVertex3f( 10.0, 10.0, 0.5);
		glVertex3f( 10.0, 0.0, 0.5);
	glEnd();
}

void draw_maze() {
	int i;

	// Drawing Maze Floor 
	glBegin(GL_QUADS);
		glColor3f(0.0, 1.0, 0.0);
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

	// Set Callback Functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard2);

	set_projection();

	glutMainLoop();
	return 0;
}