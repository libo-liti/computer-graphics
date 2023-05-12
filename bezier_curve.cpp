#include <windows.h>
#include <math.h>
#include <gl/gl.h>
#include <gl/glut.h> // (or others, depending on the system in use)
#include <iostream>

#define		Window_Width	800
#define		Window_Height	800

int			point[100][2];
int			p[100][2];
int			num = 0;

long long factorial(int n)
{
	if (n <= 1) return 1;
	else return n * factorial(n - 1);
}

float binomial(int n, int i)
{
	float value = factorial(n) / (factorial(n - i) * factorial(i));
	return value;
}

float bernstein(int degree, int index, float time)
{
	float value;
	value = binomial(degree, index) * pow((1 - time), degree - index) * pow(time, index);
	return value;
}

void Draw_Bezier_Curve(void)
{
	int curve_degree;
	int curvepoint_num = num;
	float patial_time;
	float sumx;
	float sumy;

	glColor3f(1.0, 1.0, 0.0);
	
	curve_degree = curvepoint_num - 1;
	patial_time = 1.0 / curvepoint_num;

	glBegin(GL_LINE_STRIP);
	for (float time = 0; time <= 1.0; time += patial_time)
	{
		sumx = sumy = 0.0;
	
		for (int i = 0; i < curvepoint_num; i++)
		{
			sumx += bernstein(curve_degree, i, time) * point[i][0];
			sumy += bernstein(curve_degree, i, time) * point[i][1];
		}
		glVertex2f(sumx, sumy);
	}
	glEnd();
}
void Draw_Control_Points(void) {

	glPointSize(5.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_POINTS);
	for (int k = 0; k < num; k++) {
		glVertex2f(point[k][0], point[k][1]);
	}
	glEnd();

}

void RenderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glColor3f(1.0, 0.0, 0.0);

	if (num >= 1)
		Draw_Control_Points();
	if (num >= 2)
		Draw_Bezier_Curve();

	glFlush();
	glutSwapBuffers();
}
void init(void) {
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION); // Set projection parameters.
	glLoadIdentity();
	gluOrtho2D(0, Window_Width, 0, Window_Height);
}
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		point[num][0] = x;
		point[num][1] = Window_Height - y;
		num++;
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		num = 0;
	}
	glutPostRedisplay();
	RenderScene();
}


int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(Window_Width, Window_Height);
	glutCreateWindow("Bezier Curve");
	init();
	glutDisplayFunc(RenderScene);
	glutMouseFunc(mouse);
	glutMainLoop();
	return 0;
}