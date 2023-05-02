#include <windows.h>
#include <math.h>
#include <gl/gl.h>
#include <gl/glut.h> // (or others, depending on the system in use)

#define      PI   3.1415926

int         Width = 800;
int         Height = 800;

float      sun_radius = 50.0;
float      mercury_radius = 10.0;
float      earth_radius = 20.0;
float      moon_radius = 5.0;
float      mars_radius = 25.0;

float      sunRotation = 0.0, mercuryRotation = 0.0, earthRotation = 10.0, moonRotation = 0.0, marsRotation = 0.0;

float      sun_rotation_speed = 0.1;
float      mercury_rotation_speed = 2;
float      earth_rotation_speed = 0.2;
float      moon_rotation_speed = 2;
float      mars_rotation_speed = 0.1;


void init(void) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0 * Width / 2.0, Width / 2.0, -1.0 * Height / 2.0, Height / 2.0);
}


void Draw_Circle(float c_radius) {
    float   delta;
    int      num = 32;

    delta = 2 * PI / num;
    glBegin(GL_POLYGON);
    for (int i = 0; i < num; i++)
        glVertex2f(c_radius * cos(delta * i), c_radius * sin(delta * i));
    glEnd();
}

void RenderScene(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 0.0); // Set display-window color to black.

    sunRotation += sun_rotation_speed;
    earthRotation += earth_rotation_speed;
    moonRotation += moon_rotation_speed;
    marsRotation += mars_rotation_speed;
    mercuryRotation += mercury_rotation_speed;

    /***********코드 작성하기 ***************/
    // 태양 : 빨간색의 구
    glPushMatrix();
    glRotatef(sunRotation, 0.0, 0.0, 1.0);
    glColor3f(1.0, 0.0, 0.0);
    Draw_Circle(sun_radius);
    glPopMatrix();

    // 수성
    glPushMatrix();
    glRotatef(mercuryRotation, 0.0, 0.0, 1.0);
    glTranslatef(100, 0.0, 0.0);
    glColor3f(0.0, 0.0, 1.0);
    Draw_Circle(mercury_radius);
    glPopMatrix();

    // 지구 : 초록색의 구
    glPushMatrix();
    glRotatef(earthRotation, 0.0, 0.0, 1.0);
    glTranslatef(200.0, 0.0, 0.0);
    glColor3f(0.0, 1.0, 1.0);
    Draw_Circle(earth_radius);

    // 달   : 노랑색의 구
    glPushMatrix();
    glRotatef(moonRotation, 0.0, 0.0, 1.0);
    glTranslatef(30.0, 0.0, 0.0);
    glColor3f(1.0, 1.0, 0.0);
    Draw_Circle(moon_radius);
    glPopMatrix();
    glPopMatrix();

    //화성
    glPushMatrix();
    glRotatef(marsRotation, 0.0, 0.0, 1.0);
    glTranslatef(350, 0.0, 0.0);
    glColor3f(1.0, 0.3, 0.0);
    Draw_Circle(mars_radius);
    glPopMatrix();

    glFlush();
    glutSwapBuffers();
}


void main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(Width, Height);
    glutCreateWindow("Solar System");
    init();
    glutDisplayFunc(RenderScene);
    glutIdleFunc(RenderScene);
    glutMainLoop();
}
