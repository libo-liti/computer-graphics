#include "gl/glut.h"
#include <math.h>
void display_circle()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(0.0f, 0.0f, 1.0f);

    double rad = 0.5;

    glBegin(GL_POLYGON);
    for (int i = 0; i < 17; i++) // 17번 점을 찍기
    {
        double angle = i * 3.141592 / 180 * 21.17647058823529; // 한번 찍을때마다 얼마씩 각도가 벌어지는지
        double x = rad * cos(angle);
        double y = rad * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
    glFinish();

}
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutCreateWindow
    ("OpenGL");
    glutDisplayFunc(display_circle);
    glutMainLoop();
    return 0;
}