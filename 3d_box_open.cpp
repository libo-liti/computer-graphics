#include <gl/glut.h>
#include <gl/gl.h>
#include <stdio.h>
#include <math.h>

#define	PI	3.1415926
float l = 0;
float a = 0;

GLfloat		vertices[][3] = {
		{ -1.0, -1.0,  1.0 },		// 0 
		{ -1.0,  1.0,  1.0 },		// 1
		{ 1.0,  1.0,  1.0 },		// 2
		{ 1.0, -1.0,  1.0 },		// 3
		{ -1.0, -1.0, -1.0 },		// 4
		{ -1.0,  1.0, -1.0 },		// 5
		{ 1.0,  1.0, -1.0 },		// 6
		{ 1.0, -1.0, -1.0 } };		// 7

GLfloat		colors[][3] = {
		{ 1.0, 0.0, 0.0 },			// red
		{ 0.0, 1.0, 0.0 },			// green 
		{ 1.0, 1.0, 0.0 },			// yellow
		{ 1.0, 1.0, 1.0 },			// white
		{ 0.0, 0.0, 1.0 },			// blue
		{ 1.0, 0.0, 1.0 } };		// magenta


void polygon(int a, int b, int c, int d) {
	glColor3fv(colors[a]);
	glBegin(GL_POLYGON);
	glVertex3fv(vertices[a]);
	glVertex3fv(vertices[b]);
	glVertex3fv(vertices[c]);
	glVertex3fv(vertices[d]);
	glEnd();
}

void red(int a, int b, int c, int d)
{
	glColor3fv(colors[a]);
	glBegin(GL_POLYGON);
	glVertex3f(vertices[a][0], vertices[a][1], vertices[a][2] + l);
	glVertex3f(vertices[b][0], vertices[b][1], vertices[b][2] + l);
	glVertex3f(vertices[c][0], vertices[c][1], vertices[c][2] + l);
	glVertex3f(vertices[d][0], vertices[d][1], vertices[d][2] + l);
	glEnd();
}

void cube(void) {
	// 흰색
	glPushMatrix();
	glTranslatef(0.0, -1.0, -1.0);
	glRotatef(a, 1.0, 0.0, 0.0);
	glTranslatef(0.0, 1.0, 1.0);
	polygon(3, 0, 4, 7);
	glPopMatrix();
	// 마젠타
	glPushMatrix();
	glTranslatef(-1.0, 0.0, -1.0);
	glRotatef(-a, 0.0, 1.0, 0.0);
	glTranslatef(1.0, 0.0, 1.0);
	polygon(5, 4, 0, 1);
	glPopMatrix();
	// 파란색
	polygon(4, 5, 6, 7);
	// 노란색
	glPushMatrix();
	glTranslatef(1.0, 0.0, -1.0);
	glRotatef(a, 0.0, 1.0, 0.0);
	glTranslatef(-1.0, 0.0, 1.0);
	polygon(2, 3, 7, 6);
	glPopMatrix();
	// 초록색
	glPushMatrix();
	glTranslatef(0.0, 1.0, -1.0);
	glRotatef(-a, 1.0, 0.0, 0.0);
	glTranslatef(0.0, -1.0, 1.0);
	polygon(1, 2, 6, 5);
	glPopMatrix();
	//빨간색
	red(0, 3, 2, 1);
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0, 1.0, 20.0);
}

void axis(void) {

	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0); // x축 
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(10.0, 0.0, 0.0);

	glColor3f(0.0, 1.0, 0.0); // y축 
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 10.0, 0.0);

	glColor3f(0.0, 0.0, 1.0); // z축 
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 10.0);
	glEnd();
}

void frame_reset(void) {
	glClearColor(0.6, 0.6, 0.6, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void camera(void) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(3.0, 4.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
}

void display(void)
{
	frame_reset();

	camera();
	axis();
	cube();
	if(l <= 2.0) l += 0.01;
	if(a <= 90) a += 0.1;
	
	glFlush();
	glutSwapBuffers();
}

void main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(500, 500);
	glutCreateWindow("cube");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(display);
	glutMainLoop();
}
