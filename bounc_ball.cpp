#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h> // (or others, depending on the system in use)
#include <math.h>
#include <stdio.h>
#include <cmath>

#include <iostream>

#define	width			1000
#define	height			700
#define	PI				3.1415
#define	polygon_num		50

/*	조작법
방향키로 bar 이동
u 공 속도 빠르게	d 공 속도 느리게
n 공 멈추기			r 리셋
*/

int		n;
int		left;
int		bottom;
int		field_shape;
int		time5;

class Vector
{
public:
	Vector(float xx = 0, float yy = 0) : x(xx), y(yy) {}
	void change(float xx, float yy);
	void change(Vector a);
	float dot_product(Vector a);
	float cross_product(Vector a);
	float size();
	float x, y;
};
void Vector::change(float xx, float yy)
{
	x = xx;
	y = yy;
}
void Vector::change(Vector a)
{
	x = a.x;
	y = a.y;
}
float Vector::dot_product(Vector a)
{
	return ((a.x * x) + (a.y * y));
}
float Vector::cross_product(Vector a)
{
	return abs(a.x * y - a.y * x);
}
float Vector::size()
{
	return sqrt(pow(x, 2) + pow(y, 2));
}

class Point
{
public:
	Point() : x(0), y(0) {}
	Point(float xx, float yy) : x(xx), y(yy) {}
	Point collision_point(Point a, Point b);
	void point_substitution(Point a);
	void change(float xx, float yy);
	void change(Point a);
	float dist(Point a);
	float point_to_segment(Point a, Point b);
	float x, y;
};
Point Point::collision_point(Point a, Point b)
{
	Point tem;
	float m1, k1, m2, k2;
	if (a.y == b.y) // 수평
	{
		tem.x = x; tem.y = a.y;
	}
	else if (a.x == b.x) // 수직
	{
		tem.x = a.x; tem.y = y;
	}
	else
	{
		m1 = (a.y - b.y) / (a.x - b.x); // ab ----> a.y = (a.y - b.y)/(a.x - b.x) * a.x
		k1 = -m1 * a.x + a.y;

		m2 = -1.0 / m1; // 두 직선은 직교하기 때문에 곱했을때 값이 -1
		k2 = y - m2 * x; // y = m1x + k1, y = m2x + k2의 교점

		tem.x = (k2 - k1) / (m1 - m2);
		tem.y = m1 * x + k1;
	}
	return tem;
}
void Point::point_substitution(Point a)
{
	x = a.x;
	y = a.y;
}
void Point::change(float xx, float yy)
{
	x = xx;
	y = yy;
}
void Point::change(Point a)
{
	x = a.x;
	y = a.y;
}
float Point::dist(Point a)
{
	return sqrt(pow(a.x - x, 2) + pow(a.y - y, 2));
}
float Point::point_to_segment(Point a, Point b)
{
	Vector ap(x - a.x, y - a.y);
	Vector ab(b.x - a.x, b.y - a.y);
	float dot_p = ap.x * ab.x + ap.y * ab.y;
	float cross_p = ap.cross_product(ab);
	if (dot_p < 0) return dist(a);
	else if (dot_p / ab.size() > ab.size()) return dist(b);
	else return cross_p / ab.size();
}

class Moving_ball
{
public:
	Moving_ball(float x = width / 2, float y = height / 4, float vx = 1.0, float vy = 1.0, float r = 10)
		: pos(x, y), v(vx, vy), radius(r) {}
	void ball_speed(float x, float y);
	Point pos;
	Vector v;
	float radius;
};
void Moving_ball::ball_speed(float x, float y)
{
	v.x = x;
	v.y = y;
}
Moving_ball ball;

class Brick
{
public:
	Brick(int w = 40, int h = 20) : brick_width(w), brick_height(h)
	{
		for (int i = 0; i < 15; i++)
			memset(brick[i], 1, sizeof(brick[i]));
	}
	void draw_brick();
	bool brick[15][15];
	float brick_width, brick_height;
};
void Brick::draw_brick()
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
	Bar(float x = width / 2, float y = height / 5, float w = 300, float h = 25)
		: pos(x, y), bar_width(w), bar_height(h) {}
	void draw();
	void collision_bar();
	float bar_width, bar_height;
	Point pos;
};
void Bar::draw()
{
	glColor3f(0.5, 0.5, 0.5);
	glBegin(GL_POLYGON);
	glVertex2f(pos.x - bar_width / 2, pos.y + bar_height / 2);
	glVertex2f(pos.x + bar_width / 2, pos.y + bar_height / 2);
	glVertex2f(pos.x + bar_width / 2, pos.y - bar_height / 2);
	glVertex2f(pos.x - bar_width / 2, pos.y - bar_height / 2);
	glEnd();
}
void Bar::collision_bar()
{
	Point a(pos.x - bar_width / 2, pos.y + bar_height / 2), b(pos.x + bar_width / 2, pos.y + bar_height / 2);
	if (ball.pos.point_to_segment(a, b) <= ball.radius)
	{
		ball.v.y *= -1;
		ball.pos.y = pos.y + bar_height / 2 + ball.radius;
	}
}

class Rect
{
public:
	Rect()
	{
		wall[0].change(5, 0);					// 좌하
		wall[1].change(5, height - 5);			// 좌상
		wall[2].change(width - 5, height - 5);	// 우상
		wall[3].change(width - 5, 0);			// 우하
	}
	void field();
	void collision_wall();
	Point wall[4];
};
void Rect::field()
{
	glLineWidth(5);
	glColor3f(0.0, 1.0, 0.5);
	glBegin(GL_LINE_STRIP);
	glVertex2f(wall[0].x, wall[0].y);
	glVertex2f(wall[1].x, wall[1].y);
	glVertex2f(wall[2].x, wall[2].y);
	glVertex2f(wall[3].x, wall[3].y);
	glEnd();
}
void Rect::collision_wall()
{
	for (int i = 0; i < 3; i++)
	{
		Point a(wall[i].x, wall[i].y), b(wall[(i + 1)].x, wall[(i + 1)].y);
		if (ball.pos.point_to_segment(a, b) <= ball.radius)
		{
			Vector u, w, n, ab(b.x - a.x, b.y - a.y);
			w.change(-ball.v.x, -ball.v.y);
			n.change(ab.y / ab.size(), -ab.x / ab.size());
			u.x = 2 * w.dot_product(n) * n.x - w.x;
			u.y = 2 * w.dot_product(n) * n.y - w.y;
			ball.v.change(u);

			if (width - 5 <= ball.pos.x + ball.radius) ball.pos.x = width - 5 - ball.radius;		// 오른쪽 벽 넘어갈시 위치조정
			else if (5 >= ball.pos.x - ball.radius) ball.pos.x = 5 + ball.radius;					// 왼쪽 벽 넘어갈시 위치조정
			else if (height - 5 <= ball.pos.y + ball.radius) ball.pos.y = height - 5 - ball.radius;	// 위쪽 벽 넘어갈시 위치조정
		}
	}
}

class Rhombus
{
public:
	Rhombus()
	{
		wall[0].change(0, height / 2);
		wall[1].change(width / 2, height);
		wall[2].change(width, height / 2);
		wall[3].change(width / 2, 0);
	}
	void field();
	void collision_wall();
	Point wall[4];
};
void Rhombus::field()
{
	glLineWidth(5);
	glColor3f(0.0, 1.0, 0.5);
	glBegin(GL_LINE_LOOP);
	glVertex2f(wall[0].x, wall[0].y);
	glVertex2f(wall[1].x, wall[1].y);
	glVertex2f(wall[2].x, wall[2].y);
	glVertex2f(wall[3].x, wall[3].y);
	glEnd();
}
void Rhombus::collision_wall()
{
	for (int i = 0; i < 4; i++)
	{
		Point a(wall[i].x, wall[i].y), b(wall[(i + 1) % 4].x, wall[(i + 1) % 4].y);
		if (ball.pos.point_to_segment(a, b) <= ball.radius)
		{
			Vector u, w, n, ab(b.x - a.x, b.y - a.y);
			w.change(-ball.v.x, -ball.v.y);
			n.change(ab.y / ab.size(), -ab.x / ab.size());
			u.x = 2 * w.dot_product(n) * n.x - w.x;
			u.y = 2 * w.dot_product(n) * n.y - w.y;
			ball.v.change(u);

			/*공이 벽에 먹히는 현상 처리 해야함*/
		}
	}
}

class Circle
{
public:
	Circle() : mid(width /2, height / 2), radius(300) {}
	void field();
	void collision_wall();
	Point mid;
	float radius;
};
void Circle::field()
{
	float	delta;
	delta = 2 * PI / polygon_num;

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < polygon_num; i++)
		glVertex2f(mid.x + radius * cos(delta * i), mid.y + radius * sin(delta * i));
	glEnd();
}
void Circle::collision_wall()
{
	if (ball.pos.dist(mid) + ball.radius >= radius)
	{
		Vector n, w, u;
		n.change(ball.pos.x - mid.x, ball.pos.y - mid.y);
		n.change(n.x / n.size(), n.y / n.size());
		w.change(-ball.v.x, -ball.v.y);
		u.x = 2 * w.dot_product(n) * n.x - w.x;
		u.y = 2 * w.dot_product(n) * n.y - w.y;
		ball.v.change(u);

		/*공이 벽에 먹히는 현상 처리 해야함*/
	}
}

class Field
{
public:
	Field() : num(0) { wall[0].change(0, 0); }
	void draw();
	void collision_wall();
	int num;
	Point wall[100];
};
void Field::draw()
{
	for (int i = 0; i < num / 2; i++)
	{
		glColor3f(0.0, 0.5, 0.5);
		glBegin(GL_LINES);
		glVertex2f(wall[i * 2].x, wall[i * 2].y);
		glVertex2f(wall[i * 2 + 1].x, wall[i * 2 + 1].y);
		glEnd();
	}
}
void Field::collision_wall()
{
	for (int i = 0; i < num / 2; i++)
	{
		Point a(wall[i * 2].x, wall[i * 2].y), b(wall[(i * 2 + 1)].x, wall[(i * 2 + 1)].y);
		if (ball.pos.point_to_segment(a, b) <= ball.radius)
		{
			Vector u, w, n, ab(b.x - a.x, b.y - a.y);
			w.change(-ball.v.x, -ball.v.y);
			n.change(ab.y / ab.size(), -ab.x / ab.size());
			u.x = 2 * w.dot_product(n) * n.x - w.x;
			u.y = 2 * w.dot_product(n) * n.y - w.y;
			ball.v.change(u);

			/*공이 벽에 먹히는 현상 처리 해야함*/
		}
	}
}

Field field;
Rect rect;
Rhombus rhombus;
Circle circle;
Bar bar;
Brick brick;
Point p1; // 충돌체크 할때 필요한 직선(선분)의 두 점
Point p2;

void init(void)
{
	ball.v.x = -0.125;		ball.v.y = 0.04;
	bar.bar_width = 300;	bar.bar_height = 25;
	ball.radius = 10;
	field_shape = 1;
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

void Collision(int i, int j, bool s)
{
	brick.brick[i][j] = 0;
	ball.pos = ball.pos.collision_point(p1, p2);

	if (s == 0) // 세로벽 맞았을때
	{
		if (ball.v.x > 0) ball.pos.x -= ball.radius;
		else ball.pos.x += ball.radius;
		ball.v.x = -1 * ball.v.x;
	}
	else // 가로벽 맞았을때
	{
		if (ball.v.y > 0) ball.pos.y -= ball.radius;
		else ball.pos.y += ball.radius;
		ball.v.y = -1 * ball.v.y;
	}
}

void Collision_Detection_to_brick()
{
	for (int i = 0; i < 15; i++)
		for (int j = 0; j < 15; j++)
		{
			if (i == 14 && brick.brick[14][j] == 1) // 첫째줄 체크
			{
				p1.change(j * brick.brick_width, height - 15 * brick.brick_height);
				p2.change((j + 1) * brick.brick_width, height - 15 * brick.brick_height);
				if (ball.pos.point_to_segment(p1, p2) <= ball.radius) Collision(14, j, 1);
			}
			if (brick.brick[i][j] == 0)
			{
				if (brick.brick[i][j - 1] == 1 && j != 0) // 왼쪽벽
				{
					p1.change(j * brick.brick_width, height - i * brick.brick_height);
					p2.change(j * brick.brick_width, height - (i + 1) * brick.brick_height);
					if (ball.pos.point_to_segment(p1, p2) <= ball.radius) Collision(i, j - 1, 0);
				}
				if (brick.brick[i][j + 1] == 1 && j != 14) // 오른쪽벽
				{
					p1.change((j + 1) * brick.brick_width, height - i * brick.brick_height);
					p2.change((j + 1) * brick.brick_width, height - (i + 1) * brick.brick_height);
					if (ball.pos.point_to_segment(p1, p2) <= ball.radius) Collision(i, j + 1, 0);
				}
				if (brick.brick[i - 1][j] == 1 && i != 0) // 위쪽벽
				{
					p1.change(j * brick.brick_width, height - i * brick.brick_height);
					p2.change((j + 1) * brick.brick_width, height - i * brick.brick_height);
					if (ball.pos.point_to_segment(p1, p2) <= ball.radius) Collision(i - 1, j, 1);
				}
				if (brick.brick[i + 1][j] == 1 && i != 14) // 아래쪽벽
				{
					p1.change(j * brick.brick_width, height - (i + 1) * brick.brick_height);
					p2.change((j + 1) * brick.brick_width, height - (i + 1) * brick.brick_height);
					if (ball.pos.point_to_segment(p1, p2) <= ball.radius) Collision(i + 1, j, 1);
				}
			}
		}
}

void RenderScene(void) {

	glClearColor(1.0, 1.0, 0.0, 0.0); // Set display-window color to Yellow
	glClear(GL_COLOR_BUFFER_BIT);

	// 움직이는 공의 위치 변화 
	if (n % 2 == 0) // n 누를때마다 멈추기
	{
		ball.pos.x += ball.v.x;
		ball.pos.y += ball.v.y;
	}

	// 충돌 처리 부분
	//Collision_Detection_to_Walls();			// 공과 벽의 충돌 함수
	//Collision_Detection_to_brick();			// 공과 벽돌의 충돌 함수

	// 움직이는 공 그리기 
	glColor3f(0.0, 0.0, 1.0);
	Modeling_Circle(ball.radius, ball.pos);

	switch (field_shape)
	{
	case 1:
		rect.field();
		rect.collision_wall();
		break;
	case 2:
		rhombus.field();
		rhombus.collision_wall();
		break;
	case 3:
		circle.field();
		circle.collision_wall();
		break;
	case 4:
		field.draw();
		field.collision_wall();
	default: break;
	}

	//time5 = (int)glutGet(GLUT_ELAPSED_TIME);
	//if(time5 % 1000 == 0)
	//	std::cout << time5 / 1000 << std::endl;

	//brick.draw_brick();						// 벽돌 그리기

	bar.draw();									//  bar 그리기
	bar.collision_bar();

	glutSwapBuffers();
	glFlush();
}

void Reset() // 초기상태로 돌리기
{
	ball.pos.x = width / 2; ball.pos.y = height / 4;
	ball.ball_speed(0.125, 0.075);
	bar.pos.x = width / 2; bar.pos.y = height / 5;
	for (int i = 0; i < 15; i++)
		memset(brick.brick[i], 1, sizeof(brick.brick[i]));
}

void MyKey(unsigned char key, int x, int y)
{

	switch (key) {
	case 'n':	n += 1;								// n을 눌러서 공을 멈추기
		break;
	case 'u':	ball.v.x *= 1.1; ball.v.y *= 1.1;		// u로 공속도 올리기
		break;
	case 'd':	ball.v.x /= 1.1; ball.v.y /= 1.1;		// d로 공속도 내리기
		break;
	case 'r':	Reset();								// r로 리셋
		break;
	case '1': field_shape = 1;
		break;
	case '2': field_shape = 2;
		break;
	case '3': field_shape = 3;
		break;
	case '4': field_shape = 4;
		break;
	default: break;
	}
	glutPostRedisplay();
}

void SpecialKey(int key, int x, int y) // bar를 방향키로 조절
{
	switch (key)
	{
	case GLUT_KEY_LEFT:		bar.pos.x -= 30;		break;
	case GLUT_KEY_RIGHT:	bar.pos.x += 30;		break;
	default: break;
	}
	glutPostRedisplay();
}

void MyMouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		field.wall[field.num++].change(x, height - y);
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		field.wall[field.num++].change(x, height - y);

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
		field.num = 0;
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
	glutMouseFunc(MyMouse);
	glutReshapeFunc(MyReshape);
	glutDisplayFunc(RenderScene);
	glutIdleFunc(RenderScene);
	glutMainLoop();
}