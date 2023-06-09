#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h> // (or others, depending on the system in use)
#include <math.h>
#include <stdio.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <time.h>
#include < GL/freeglut.h>
#include <string>

#pragma comment(lib, "winmm.lib")
#include "Mmsystem.h"
#include "Digitalv.h"
MCI_OPEN_PARMS m_mciOpenParms;
MCI_PLAY_PARMS m_mciPlayParms;
DWORD m_dwDeviceID;
MCI_OPEN_PARMS mciOpen;
MCI_PLAY_PARMS mciPlay;

#define	width			1000
#define	height			700
#define	PI				3.1415
#define	polygon_num		50
#define zelda			"zelda.mp3"
#define maple			"maple.mp3"
#define river			"river_flows_in_you.mp3"
#define sound			"sound.wav"
#define sound2			"sound2.wav"

/*	조작법
방향키로 bar 이동
u 공 속도 빠르게	d 공 속도 느리게
n 공 멈추기			r 리셋
*/

int		dwID;
int		bgm_mode;
int		n;
int		left;
int		bottom;
int		field_shape;
int		time5;
int		game_mode;
int		couple_game_mode;
bool	multi;
bool	couple_mode;

void bgm(int mode);
void field_select();
void game_mode_select();
void moving_ball_draw();
void move_ball();

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
	bool equal(Point a);
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
bool Point::equal(Point a)
{
	if (x == a.x && y == a.y) return 1;
	else return 0;
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
	else if (dot_p / ab.size() > ab.size()) return dist(b); // 정확히는 ab 사이즈 제곱 + 원의 반지름 제곱이랑 ap사이즈 제곱 비교해야함
	else return cross_p / ab.size();
}

class Moving_ball
{
public:
	Moving_ball(float x = width / 2, float y = height / 4, float vx = 0.2, float vy = 0.1, float r = 10)
		: pos(x, y), v(vx, vy), radius(r), tem_v(0, 0) {}
	void ball_speed(float x, float y);
	Point pos;
	Vector v, tem_v;
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
	Brick(int w = 49, int h = 20) : brick_width(w), brick_height(h)
	{
		for (int i = 0; i < 15; i++)
			memset(brick[i], 1, sizeof(brick[i]));
	}
	void draw_brick();
	bool brick[15][20];
	float brick_width, brick_height;
};
void Brick::draw_brick()
{
	for (int i = 0; i < 15; i++)
		for (int j = 0; j < 20; j++)
			if (brick[i][j] == 1)
			{
				glColor3f(0.0, 1.0, 1.0);
				glBegin(GL_LINE_LOOP);
				glVertex2f(10 + j * brick_width, height - i * brick_height - 10);
				glVertex2f(10 + (j + 1) * brick_width, height - i * brick_height - 10);
				glVertex2f(10 + (j + 1) * brick_width, height - (i + 1) * brick_height - 10);
				glVertex2f(10 + j * brick_width, height - (i + 1) * brick_height - 10);
				glEnd();
			}
}
Brick brick;

class Bar
{
public:
	Bar(float x = width / 2, float y = height / 5, float w = 300, float h = 25)
		: pos(x, y), bar_width(w), bar_height(h), bar_move(true) {}
	void draw();
	void collision_bar(Moving_ball& ball);
	void zelda_mode(Moving_ball& ball);
	void change_pos(float x, float y);

	bool bar_move;
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
void Bar::collision_bar(Moving_ball& ball)
{
	Point a(pos.x - bar_width / 2, pos.y + bar_height / 2), b(pos.x + bar_width / 2, pos.y + bar_height / 2);
	if (ball.pos.point_to_segment(a, b) <= ball.radius)
	{
		ball.v.y *= -1;
		ball.pos.y = pos.y + bar_height / 2 + ball.radius;
	}
}
void Bar::zelda_mode(Moving_ball& ball)
{
	Point a(pos.x - bar_width / 2, pos.y + bar_height / 2), b(pos.x + bar_width / 2, pos.y + bar_height / 2);
	if (ball.pos.point_to_segment(a, b) <= ball.radius)
	{
		ball.pos.y = pos.y + bar_height / 2 + ball.radius;
		ball.v.x = 0; ball.v.y = 0;
		bar_move = false;

		glBegin(GL_LINE_LOOP);
		glVertex2f(ball.pos.x, ball.pos.y);
		glVertex2f(ball.pos.x + ball.tem_v.x * 100, ball.pos.y + ball.tem_v.y * 100);
		glEnd();
	}
}
void Bar::change_pos(float x, float y)
{
	pos.x = x;
	pos.y = y;
}

void collision_process(Point a, Point b, Moving_ball& moving_ball)
{
	Vector u, w, n, ab(b.x - a.x, b.y - a.y);
	w.change(-moving_ball.v.x, -moving_ball.v.y);
	n.change(ab.y / ab.size(), -ab.x / ab.size());
	u.x = 2 * w.dot_product(n) * n.x - w.x;
	u.y = 2 * w.dot_product(n) * n.y - w.y;
	moving_ball.v.change(u);
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
	void collision_brick();
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
			collision_process(a, b, ball);

			if (width - 10 <= ball.pos.x + ball.radius) ball.pos.x = width - 10 - ball.radius;		// 오른쪽 벽 넘어갈시 위치조정
			else if (10 >= ball.pos.x - ball.radius) ball.pos.x = 10 + ball.radius;					// 왼쪽 벽 넘어갈시 위치조정
			else if (height - 10 <= ball.pos.y + ball.radius) ball.pos.y = height - 10 - ball.radius;	// 위쪽 벽 넘어갈시 위치조정
		}
	}
}
void Rect::collision_brick()		// 두벽 충돌체크, 모서리 체크 해야함
{
	Point a, b, c, d, tem;
	for (int i = 0; i < 20; i++)
	{
		if (brick.brick[14][i] == 1) // 맨 아래쪽 벽돌 충돌 체크
		{
			a.change(10 + i * brick.brick_width, height - 15 * brick.brick_height - 10);
			b.change(10 + (i + 1) * brick.brick_width, height - 15 * brick.brick_height - 10);
			if (ball.pos.point_to_segment(a, b) <= ball.radius)
			{
				brick.brick[14][i] = 0;
				tem.change(ball.pos.collision_point(a, b));
				if (b.equal(tem))
				{
					brick.brick[14][i + 1] = 0;
					ball.v.y *= -1;
				}
				else collision_process(a, b, ball);
				ball.pos.y = ball.pos.collision_point(a, b).y - ball.radius;
				return;
			}
		}
	}
	for (int i = 0; i < 15; i++)
		for (int j = 0; j < 20; j++)
		{
			if (brick.brick[i][j] == 0)
			{
				if (j != 0 && brick.brick[i][j - 1] == 1) // 왼쪽벽
				{
					a.change(10 + j * brick.brick_width, height - i * brick.brick_height - 10);
					b.change(10 + j * brick.brick_width, height - (i + 1) * brick.brick_height - 10);
					c.change(10 + (j + 1) * brick.brick_width, height - i * brick.brick_height - 10);
					c.change(10 + (j + 1) * brick.brick_width, height - (i + 1) * brick.brick_height - 10);
					//if (ball.pos.point_to_segment(a, b) <= ball.radius && ball.pos.point_to_segment(a, c) <= ball.radius)	// 왼쪽벽과 위쪽벽이 동시에 닿아있다면?
					//{
					//	ball.pos.x -= ball.v.x; ball.pos.y -= ball.v.y;														// 한칸 되돌려서 더 가까운쪽 판단
					//	(ball.pos.point_to_segment(a, b) < ball.pos.point_to_segment(a, c)) ? 
					//}
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick.brick[i][j - 1] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick.brick[i + 1][j - 1] == 1)
						{
							brick.brick[i + 1][j - 1] = 0;
							ball.v.x *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.x = ball.pos.collision_point(a, b).x + ball.radius;
						return;
					}
				}
				if (j != 19 && brick.brick[i][j + 1] == 1) // 오른벽
				{
					a.change(10 + (j + 1) * brick.brick_width, height - i * brick.brick_height - 10);
					b.change(10 + (j + 1) * brick.brick_width, height - (i + 1) * brick.brick_height - 10);
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick.brick[i][j + 1] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick.brick[i + 1][j + 1] == 1)
						{
							brick.brick[i + 1][j + 1] = 0;
							ball.v.x *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.x = ball.pos.collision_point(a, b).x - ball.radius;
						return;
					}
				}
				if (i != 0 && brick.brick[i - 1][j] == 1) // 위쪽벽
				{
					a.change(10 + j * brick.brick_width, height - i * brick.brick_height - 10);
					b.change(10 + (j + 1) * brick.brick_width, height - i * brick.brick_height - 10);
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick.brick[i - 1][j] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick.brick[i - 1][j + 1] == 1)
						{
							brick.brick[i - 1][j + 1] = 0;
							ball.v.y *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.collision_point(a, b).y - ball.radius;
						return;
					}
				}
				if (i != 14 && brick.brick[i + 1][j] == 1) // 아래쪽벽
				{
					a.change(10 + j * brick.brick_width, height - (i + 1) * brick.brick_height - 10);
					b.change(10 + (j + 1) * brick.brick_width, height - (i + 1) * brick.brick_height - 10);
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick.brick[i + 1][j] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick.brick[i + 1][j + 1] == 1)
						{
							brick.brick[i + 1][j + 1] = 0;
							ball.v.y *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.y = ball.pos.collision_point(a, b).y + ball.radius;
						return;
					}
				}
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
			collision_process(a, b, ball);

			/*공이 벽에 먹히는 현상 처리 해야함*/
		}
	}
}

class Circle
{
public:
	Circle() : mid(width / 2, height / 2), radius(300) {}
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
	void collision_wall(Moving_ball &ball);
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
void Field::collision_wall(Moving_ball &ball)
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

class Couple
{
public:
	Couple() : brick_width(97), brick_height(50), p1(0), p2(0)
	{
		bar[0].pos.x = width / 4; bar[0].pos.y = height / 5; bar[0].bar_width = 100;
		bar[1].pos.x = width / 4 * 3; bar[1].pos.y = height / 5; bar[1].bar_width = 100;

		ball[0].pos.x = width / 4; ball[0].pos.y = height / 4;
		ball[1].pos.x = width / 4 * 3; ball[1].pos.y = height / 4;

		wall[0].change(5, 0);					// 좌하
		wall[1].change(5, height - 5);			// 좌상
		wall[2].change(width / 2, height - 5);	// 중상
		wall[3].change(width - 5, height - 5);	// 우상
		wall[4].change(width - 5, 0);			// 우하
		wall[5].change(width / 2, 0);			// 중하

		std::fill(&brick[0][0][0], &brick[5][4][2], 1);
	}
	void field();
	void collision_wall();
	void draw_brick();
	void collision_brick();
	void create_wall();

	Bar bar[2];
	Moving_ball ball[2];
	Point wall[6];
	int brick_width, brick_height;
	bool brick[6][5][2];
	bool p1, p2;
};
void Couple::field()
{
	glLineWidth(5);
	glColor3f(0.0, 1.0, 0.5);
	glBegin(GL_LINE_STRIP);
	glVertex2f(wall[0].x, wall[0].y);
	glVertex2f(wall[1].x, wall[1].y);
	glVertex2f(wall[2].x, wall[2].y);
	glVertex2f(wall[3].x, wall[3].y);
	glVertex2f(wall[4].x, wall[4].y);
	glEnd();

	glColor3f(0.0, 1.0, 0.5);
	glBegin(GL_LINE_STRIP);
	glVertex2f(wall[2].x, wall[2].y);
	glVertex2f(wall[5].x, wall[5].y);
	glEnd();
}
void Couple::collision_wall()
{
	for (int i = 0; i < 2; i++)
	{
		Point a(wall[i].x, wall[i].y), b(wall[(i + 1)].x, wall[(i + 1)].y);
		if (ball[0].pos.point_to_segment(a, b) <= ball[0].radius)
			collision_process(a, b, ball[0]);
	}
	Point a(wall[2].x, wall[2].y), b(wall[5].x, wall[5].y);
	if (ball[0].pos.point_to_segment(a, b) <= ball[0].radius)
		collision_process(a, b, ball[0]);
	if (ball[1].pos.point_to_segment(a, b) <= ball[1].radius)
		collision_process(a, b, ball[1]);
	for (int i = 2; i < 4; i++)
	{
		Point a(wall[i].x, wall[i].y), b(wall[(i + 1)].x, wall[(i + 1)].y);
		if (ball[1].pos.point_to_segment(a, b) <= ball[1].radius)
			collision_process(a, b, ball[1]);
	}
}
void Couple::draw_brick()
{
	glColor3f(0.5, 0.5, 0.5);
	for (int k = 0; k < 2; k++)
		for (int i = 0; i < 6; i++)
			for (int j = 0; j < 5; j++)
			{
				if (brick[i][j][k])
				{
					glBegin(GL_LINE_LOOP);
					glVertex2f((width / 2 - 5) * k + 10 + j * brick_width, height - i * brick_height - 10);
					glVertex2f((width / 2 - 5) * k + 10 + (j + 1) * brick_width, height - i * brick_height - 10);
					glVertex2f((width / 2 - 5) * k + 10 + (j + 1) * brick_width, height - (i + 1) * brick_height - 10);
					glVertex2f((width / 2 - 5) * k + 10 + j * brick_width, height - (i + 1) * brick_height - 10);
					glEnd();
				}
			}
}
void Couple::collision_brick()
{
	Point a, b, tem;
	for (int k = 0; k < 2; k++)
	{
		for (int i = 0; i < 5; i++)
		{
			if (brick[5][i][k] == 1) // 맨 아래쪽 벽돌 충돌 체크
			{
				a.change((width / 2 - 5) * k + 10 + i * brick_width, height - 6 * brick_height - 10);
				b.change((width / 2 - 5) * k + 10 + (i + 1) * brick_width, height - 6 * brick_height - 10);
				if (ball[k].pos.point_to_segment(a, b) <= ball[k].radius)
				{
					brick[5][i][k] = 0;
					tem.change(ball[k].pos.collision_point(a, b));
					if (b.equal(tem))
					{
						brick[5][i + 1][k] = 0;
						ball[k].v.y *= -1;
					}
					else collision_process(a, b, ball[k]);
					ball[k].pos.y = ball[k].pos.collision_point(a, b).y - ball[k].radius;
					return;
				}
			}
		}
	}
	for (int k = 0; k < 2; k++)
	{
		for (int i = 0; i < 6; i++)
			for (int j = 0; j < 5; j++)
			{
				if (brick[i][j][k] == 0)
				{
					if (j != 0 && brick[i][j - 1][k] == 1) // 왼쪽벽
					{
						a.change((width / 2 - 5) * k + 10 + j * brick_width, height - i * brick_height - 10);
						b.change((width / 2 - 5) * k + 10 + j * brick_width, height - (i + 1) * brick_height - 10);
						if (ball[k].pos.point_to_segment(a, b) <= ball[k].radius)
						{
							brick[i][j - 1][k] = 0;
							tem.change(ball[k].pos.collision_point(a, b));
							if (b.equal(tem) && brick[i + 1][j - 1][k] == 1)
							{
								brick[i + 1][j - 1][k] = 0;
								ball[k].v.x *= -1;
							}
							else collision_process(a, b, ball[k]);
							ball[k].pos.x = ball[k].pos.collision_point(a, b).x + ball[k].radius;
							return;
						}
					}
					if (j != 19 && brick[i][j + 1][k] == 1) // 오른벽
					{
						a.change((width / 2 - 5) * k + 10 + (j + 1) * brick_width, height - i * brick_height - 10);
						b.change((width / 2 - 5) * k + 10 + (j + 1) * brick_width, height - (i + 1) * brick_height - 10);
						if (ball[k].pos.point_to_segment(a, b) <= ball[k].radius)
						{
							brick[i][j + 1][k] = 0;
							tem.change(ball[k].pos.collision_point(a, b));
							if (b.equal(tem) && brick[i + 1][j + 1][k] == 1)
							{
								brick[i + 1][j + 1][k] = 0;
								ball[k].v.x *= -1;
							}
							else collision_process(a, b, ball[k]);
							ball[k].pos.x = ball[k].pos.collision_point(a, b).x - ball[k].radius;
							return;
						}
					}
					if (i != 0 && brick[i - 1][j][k] == 1) // 위쪽벽
					{
						a.change((width / 2 - 5) * k + 10 + j * brick_width, height - i * brick_height - 10);
						b.change((width / 2 - 5) * k + 10 + (j + 1) * brick_width, height - i * brick_height - 10);
						if (ball[k].pos.point_to_segment(a, b) <= ball[k].radius)
						{
							brick[i - 1][j][k] = 0;
							tem.change(ball[k].pos.collision_point(a, b));
							if (b.equal(tem) && brick[i - 1][j + 1][k] == 1)
							{
								brick[i - 1][j + 1][k] = 0;
								ball[k].v.y *= -1;
							}
							else collision_process(a, b, ball[k]);
							ball[k].pos.collision_point(a, b).y - ball[k].radius;
							return;
						}
					}
					if (i != 14 && brick[i + 1][j][k] == 1) // 아래쪽벽
					{
						a.change((width / 2 - 5) * k + 10 + j * brick_width, height - (i + 1) * brick_height - 10);
						b.change((width / 2 - 5) * k + 10 + (j + 1) * brick_width, height - (i + 1) * brick_height - 10);
						if (ball[k].pos.point_to_segment(a, b) <= ball[k].radius)
						{
							brick[i + 1][j][k] = 0;
							tem.change(ball[k].pos.collision_point(a, b));
							if (b.equal(tem) && brick[i + 1][j + 1][k] == 1)
							{
								brick[i + 1][j + 1][k] = 0;
								ball[k].v.y *= -1;
							}
							else collision_process(a, b, ball[k]);
							ball[k].pos.y = ball[k].pos.collision_point(a, b).y + ball[k].radius;
							return;
						}
					}
				}
			}
	}
}
void Couple::create_wall()
{
	glColor3f(0.7, 0.0, 0.3);
	if (p1)
	{
		glBegin(GL_LINES);
		glVertex2f(10, height / 2);
		glVertex2f(width / 6, height / 2);
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(width / 6 * 2, height / 2);
		glVertex2f(width / 2 - 5, height / 2);
		glEnd();
		Point a(10, height / 2), b(width / 6, height / 2);
		if (ball[0].pos.point_to_segment(a, b) <= ball[0].radius)
		{
			collision_process(a, b, ball[0]);
			p1 = !p1;
		}
		a.change(width / 6 * 2, height / 2); b.change(width / 2 - 5, height / 2);
		if (ball[0].pos.point_to_segment(a, b) <= ball[0].radius)
		{
			collision_process(a, b, ball[0]);
			p1 = !p1;
		}
	}
	if (p2)
	{
		glBegin(GL_LINES);
		glVertex2f(width / 2 + 5, height / 2);
		glVertex2f(width / 6 * 4, height / 2);
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(width / 6 * 5, height / 2);
		glVertex2f(width - 5, height / 2);
		glEnd();
		Point a(width / 2 + 5, height / 2), b(width / 6 * 4, height / 2);
		if (ball[1].pos.point_to_segment(a, b) <= ball[1].radius)
		{
			collision_process(a, b, ball[1]);
			p2 = !p2;
		}
		a.change(width / 6 * 5, height / 2); b.change(width - 5, height / 2);
		if (ball[1].pos.point_to_segment(a, b) <= ball[1].radius)
		{
			collision_process(a, b, ball[1]);
			p2 = !p2;
		}

	}
}

Couple couple;
Field field;
Rect rect;
Rhombus rhombus;
Circle circle;
Bar bar;

time_t start, end;
int current_time;
int last_time;
// 초기설정
void init(void)
{
	ball.v.x = 0.4;		ball.v.y = 0.3;
	bar.bar_width = 300;	bar.bar_height = 25;
	ball.radius = 10;
	field_shape = 1;		game_mode = 1;
	couple_game_mode = 1;

	couple.ball[0].pos.x = width / 4; couple.ball[0].pos.y = height / 4;
	couple.ball[1].pos.x = width / 4 * 3; couple.ball[1].pos.y = height / 4;
	couple.ball[0].ball_speed(0.125, 0.075); couple.ball[1].ball_speed(0.125, 0.075);
	couple.bar[0].bar_width = 100; couple.bar[1].bar_width = 100;
	time(&start);	time(&end);
}

void MyReshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(left, left + width + 300, bottom, bottom + height);
}
// 원 그리기
void Modeling_Circle(float radius, Point CC) {
	float	delta;

	delta = 2 * PI / polygon_num;
	glBegin(GL_POLYGON);
	for (int i = 0; i < polygon_num; i++)
		glVertex2f(CC.x + radius * cos(delta * i), CC.y + radius * sin(delta * i));
	glEnd();
} 

void text()
{
	std::string time_print, time_min, time_sec, bgm_name, game_mode_name;

	time_min = current_time / 60;
	time_sec = current_time % 60;

	time(&end);
	current_time = end - start;
	if (current_time != last_time)
		last_time = current_time;

	//const unsigned char* t = reinterpret_cast<const unsigned char*>("text to render");

	if (current_time / 60 < 10)
		time_min = "0" + std::to_string(current_time / 60);
	else
		time_min = std::to_string(current_time / 60);
	if (current_time % 60 < 10)
		time_sec = "0" + std::to_string(current_time % 60);
	else
		time_sec = std::to_string(current_time % 60);
	time_print = time_min + " : " + time_sec;

	const unsigned char* t = (unsigned char*)time_print.c_str();
	glColor3f(1.0f, 1.0f, 1.0f);
	glRasterPos2i(width + 100, height / 2);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, t);

	bgm_name = "bgm : ";
	switch (bgm_mode % 3)
	{
	case 0: bgm_name += zelda; break;
	case 1: bgm_name += maple; break;
	case 2: bgm_name += river; break;
	}

	const unsigned char* name = (unsigned char*)bgm_name.c_str();
	glRasterPos2i(width + 10, height / 3);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, name);

	game_mode_name = "game mode : ";
	if (!couple_mode)
	{
		switch (game_mode)
		{
		case 1: game_mode_name += "nomal"; break;
		case 2: game_mode_name += "zelda"; break;
		case 3: game_mode_name += "zelda + draw"; break;
		case 4: game_mode_name += "no bar + draw"; break;
		}
	}
	else
	{
		switch (couple_game_mode)
		{
		case 1: game_mode_name += "nomal"; break;
		case 2: game_mode_name += "zelda"; break;
		case 3: game_mode_name += "zelda + draw"; break;
		case 4: game_mode_name += "no bar + draw"; break;
		}
	}

	const unsigned char* mode_name = (unsigned char*)game_mode_name.c_str();
	glRasterPos2i(width + 10, height / 4);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, mode_name);
}

void field_select()
{
	switch (field_shape)
	{
	case 1:							// 사각형
		rect.field();
		rect.collision_wall();
		break;
	case 2:
		rhombus.field();			// 마름모
		rhombus.collision_wall();
		break;
	case 3:
		circle.field();				// 원
		circle.collision_wall();
		break;
	case 4:							// 사용자가 직접 만드는거
		break;
	case 5:
		couple.field();
		break;
	default: break;
	}
}

void game_mode_select()
{
	switch (game_mode)
	{
	case 1:
		bar.draw();
		bar.collision_bar(ball);
		field.draw();
		field.collision_wall(ball);
		break;
	case 2:
		bar.draw();
		bar.zelda_mode(ball);
		field.draw();
		field.collision_wall(ball);
		break;
	case 3:
		bar.draw();
		bar.zelda_mode(ball);
		field.draw();
		field.collision_wall(ball);
		break;
	case 4:
		field.draw();
		field.collision_wall(ball);
		break;
	case 5:
		
		switch (couple_game_mode)
		{
		case 1:
			couple.bar[0].draw(); couple.bar[1].draw();
			couple.bar[0].collision_bar(couple.ball[0]);
			couple.bar[1].collision_bar(couple.ball[1]);
			break;
		case 2:
			couple.bar[0].draw(); couple.bar[1].draw();
			couple.bar[0].zelda_mode(couple.ball[0]);
			couple.bar[1].zelda_mode(couple.ball[1]);
			break;
		case 3:
			couple.bar[0].draw(); couple.bar[1].draw();
			couple.bar[0].zelda_mode(couple.ball[0]);
			couple.bar[1].zelda_mode(couple.ball[1]);
			field.draw();
			field.collision_wall(couple.ball[0]);
			field.collision_wall(couple.ball[1]);
			break;
		case 4:
			field.draw();
			field.collision_wall(couple.ball[0]);
			field.collision_wall(couple.ball[1]);
			break;
		}
		couple.create_wall();
		couple.collision_wall();
		couple.collision_brick();
		couple.draw_brick();
		break;
	default: break;
	}
}

void moving_ball_draw()
{
	if (couple_mode)
	{
		glColor3f(0.0, 0.0, 1.0);
		Modeling_Circle(ball.radius, couple.ball[0].pos);
		glColor3f(1.0, 0.0, 0.0);
		Modeling_Circle(ball.radius, couple.ball[1].pos);
	}
	else
	{
		glColor3f(1.0, 1.0, 1.0);
		Modeling_Circle(ball.radius, ball.pos);
	}
}

void move_ball()
{
	if (n % 2 == 0) // n 누를때마다 멈추기
	{
		if (couple_mode)
		{
			couple.ball[0].pos.x += couple.ball[0].v.x;
			couple.ball[0].pos.y += couple.ball[0].v.y;

			couple.ball[1].pos.x += couple.ball[1].v.x;
			couple.ball[1].pos.y += couple.ball[1].v.y;
		}
		else
		{
			ball.pos.x += ball.v.x;
			ball.pos.y += ball.v.y;
		}
	}
}

void RenderScene(void) {

	glClearColor(0.0, 0.0, 0.0, 0.0); // Set display-window color to Yellow
	glClear(GL_COLOR_BUFFER_BIT);
 
	move_ball();
	field_select();
	game_mode_select();
	moving_ball_draw();

	text();

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
	time(&start);
}

void couple_reset()
{
	couple.ball[0].pos.x = width / 4; couple.ball[0].pos.y = height / 4;
	couple.ball[1].pos.x = width / 4 * 3; couple.ball[1].pos.y = height / 4;
	couple.ball[0].ball_speed(0.125, 0.075); couple.ball[1].ball_speed(0.125, 0.075);
	std::fill(&couple.brick[0][0][0], &couple.brick[5][4][2], 1);
	time(&start);
}

void MyKey(unsigned char key, int x, int y)
{
	if (couple_mode)
	{
		if (!couple.bar[0].bar_move)
		{
			switch (key)
			{
			case 'a': couple.ball[0].tem_v.x -= 0.1;		break;
			case 'd': couple.ball[0].tem_v.x += 0.1;		break;
			case 'w': couple.ball[0].tem_v.y += 0.1;		break;
			case 's': couple.ball[0].tem_v.y -= 0.1;		break;
			case 'r': couple_reset();						break;
			case '1': couple_game_mode = 1;					break;
			case '2': couple_game_mode = 2;					break;
			case '3': couple_game_mode = 3;					break;
			case '4': couple_game_mode = 4;					break;
			case 'u': 
				couple.ball[0].v.x *= 1.1; couple.ball[0].v.y *= 1.1;
				couple.ball[1].v.x *= 1.1; couple.ball[1].v.y *= 1.1;
				break;
			case '\'': couple.p1 = !couple.p1;				break;
			case 'b': couple.p2 = !couple.p2;				break;
			case 'n':	n += 1;								break;			// n을 눌러서 공을 멈추기
			case 32:
				couple.ball[0].v.change(couple.ball[0].tem_v);
				couple.ball[0].tem_v.change(0, 0);
				couple.bar[0].bar_move = true;
				break;
			case 'c': couple_mode = !couple_mode; field_shape = 1; game_mode = 1; time(&start); break;
			case 'm': bgm(++bgm_mode); break;
			case 27: exit(0); break;
			}
		}
		else
		{
			switch (key)
			{
			case 'a': couple.bar[0].pos.x -= 30;			break;
			case 'd': couple.bar[0].pos.x += 30;			break;
			case 'r': couple_reset();						break;
			case '0': couple_game_mode = 1;					break;
			case '1': couple_game_mode = 2;					break;
			case '2': couple_game_mode = 3;					break;
			case '3': couple_game_mode = 4;					break;
			case 'u':
				couple.ball[0].v.x *= 1.1; couple.ball[0].v.y *= 1.1;
				couple.ball[1].v.x *= 1.1; couple.ball[1].v.y *= 1.1;
				break;
			case '\'': couple.p1 = !couple.p1;				break;
			case 'b': couple.p2 = !couple.p2;				break;
			case 'n':	n += 1;								break;			// n을 눌러서 공을 멈추기
			case 'c': couple_mode = !couple_mode; field_shape = 1; game_mode = 1; time(&start); break;
			case 'm': bgm(++bgm_mode); break;
			case 27: exit(0); break;
			}
		}
		if (!couple.bar[1].bar_move)
		{
			switch (key)
			{
			case '4': couple.ball[1].tem_v.x -= 0.1; break;
			case '6': couple.ball[1].tem_v.x += 0.1; break;
			case '8': couple.ball[1].tem_v.y += 0.1; break;
			case '5': couple.ball[1].tem_v.y -= 0.1; break;
			case 13:
				couple.ball[1].v.change(couple.ball[1].tem_v);
				couple.ball[1].tem_v.change(0, 0);
				couple.bar[1].bar_move = true;
				break;
			}
		}
		else
		{
			switch (key)
			{
			case '4': couple.bar[1].pos.x -= 30; break;
			case '6': couple.bar[1].pos.x += 30; break;
			}
		}
	}
	else
	{
		switch (key) {
		case 'n':	n += 1;												// n을 눌러서 공을 멈추기
			break;
		case 'u':	ball.v.x *= 1.1; ball.v.y *= 1.1;					// u로 공속도 올리기
			break;
		case 'd':	ball.v.x /= 1.1; ball.v.y /= 1.1;					// d로 공속도 내리기
			break;
		case 'r':	Reset();											// r로 리셋
			break;
		case '1': field_shape = 1; time(&start);	// 사각형
			break;
		case '2': field_shape = 2; time(&start);	// 마름모
			break;
		case '3': field_shape = 3; time(&start);	// 원
			break;
		case '4': field_shape = 4; time(&start);	// 아무것도 없는 맵
			break;
		case 'c': field_shape = 5; game_mode = 5; couple_mode = !couple_mode; time(&start);
			break;
		case 32:														// 스페이스 바(2번, 3번 모드일때만 작동)
			if (bar.bar_move) break;
			ball.v.change(ball.tem_v);
			ball.tem_v.change(0, 0);
			bar.bar_move = true;
			break;
		case 'm': bgm(++bgm_mode); break;
		case 27: exit(0); break;
		default: break;
		}
	}
	glutPostRedisplay();
}

void SpecialKey(int key, int x, int y)
{
	if (bar.bar_move)	// 2번, 3번 모드일때 바에 닿으면 방향키로 바가 움직이는게 아니라 공의 V벡터가 움직임
	{
		switch (key)
		{
		case GLUT_KEY_LEFT:		bar.pos.x -= 30;		break;
		case GLUT_KEY_RIGHT:	bar.pos.x += 30;		break;
		case GLUT_KEY_F1:		game_mode = 1;			break;			// 일반 공 튕기기
		case GLUT_KEY_F2:		game_mode = 2;			break;			// 바에 맞으면 공이 멈추고 방향키로 방향 정하고 스페이스 바로 시작
		case GLUT_KEY_F3:		game_mode = 3;			break;			// 2번 모드 + 마우스로 맵 만들 수 있음
		case GLUT_KEY_F4:		game_mode = 4;			break;			// 바 없어지고 마우스로 직접 맵을 만들어야함
		default: break;
		}
	}
	else
	{
		switch (key)
		{
		case GLUT_KEY_RIGHT:	ball.tem_v.x += 0.1;	break;
		case GLUT_KEY_LEFT:		ball.tem_v.x -= 0.1;	break;
		case GLUT_KEY_UP:		ball.tem_v.y += 0.1;	break;
		case GLUT_KEY_DOWN:		ball.tem_v.y -= 0.1;	break;
		case GLUT_KEY_F1:		game_mode = 1;			break;
		case GLUT_KEY_F2:		game_mode = 2;			break;
		case GLUT_KEY_F3:		game_mode = 3;			break;
		case GLUT_KEY_F4:		game_mode = 4;			break;
		default: break;
		}
	}
	glutPostRedisplay();
}

void MyMouse(int button, int state, int x, int y)
{
	if ((game_mode == 3 || game_mode == 4 || couple_game_mode == 3 || couple_game_mode == 4) && x <= width)		// 3번, 4번 모드일때 맵을 그릴 수 있음
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
			field.wall[field.num++].change(x, height - y);
		if(field.num % 2 == 1)
			if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
				field.wall[field.num++].change(x, height - y);
		if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
			field.num = 0;
	}
	else if ((game_mode == 3 || game_mode == 4 || couple_game_mode == 3 || couple_game_mode == 4) && x > width)
	{
		if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)
			field.wall[field.num--];
	}
}

void bgm(int mode)
{
	mciSendCommandW(dwID, MCI_CLOSE, 0, NULL);
	switch (mode % 3)
	{
	case 0:
		mciOpen.lpstrElementName = zelda;
		break;
	case 1:
		mciOpen.lpstrElementName = maple;
		break;
	case 2:
		mciOpen.lpstrElementName = river;
		break;
	}
	mciOpen.lpstrDeviceType = "mpegvideo";

	mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
		(DWORD)(LPVOID)&mciOpen);

	dwID = mciOpen.wDeviceID;

	mciSendCommand(dwID, MCI_PLAY, MCI_DGV_PLAY_REPEAT, // play & repeat
		(DWORD)(LPVOID)&m_mciPlayParms);
}

void main(int argc, char** argv)
{
	bgm(bgm_mode);
	glutInit(&argc, argv);
	glutInitWindowPosition(100, 100);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(width + 300, height);
	glutCreateWindow("Bouncing Ball & Wall");
	init();
	glutKeyboardFunc(MyKey);
	glutSpecialFunc(SpecialKey);
	glutMouseFunc(MyMouse);
	glutReshapeFunc(MyReshape);
	glutDisplayFunc(RenderScene);
	glutIdleFunc(RenderScene);
	glutMainLoop();
	//PlaySound("sound.wav", 0, SND_FILENAME | SND_ASYNC);
}