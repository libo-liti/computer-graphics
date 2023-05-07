#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h> // (or others, depending on the system in use)
#include <math.h>
#include <stdio.h>
#include <cmath>

#define	width 			600
#define	height			700
#define	PI				3.1415
#define	polygon_num		50

int		num = 1;
int		left = 0;
int		bottom = 0;

class Point
{
public:
	Point(){}
	Point(float xx, float yy) : x(xx), y(yy) {}
	void change(float xx, float yy);
	float x, y;
};
void Point::change(float xx, float yy)
{
	x = xx;
	y = yy;
}

class Moving_ball
{
public:
	Moving_ball(float x = width / 2, float y = height / 4, float vx = 1.0, float vy = 1.0, float r = 10)
		: Pos(x, y), Vel(vx, vy), radius(r) {}
	void ball_speed(float xx, float yy);
	Point Pos;
	Point Vel;
	float radius;
};
void Moving_ball::ball_speed(float xx, float yy)
{
	Vel.x = xx;
	Vel.y = yy;
}

class Brick
{
public:
	Brick(int w = 40, int h = 20) : brick_width(w), brick_height(h)
	{
		for (int i = 0; i < 15; i++)
			memset(brick[i], 1, sizeof(brick[i]));
	}
	void Draw_brick();
	bool brick[15][15];
	int brick_width, brick_height;
};
void Brick::Draw_brick()
{
	for (int i = 0; i < 15; i++)
		for (int j = 0; j < 15; j++)
			if (brick[i][j] == 1)
			{
				glColor3f(0.0, 1.0, 1.0);
				glBegin(GL_LINE_LOOP);
				glVertex2f(j * brick_width, height - i * brick_height);
				glVertex2f((j + 1) * brick_width, height - i * brick_height);
				glVertex2f((j + 1) * brick_width, height - (i + 1) * brick_height);
				glVertex2f(j * brick_width, height - (i + 1) * brick_height);
				glEnd();
			}
}

class Bar
{
public:
	Bar(float x = width / 2, float y = height / 5, int w = 300, int h = 25)
		: bar_Pos(x, y), bar_width(w), bar_height(h){}
	void Draw_bar();
	void bar_size_change(int w, int h);
	int bar_width, bar_height;
	Point bar_Pos;
};
void Bar::Draw_bar()
{
	glColor3f(0.5, 0.5, 0.5);
	glBegin(GL_POLYGON);
	glVertex2f(bar_Pos.x - bar_width / 2, bar_Pos.y + bar_height / 2);
	glVertex2f(bar_Pos.x + bar_width / 2, bar_Pos.y + bar_height / 2);
	glVertex2f(bar_Pos.x + bar_width / 2, bar_Pos.y - bar_height / 2);
	glVertex2f(bar_Pos.x - bar_width / 2, bar_Pos.y - bar_height / 2);
	glEnd();
}
void Bar::bar_size_change(int w, int h)
{
	bar_width = w; bar_height = h;
}

Bar bar;
Moving_ball ball;
Brick brick;
Point first; // 충돌체크 할때 필요한 직선(선분)의 두 점
Point second;

float dist(Point a, Point b) // 두 점 사이의 거리
{
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

float Point_to_Line(Point p, Point a, Point b) // 점과 선분 사이의 거리
{
	float dot_product = (p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y); // 내적값
	float cross_product = abs((a.x - p.x) * (b.y - p.y) - (a.y - p.y) * (b.x - p.x)); // 외적
	float ab = dist(a, b);
	if (dot_product < 0) return dist(a, p); // 내적갑이 마이너스라서 ap 거리
	else if (dot_product / ab > ab) return dist(b, p); // 정사영 길이가 ab 길이보다 길 경우 bp 거리
	else return cross_product / ab; // 정사영 거리(외적 / ab길이) Why? 외적 == 평행사변형 넓이
}

Point Collision_Point(Point p, Point a, Point b) // 충돌 지점 Why? 이동했을때 벽돌 안쪽에 있을경우 벽돌 바깥으로 이동하기 위해
{												// 두 직선을 구해서 교점 구하기
	Point tem;
	float m1, k1, m2, k2;
	if (a.y == b.y) // 수평
	{
		tem.x = p.x; tem.y = a.y;
	}
	else if (a.x == b.x) // 수직
	{
		tem.x = a.x; tem.y = p.y;
	}
	else
	{
		m1 = (a.y - b.y) / (a.x - b.x); // ab ----> y - ay = (ay-by)/(ax-bx) * (x-ax)
		k1 = -m1 * a.x + a.y;

		m2 = -1.0 / m1; // 두 직선은 직교하기 때문에 곱했을때 값이 -1
		k2 = p.y - m2 * p.x; // y = m1x + k1, y = m2x + k2의 교점
	}
	return tem;
}

void init(void)
{
	ball.ball_speed(0.125, 0.075);
	bar.bar_size_change(600, 25);
}

void MyReshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(left, left + width, bottom, bottom + height);
}

void Modeling_Circle(float radius, Point CC) {
	float	delta;

	delta = 2 * PI / polygon_num;
	glBegin(GL_POLYGON);
	for (int i = 0; i < polygon_num; i++)
		glVertex2f(CC.x + radius * cos(delta * i), CC.y + radius * sin(delta * i));
	glEnd();
}

void Collision_Detection_to_Walls(void) {
	// ************** 당신의 코드
	if (ball.Pos.y + ball.radius >= height)
	{
		ball.Vel.y = -1 * ball.Vel.y;
		ball.Pos.y = height - ball.radius;
	}
	else if (ball.Pos.x + ball.radius >= width)
	{
		ball.Vel.x = -1 * ball.Vel.x;
		ball.Pos.x = width - ball.radius;
	}
	else if (ball.Pos.x - ball.radius <= left)
	{
		ball.Vel.x = -1 * ball.Vel.x;
		ball.Pos.x = left + ball.radius;
	}
	
	first.change(bar.bar_Pos.x - bar.bar_width / 2, bar.bar_Pos.y + bar.bar_height / 2);
	second.change(bar.bar_Pos.x + bar.bar_width / 2, bar.bar_Pos.y + bar.bar_height / 2);

	if (Point_to_Line(ball.Pos, first, second) <= ball.radius) // bar랑 충돌할때
	{
		ball.Pos = Collision_Point(ball.Pos, first, second);
		ball.Pos.y += ball.radius;
		ball.Vel.y = -1 * ball.Vel.y;
	}
	/*else if (ball.Pos.y - ball.radius <= bottom)
	{
		ball.Vel.y = -1 * ball.Vel.y;
		ball.Pos.y = bottom + ball.radius;
	}*/
}

void Collision(int i, int j, bool s)
{
	brick.brick[i][j] = 0;
	ball.Pos = Collision_Point(ball.Pos, first, second);

	if (s == 0) // 세로벽 맞았을때
	{
		if (ball.Vel.x > 0) ball.Pos.x -= ball.radius;
		else ball.Pos.x += ball.radius;
		ball.Vel.x = -1 * ball.Vel.x;
	}
	else // 가로벽 맞았을때
	{
		if (ball.Vel.y > 0) ball.Pos.y -= ball.radius;
		else ball.Pos.y += ball.radius;
		ball.Vel.y = -1 * ball.Vel.y;
	}
}

void Collision_Detection_to_brick()
{
	for(int i = 0; i < 15; i++)
		for (int j = 0; j < 15; j++)
		{
			if (i == 14 && brick.brick[14][j] == 1) // 첫째줄 체크
			{
				first.change(j * brick.brick_width, height - 15 * brick.brick_height);
				second.change((j + 1) * brick.brick_width, height - 15 * brick.brick_height);
				if (Point_to_Line(ball.Pos, first, second) <= ball.radius) Collision(14, j, 1);
			}
			if (brick.brick[i][j] == 0)
			{
				if (brick.brick[i][j - 1] == 1 && j != 0) // 왼쪽벽
				{
					first.change(j * brick.brick_width, height - i * brick.brick_height);
					second.change(j * brick.brick_width, height - (i + 1) * brick.brick_height);
					if (Point_to_Line(ball.Pos, first, second) <= ball.radius) Collision(i, j - 1, 0);
				}
				if (brick.brick[i][j + 1] == 1 && j != 14) // 오른쪽벽
				{
					first.change((j + 1) * brick.brick_width, height - i * brick.brick_height);
					second.change((j + 1) * brick.brick_width, height - (i + 1) * brick.brick_height);
					if (Point_to_Line(ball.Pos, first, second) <= ball.radius) Collision(i, j + 1, 0);
				}
				if (brick.brick[i - 1][j] == 1 && i != 0) // 위쪽벽
				{
					first.change(j * brick.brick_width, height - i * brick.brick_height);
					second.change((j + 1) * brick.brick_width, height - i * brick.brick_height);
					if (Point_to_Line(ball.Pos, first, second) <= ball.radius) Collision(i - 1, j, 1);
				}
				if (brick.brick[i + 1][j] == 1 && i != 14) // 아래쪽벽
				{
					first.change(j * brick.brick_width, height - (i + 1) * brick.brick_height);
					second.change((j + 1) * brick.brick_width, height - (i + 1) * brick.brick_height);
					if (Point_to_Line(ball.Pos, first, second) <= ball.radius) Collision(i + 1, j, 1);
				}
			}
		}
}

void RenderScene(void) {

	glClearColor(1.0, 1.0, 0.0, 0.0); // Set display-window color to Yellow
	glClear(GL_COLOR_BUFFER_BIT);

	// 움직이는 공의 위치 변화 
	if (num % 2) // n 누를때마다 멈추기
	{
		ball.Pos.x += ball.Vel.x;
		ball.Pos.y += ball.Vel.y;
	}

	// 충돌 처리 부분
	Collision_Detection_to_Walls();			// 공과 벽의 충돌 함수
	Collision_Detection_to_brick();			// 공과 벽돌의 충돌 함수

	// 움직이는 공 그리기 
	glColor3f(0.0, 0.0, 1.0);
	Modeling_Circle(ball.radius, ball.Pos);

	brick.Draw_brick();						// 벽돌 그리기
	bar.Draw_bar();							//  bar 그리기

	glutSwapBuffers();
	glFlush();
}

void Reset() // 초기상태로 돌리기
{
	ball.Pos.x = width / 2; ball.Pos.y = height / 4;
	ball.ball_speed(0.125, 0.075);
	bar.bar_Pos.x = width / 2; bar.bar_Pos.y = height / 5;
	for (int i = 0; i < 15; i++)
		memset(brick.brick[i], 1, sizeof(brick.brick[i]));
}

void MyKey(unsigned char key, int x, int y)
{

	switch (key) {
	case 'n':	num += 1;								// n을 눌러서 공을 멈추기
		break;
	case 'u':	ball.Vel.x *= 1.1; ball.Vel.y *= 1.1;	// u로 공속도 올리기
		break;
	case 'd':	ball.Vel.x /= 1.1; ball.Vel.y /= 1.1;	// d로 공속도 내리기
		break;
	case 'r':											// r로 리셋
		Reset();
	default:	break;
	}
	glutPostRedisplay();
}

void SpecialKey(int key, int x, int y) // bar를 방향키로 조절
{
	switch (key)
	{
	case GLUT_KEY_LEFT:		bar.bar_Pos.x -= 10;		break;
	case GLUT_KEY_RIGHT:	bar.bar_Pos.x += 10;		break;
	default: break;
	}
	glutPostRedisplay();
}

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowPosition(100, 100);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(width, height);
	glutCreateWindow("Bouncing Ball & Wall");
	init();
	glutKeyboardFunc(MyKey);
	glutSpecialFunc(SpecialKey);
	glutReshapeFunc(MyReshape);
	glutDisplayFunc(RenderScene);
	glutIdleFunc(RenderScene);
	glutMainLoop();
}