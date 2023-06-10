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
#include <cstdlib>

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
int		field_shape;
int		game_mode;
int		couple_game_mode;
int current_time;
int last_time;
int clear_time;
int select_num;
int flag;
bool main_num;
float angle;

time_t start, end;

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
Bar bar;

void collision_process(Point a, Point b, Moving_ball& moving_ball)
{
	Vector u, w, n, ab(b.x - a.x, b.y - a.y);
	w.change(-moving_ball.v.x, -moving_ball.v.y);
	n.change(ab.y / ab.size(), -ab.x / ab.size());
	u.x = 2 * w.dot_product(n) * n.x - w.x;
	u.y = 2 * w.dot_product(n) * n.y - w.y;
	moving_ball.v.change(u);
	PlaySound("sound.wav", 0, SND_FILENAME | SND_ASYNC);
}

class Rect
{
public:
	Rect() : brick_width(98), brick_height(20), clear_flag(0), stop(0), brick_num(150)
	{
		std::fill(&brick[0][0], &brick[14][10], 1);

		wall[0].change(5, 0);					// 좌하
		wall[1].change(5, height - 5);			// 좌상
		wall[2].change(width - 5, height - 5);	// 우상
		wall[3].change(width - 5, 0);			// 우하
	}
	void field();
	void draw_brick();
	void collision_wall();
	void collision_brick();
	void clear();
	void reset();
	Point wall[4];
	int brick_width, brick_height, brick_num;
	bool brick[15][10], clear_flag, stop;
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
void Rect::draw_brick()
{
	for (int i = 0; i < 15; i++)
		for (int j = 0; j < 10; j++)
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
	Point a, b, tem;
	for (int i = 0; i < 20; i++)
	{
		if (brick[14][i] == 1) // 맨 아래쪽 벽돌 충돌 체크
		{
			a.change(10 + i * brick_width, height - 15 * brick_height - 10);
			b.change(10 + (i + 1) * brick_width, height - 15 * brick_height - 10);
			if (ball.pos.point_to_segment(a, b) <= ball.radius)
			{
				brick[14][i] = 0;
				tem.change(ball.pos.collision_point(a, b));
				if (b.equal(tem))
				{
					brick[14][i + 1] = 0;
					ball.v.y *= -1;
				}
				else collision_process(a, b, ball);
				ball.pos.y = ball.pos.collision_point(a, b).y - ball.radius;
				brick_num--;
				return;
			}
		}
	}
	for (int i = 0; i < 15; i++)
		for (int j = 0; j < 20; j++)
		{
			if (brick[i][j] == 0)
			{
				if (j != 0 && brick[i][j - 1] == 1) // 왼쪽벽
				{
					a.change(10 + j * brick_width, height - i * brick_height - 10);
					b.change(10 + j * brick_width, height - (i + 1) * brick_height - 10);
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick[i][j - 1] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick[i + 1][j - 1] == 1)
						{
							brick[i + 1][j - 1] = 0;
							ball.v.x *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.x = ball.pos.collision_point(a, b).x + ball.radius;
						brick_num--;
						return;
					}
				}
				if (j != 19 && brick[i][j + 1] == 1) // 오른벽
				{
					a.change(10 + (j + 1) * brick_width, height - i * brick_height - 10);
					b.change(10 + (j + 1) * brick_width, height - (i + 1) * brick_height - 10);
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick[i][j + 1] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick[i + 1][j + 1] == 1)
						{
							brick[i + 1][j + 1] = 0;
							ball.v.x *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.x = ball.pos.collision_point(a, b).x - ball.radius;
						brick_num--;
						return;
					}
				}
				if (i != 0 && brick[i - 1][j] == 1) // 위쪽벽
				{
					a.change(10 + j * brick_width, height - i * brick_height - 10);
					b.change(10 + (j + 1) * brick_width, height - i * brick_height - 10);
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick[i - 1][j] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick[i - 1][j + 1] == 1)
						{
							brick[i - 1][j + 1] = 0;
							ball.v.y *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.collision_point(a, b).y - ball.radius;
						brick_num--;
						return;
					}
				}
				if (i != 14 && brick[i + 1][j] == 1) // 아래쪽벽
				{
					a.change(10 + j * brick_width, height - (i + 1) * brick_height - 10);
					b.change(10 + (j + 1) * brick_width, height - (i + 1) * brick_height - 10);
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick[i + 1][j] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick[i + 1][j + 1] == 1)
						{
							brick[i + 1][j + 1] = 0;
							ball.v.y *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.y = ball.pos.collision_point(a, b).y + ball.radius;
						brick_num--;
						return;
					}
				}
			}
		}
}
void Rect::clear()
{
	if (!brick_num)
	{
		std::string time_print, time_min, time_sec;
		ball.ball_speed(0, 0);

		if (!stop)
		{
			time(&end);
			clear_time = (int)(end - start);
			stop = 1;
		}

		time_min = clear_time / 60;
		time_sec = clear_time % 60;

		if (clear_time / 60 < 10)
			time_min = "0" + std::to_string(clear_time / 60);
		else
			time_min = std::to_string(clear_time / 60);
		if (clear_time % 60 < 10)
			time_sec = "0" + std::to_string(clear_time % 60);
		else
			time_sec = std::to_string(clear_time % 60);
		time_print = "clear time : " + time_min + " : " + time_sec;

		const unsigned char* t = (unsigned char*)time_print.c_str();
		glColor3f(1.0f, 1.0f, 1.0f);
		glRasterPos2i(width + 10, height / 3 * 2);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, t);

		const unsigned char* message = reinterpret_cast<const unsigned char*>("clear");
		glColor3f(1.0f, 1.0f, 1.0f);
		glRasterPos2i(width / 2, height / 2);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, message);
	}
}
void Rect::reset()
{
	std::fill(&brick[0][0], &brick[14][10], 1);
	stop = 0;
	brick_num = 150;
}

class Rhombus
{
public:
	Rhombus() : brick_width(80), brick_height(40), clear_flag(0), stop(0), brick_num(36)
	{
		std::fill(&brick[0][0], &brick[5][6], 1);

		wall[0].change(0, height / 2);
		wall[1].change(width / 2, height);
		wall[2].change(width, height / 2);
		wall[3].change(width / 2, 0);
	}
	void field();
	void draw_brick();
	void collision_wall();
	void collision_brick();
	void clear();
	void reset();
	Point wall[4];
	int brick_width, brick_height, brick_num;
	bool brick[6][6], clear_flag, stop;
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
void Rhombus::draw_brick()
{
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
		{
			if (brick[i][j])
			{
				glBegin(GL_LINE_LOOP);
				glVertex2f(5 + width / 4 + j * brick_width, height - i * brick_height - height / 4);
				glVertex2f(5 + width / 4 + (j + 1) * brick_width, height - i * brick_height - height / 4);
				glVertex2f(5 + width / 4 + (j + 1) * brick_width, height - (i + 1) * brick_height - height / 4);
				glVertex2f(5 + width / 4 + j * brick_width, height - (i + 1) * brick_height - height / 4);
				glEnd();
			}
		}
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
void Rhombus::collision_brick()		// 두벽 충돌체크, 모서리 체크 해야함
{
	Point a, b, tem;
	for (int i = 0; i < 6; i++)
	{
		if (brick[5][i] == 1) // 맨 아래쪽 벽돌 충돌 체크
		{
			a.change(5 + width / 4 + i * brick_width, height - 6 * brick_height - height / 4);
			b.change(5 + width / 4 + (i + 1) * brick_width, height - 6 * brick_height - height / 4);
			if (ball.pos.point_to_segment(a, b) <= ball.radius)
			{
				brick[5][i] = 0;
				tem.change(ball.pos.collision_point(a, b));
				if (b.equal(tem))
				{
					brick[5][i + 1] = 0;
					ball.v.y *= -1;
				}
				else collision_process(a, b, ball);
				ball.pos.y = ball.pos.collision_point(a, b).y - ball.radius;
				brick_num--;
				return;
			}
		}
		if (brick[0][i] == 1) // 맨 위쪽 벽돌 충돌 체크
		{
			a.change(5 + width / 4 + i * brick_width, height - height / 4);
			b.change(5 + width / 4 + (i + 1) * brick_width, height - height / 4);
			if (ball.pos.point_to_segment(a, b) <= ball.radius)
			{
				brick[0][i] = 0;
				tem.change(ball.pos.collision_point(a, b));
				if (b.equal(tem))
				{
					brick[0][i + 1] = 0;
					ball.v.y *= -1;
				}
				else collision_process(a, b, ball);
				ball.pos.y = ball.pos.collision_point(a, b).y + ball.radius;
				brick_num--;
				return;
			}
		}
		if (brick[i][0] == 1) // 맨 왼쪽 벽돌 충돌 체크
		{
			a.change(5 + width / 4, height - i * brick_height - height / 4);
			b.change(5 + width / 4, height - (i + 1) * brick_height - height / 4);
			if (ball.pos.point_to_segment(a, b) <= ball.radius)
			{
				brick[i][0] = 0;
				tem.change(ball.pos.collision_point(a, b));
				if (b.equal(tem))
				{
					brick[i + 1][0] = 0;
					ball.v.x *= -1;
				}
				else collision_process(a, b, ball);
				ball.pos.x = ball.pos.collision_point(a, b).x - ball.radius;
				brick_num--;
				return;
			}
		}
		if (brick[i][5] == 1) // 맨 오른쪽 벽돌 충돌 체크
		{
			a.change(5 + width / 4 + 6 * brick_width, height - i * brick_height - height / 4);
			b.change(5 + width / 4 + 6 * brick_width, height - (i + 1) * brick_height - height / 4);
			if (ball.pos.point_to_segment(a, b) <= ball.radius)
			{
				brick[i][5] = 0;
				tem.change(ball.pos.collision_point(a, b));
				if (b.equal(tem))
				{
					brick[i + 1][5] = 0;
					ball.v.x *= -1;
				}
				else collision_process(a, b, ball);
				ball.pos.x = ball.pos.collision_point(a, b).x + ball.radius;
				brick_num--;
				return;
			}
		}
	}

	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
		{
			if (brick[i][j] == 0)
			{
				if (j != 0 && brick[i][j - 1] == 1) // 왼쪽벽
				{
					a.change(5 + width / 4 + j * brick_width, height - i * brick_height - height / 4);
					b.change(5 + width / 4 + j * brick_width, height - (i + 1) * brick_height - height / 4);
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick[i][j - 1] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick[i + 1][j - 1] == 1)
						{
							brick[i + 1][j - 1] = 0;
							ball.v.x *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.x = ball.pos.collision_point(a, b).x + ball.radius;
						brick_num--;
						return;
					}
				}
				if (j != 5 && brick[i][j + 1] == 1) // 오른벽
				{
					a.change(5 + width / 4 + (j + 1) * brick_width, height - i * brick_height - height / 4);
					b.change(5 + width / 4 + (j + 1) * brick_width, height - (i + 1) * brick_height - height / 4);
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick[i][j + 1] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick[i + 1][j + 1] == 1)
						{
							brick[i + 1][j + 1] = 0;
							ball.v.x *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.x = ball.pos.collision_point(a, b).x - ball.radius;
						brick_num--;
						return;
					}
				}
				if (i != 0 && brick[i - 1][j] == 1) // 위쪽벽
				{
					a.change(5 + width / 4 + j * brick_width, height - i * brick_height - height / 4);
					b.change(5 + width / 4 + (j + 1) * brick_width, height - i * brick_height - height / 4);
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick[i - 1][j] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick[i - 1][j + 1] == 1)
						{
							brick[i - 1][j + 1] = 0;
							ball.v.y *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.collision_point(a, b).y - ball.radius;
						brick_num--;
						return;
					}
				}
				if (i != 5 && brick[i + 1][j] == 1) // 아래쪽벽
				{
					a.change(5 + width / 4 + j * brick_width, height - (i + 1) * brick_height - height / 4);
					b.change(5 + width / 4 + (j + 1) * brick_width, height - (i + 1) * brick_height - height / 4);
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick[i + 1][j] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick[i + 1][j + 1] == 1)
						{
							brick[i + 1][j + 1] = 0;
							ball.v.y *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.y = ball.pos.collision_point(a, b).y + ball.radius;
						brick_num--;
						return;
					}
				}
			}
		}
}
void Rhombus::clear()
{
	if (!brick_num)
	{
		std::string time_print, time_min, time_sec;
		ball.ball_speed(0, 0);

		if (!stop)
		{
			time(&end);
			clear_time = (int)(end - start);
			stop = 1;
		}

		time_min = clear_time / 60;
		time_sec = clear_time % 60;

		if (clear_time / 60 < 10)
			time_min = "0" + std::to_string(clear_time / 60);
		else
			time_min = std::to_string(clear_time / 60);
		if (clear_time % 60 < 10)
			time_sec = "0" + std::to_string(clear_time % 60);
		else
			time_sec = std::to_string(clear_time % 60);
		time_print = "clear time : " + time_min + " : " + time_sec;

		const unsigned char* t = (unsigned char*)time_print.c_str();
		glColor3f(1.0f, 1.0f, 1.0f);
		glRasterPos2i(width + 10, height / 3 * 2);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, t);

		const unsigned char* message = reinterpret_cast<const unsigned char*>("clear");
		glColor3f(1.0f, 1.0f, 1.0f);
		glRasterPos2i(width / 2, height / 2);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, message);
	}
}
void Rhombus::reset()
{
	std::fill(&brick[0][0], &brick[5][6], 1);
	stop = 0;
	brick_num = 36;
}

class Circle
{
public:
	Circle() : mid(width / 2, height / 2), radius(300), brick_width(80), brick_height(40), clear_flag(0), stop(0), brick_num(36)
	{
		std::fill(&brick[0][0], &brick[5][6], 1);
	}
	void field();
	void draw_brick();
	void collision_wall();
	void collision_brick();
	void clear();
	void reset();
	Point mid;
	float radius;
	int brick_width, brick_height, brick_num;
	bool brick[6][6], clear_flag, stop;
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
void Circle::draw_brick()
{
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
		{
			if (brick[i][j])
			{
				glBegin(GL_LINE_LOOP);
				glVertex2f(5 + width / 4 + j * brick_width, height - i * brick_height - height / 4);
				glVertex2f(5 + width / 4 + (j + 1) * brick_width, height - i * brick_height - height / 4);
				glVertex2f(5 + width / 4 + (j + 1) * brick_width, height - (i + 1) * brick_height - height / 4);
				glVertex2f(5 + width / 4 + j * brick_width, height - (i + 1) * brick_height - height / 4);
				glEnd();
			}
		}
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
void Circle::collision_brick()		// 두벽 충돌체크, 모서리 체크 해야함
{
	Point a, b, tem;
	for (int i = 0; i < 6; i++)
	{
		if (brick[5][i] == 1) // 맨 아래쪽 벽돌 충돌 체크
		{
			a.change(5 + width / 4 + i * brick_width, height - 6 * brick_height - height / 4);
			b.change(5 + width / 4 + (i + 1) * brick_width, height - 6 * brick_height - height / 4);
			if (ball.pos.point_to_segment(a, b) <= ball.radius)
			{
				brick[5][i] = 0;
				tem.change(ball.pos.collision_point(a, b));
				if (b.equal(tem))
				{
					brick[5][i + 1] = 0;
					ball.v.y *= -1;
				}
				else collision_process(a, b, ball);
				ball.pos.y = ball.pos.collision_point(a, b).y - ball.radius;
				brick_num--;
				return;
			}
		}
		if (brick[0][i] == 1) // 맨 위쪽 벽돌 충돌 체크
		{
			a.change(5 + width / 4 + i * brick_width, height - height / 4);
			b.change(5 + width / 4 + (i + 1) * brick_width, height - height / 4);
			if (ball.pos.point_to_segment(a, b) <= ball.radius)
			{
				brick[0][i] = 0;
				tem.change(ball.pos.collision_point(a, b));
				if (b.equal(tem))
				{
					brick[0][i + 1] = 0;
					ball.v.y *= -1;
				}
				else collision_process(a, b, ball);
				ball.pos.y = ball.pos.collision_point(a, b).y + ball.radius;
				brick_num--;
				return;
			}
		}
		if (brick[i][0] == 1) // 맨 왼쪽 벽돌 충돌 체크
		{
			a.change(5 + width / 4, height - i * brick_height - height / 4);
			b.change(5 + width / 4, height - (i + 1) * brick_height - height / 4);
			if (ball.pos.point_to_segment(a, b) <= ball.radius)
			{
				brick[i][0] = 0;
				tem.change(ball.pos.collision_point(a, b));
				if (b.equal(tem))
				{
					brick[i + 1][0] = 0;
					ball.v.x *= -1;
				}
				else collision_process(a, b, ball);
				ball.pos.x = ball.pos.collision_point(a, b).x - ball.radius;
				brick_num--;
				return;
			}
		}
		if (brick[i][5] == 1) // 맨 오른쪽 벽돌 충돌 체크
		{
			a.change(5 + width / 4 + 6 * brick_width, height - i * brick_height - height / 4);
			b.change(5 + width / 4 + 6 * brick_width, height - (i + 1) * brick_height - height / 4);
			if (ball.pos.point_to_segment(a, b) <= ball.radius)
			{
				brick[i][5] = 0;
				tem.change(ball.pos.collision_point(a, b));
				if (b.equal(tem))
				{
					brick[i + 1][5] = 0;
					ball.v.x *= -1;
				}
				else collision_process(a, b, ball);
				ball.pos.x = ball.pos.collision_point(a, b).x + ball.radius;
				brick_num--;
				return;
			}
		}
	}

	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
		{
			if (brick[i][j] == 0)
			{
				if (j != 0 && brick[i][j - 1] == 1) // 왼쪽벽
				{
					a.change(5 + width / 4 + j * brick_width, height - i * brick_height - height / 4);
					b.change(5 + width / 4 + j * brick_width, height - (i + 1) * brick_height - height / 4);
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick[i][j - 1] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick[i + 1][j - 1] == 1)
						{
							brick[i + 1][j - 1] = 0;
							ball.v.x *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.x = ball.pos.collision_point(a, b).x + ball.radius;
						brick_num--;
						return;
					}
				}
				if (j != 5 && brick[i][j + 1] == 1) // 오른벽
				{
					a.change(5 + width / 4 + (j + 1) * brick_width, height - i * brick_height - height / 4);
					b.change(5 + width / 4 + (j + 1) * brick_width, height - (i + 1) * brick_height - height / 4);
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick[i][j + 1] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick[i + 1][j + 1] == 1)
						{
							brick[i + 1][j + 1] = 0;
							ball.v.x *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.x = ball.pos.collision_point(a, b).x - ball.radius;
						brick_num--;
						return;
					}
				}
				if (i != 0 && brick[i - 1][j] == 1) // 위쪽벽
				{
					a.change(5 + width / 4 + j * brick_width, height - i * brick_height - height / 4);
					b.change(5 + width / 4 + (j + 1) * brick_width, height - i * brick_height - height / 4);
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick[i - 1][j] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick[i - 1][j + 1] == 1)
						{
							brick[i - 1][j + 1] = 0;
							ball.v.y *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.collision_point(a, b).y - ball.radius;
						brick_num--;
						return;
					}
				}
				if (i != 5 && brick[i + 1][j] == 1) // 아래쪽벽
				{
					a.change(5 + width / 4 + j * brick_width, height - (i + 1) * brick_height - height / 4);
					b.change(5 + width / 4 + (j + 1) * brick_width, height - (i + 1) * brick_height - height / 4);
					if (ball.pos.point_to_segment(a, b) <= ball.radius)
					{
						brick[i + 1][j] = 0;
						tem.change(ball.pos.collision_point(a, b));
						if (b.equal(tem) && brick[i + 1][j + 1] == 1)
						{
							brick[i + 1][j + 1] = 0;
							ball.v.y *= -1;
						}
						else collision_process(a, b, ball);
						ball.pos.y = ball.pos.collision_point(a, b).y + ball.radius;
						brick_num--;
						return;
					}
				}
			}
		}
}
void Circle::clear()
{
	if (!brick_num)
	{
		std::string time_print, time_min, time_sec;
		ball.ball_speed(0, 0);

		if (!stop)
		{
			time(&end);
			clear_time = (int)(end - start);
			stop = 1;
		}

		time_min = clear_time / 60;
		time_sec = clear_time % 60;

		if (clear_time / 60 < 10)
			time_min = "0" + std::to_string(clear_time / 60);
		else
			time_min = std::to_string(clear_time / 60);
		if (clear_time % 60 < 10)
			time_sec = "0" + std::to_string(clear_time % 60);
		else
			time_sec = std::to_string(clear_time % 60);
		time_print = "clear time : " + time_min + " : " + time_sec;

		const unsigned char* t = (unsigned char*)time_print.c_str();
		glColor3f(1.0f, 1.0f, 1.0f);
		glRasterPos2i(width + 10, height / 3 * 2);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, t);

		const unsigned char* message = reinterpret_cast<const unsigned char*>("clear");
		glColor3f(1.0f, 1.0f, 1.0f);
		glRasterPos2i(width / 2, height / 2);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, message);
	}
}
void Circle::reset()
{
	std::fill(&brick[0][0], &brick[5][6], 1);
	stop = 0;
	brick_num = 36;
}

class Field
{
public:
	Field() : num(0), brick_width(50), brick_height(35), stage(1), x(0), y(0),
		zero(0), brick_num(1), bar_flag(1)
	{
		std::fill(&brick[0][0], &brick[14][20], 0);
		wall[0].change(0, 0);
	}
	void draw();
	void draw_brick();
	void collision_wall(Moving_ball& ball);
	void collision_brick();
	void stage_mode();
	void reset();

	Point wall[100];
	int num, brick_width, brick_height, brick_num, stage, x, y;
	bool brick[15][20], zero, bar_flag;
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
void Field::draw_brick()
{
	for (int i = 0; i < 15; i++)
		for (int j = 0; j < 20; j++)
		{
			if (brick[i][j])
			{
				glBegin(GL_LINE_LOOP);
				glVertex2f(j * brick_width, height - i * brick_height);
				glVertex2f((j + 1) * brick_width, height - i * brick_height);
				glVertex2f((j + 1) * brick_width, height - (i + 1) * brick_height);
				glVertex2f(j * brick_width, height - (i + 1) * brick_height);
				glEnd();
			}
		}
}
void Field::collision_wall(Moving_ball& ball)
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
			PlaySound("sound.wav", 0, SND_FILENAME | SND_ASYNC);
			/*공이 벽에 먹히는 현상 처리 해야함*/
		}
	}
}
void Field::collision_brick()
{
	for (int i = 0; i < 15; i++)
		for (int j = 0; j < 20; j++)
			if (brick[i][j])
			{
				Point a, b, tem;
				a.change(j * brick_width, height - i * brick_height);
				b.change(j * brick_width, height - (i + 1) * brick_height);
				if (ball.pos.point_to_segment(a, b) <= ball.radius) // 왼쪽벽
				{
					brick[i][j] = 0;
					tem.change(ball.pos.collision_point(a, b));
					if (b.equal(tem) && brick[i + 1][j] == 1)
					{
						brick[i + 1][j] = 0;
						ball.v.x *= -1;
					}
					else collision_process(a, b, ball);
					ball.pos.x = ball.pos.collision_point(a, b).x - ball.radius;
					brick_num--;
					return;
				}

				a.change((j + 1) * brick_width, height - i * brick_height);
				b.change((j + 1) * brick_width, height - (i + 1) * brick_height);
				if (ball.pos.point_to_segment(a, b) <= ball.radius) // 오른쪽벽
				{
					brick[i][j] = 0;
					tem.change(ball.pos.collision_point(a, b));
					if (b.equal(tem) && brick[i + 1][j] == 1)
					{
						brick[i + 1][j] = 0;
						ball.v.x *= -1;
					}
					else collision_process(a, b, ball);
					ball.pos.x = ball.pos.collision_point(a, b).x + ball.radius;
					brick_num--;
					return;
				}

				a.change(j * brick_width, height - i * brick_height);
				b.change((j + 1) * brick_width, height - i * brick_height);
				if (ball.pos.point_to_segment(a, b) <= ball.radius) // 위쪽벽
				{
					brick[i][j] = 0;
					tem.change(ball.pos.collision_point(a, b));
					if (b.equal(tem) && brick[i][j + 1] == 1)
					{
						brick[i][j + 1] = 0;
						ball.v.y *= -1;
					}
					else collision_process(a, b, ball);
					ball.pos.y = ball.pos.collision_point(a, b).y + ball.radius;
					brick_num--;
					return;
				}

				a.change(j * brick_width, height - (i + 1) * brick_height);
				b.change((j + 1) * brick_width, height - (i + 1) * brick_height);
				if (ball.pos.point_to_segment(a, b) <= ball.radius) // 아래쪽벽
				{
					brick[i][j] = 0;
					tem.change(ball.pos.collision_point(a, b));
					if (b.equal(tem) && brick[i][j + 1] == 1)
					{
						brick[i][j + 1] = 0;
						ball.v.y *= -1;
					}
					else collision_process(a, b, ball);
					ball.pos.y = ball.pos.collision_point(a, b).y - ball.radius;
					brick_num--;
					return;
				}
			}
}
void Field::stage_mode()
{
	if (!zero)
	{
		for (int i = 0; i < stage; i++)
		{
			x = rand() % 20;
			y = rand() % 15;
			brick[y][x] = 1;
		}
		zero = 1;
	}

	if (brick_num == 0)
	{
		std::fill(&brick[0][0], &brick[14][20], 0);
		stage++;
		brick_num = stage;
		bar_flag = 1;
		zero = 0;
		num = 0;
		bar.pos.x = width / 2; bar.pos.y = height / 5;
		ball.pos.x = bar.pos.x; ball.pos.y = bar.pos.y + ball.radius;
		ball.ball_speed(0.125, 0.075);
		if (game_mode == 3 || game_mode == 4 || game_mode == 6)
			bar.zelda_mode(ball);
	}
}
void Field::reset()
{
	stage = 1; zero = 0; bar_flag = 1; brick_num = 1;
	std::fill(&brick[0][0], &brick[14][20], 0);
}
Field field;

class Couple
{
public:
	Couple() : brick_width(97), brick_height(50), p1(0), p2(0), stop(0)
	{
		brick_num[0] = brick_num[1] = 30;

		bar[0].pos.x = width / 4; bar[0].pos.y = height / 5; bar[0].bar_width = 100;
		bar[1].pos.x = width / 4 * 3; bar[1].pos.y = height / 5; bar[1].bar_width = 100;

		ball[0].pos.x = width / 4; ball[0].pos.y = height / 5 + ball[0].radius;
		ball[1].pos.x = width / 4 * 3; ball[1].pos.y = height / 5 + ball[0].radius;

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
	void clear();
	void reset();

	Bar bar[2];
	Moving_ball ball[2];
	Point wall[6];
	int brick_width, brick_height, brick_num[2];
	bool brick[6][5][2];
	bool p1, p2, stop;
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
					brick_num[k]--;
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
							brick_num[k]--;
							return;
						}
					}
					if (j != 4 && brick[i][j + 1][k] == 1) // 오른벽
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
							brick_num[k]--;
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
							brick_num[k]--;
							return;
						}
					}
					if (i != 5 && brick[i + 1][j][k] == 1) // 아래쪽벽
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
							brick_num[k]--;
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
void Couple::clear()
{
	if (!brick_num[0] || !brick_num[1])
	{
		std::string time_print, time_min, time_sec;
		ball[0].ball_speed(0, 0);
		ball[1].ball_speed(0, 0);

		if (!stop)
		{
			time(&end);
			clear_time = (int)(end - start);
			stop = 1;
		}

		time_min = clear_time / 60;
		time_sec = clear_time % 60;

		if (clear_time / 60 < 10)
			time_min = "0" + std::to_string(clear_time / 60);
		else
			time_min = std::to_string(clear_time / 60);
		if (clear_time % 60 < 10)
			time_sec = "0" + std::to_string(clear_time % 60);
		else
			time_sec = std::to_string(clear_time % 60);
		time_print = "clear time : " + time_min + " : " + time_sec;

		const unsigned char* t = (unsigned char*)time_print.c_str();
		glColor3f(1.0f, 1.0f, 1.0f);
		glRasterPos2i(width + 10, height / 3 * 2);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, t);

		if (!brick_num[0])
		{
			const unsigned char* message = reinterpret_cast<const unsigned char*>("p1 win");
			glColor3f(1.0f, 1.0f, 1.0f);
			glRasterPos2i(width / 4, height / 2);
			glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, message);
		}
		else
		{
			const unsigned char* message = reinterpret_cast<const unsigned char*>("p2 win");
			glColor3f(1.0f, 1.0f, 1.0f);
			glRasterPos2i(width / 4 * 3, height / 2);
			glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, message);
		}
	}
}
void Couple::reset()
{
	bar[0].pos.x = width / 4; bar[0].pos.y = height / 5;
	bar[1].pos.x = width / 4 * 3; bar[1].pos.y = height / 5;
	ball[0].pos.x = width / 4; ball[0].pos.y = height / 5 + ball[0].radius;
	ball[1].pos.x = width / 4 * 3; ball[1].pos.y = height / 5 + ball[0].radius;
	std::fill(&brick[0][0][0], &brick[5][4][2], 1);
	stop = 0;
	brick_num[0] = 30;
	brick_num[1] = 30;
}

Couple couple;
Rect rect;
Rhombus rhombus;
Circle circle;

// 초기설정
void init(void)
{
	ball.v.x = 0.4;		ball.v.y = 0.3;
	bar.bar_width = 300;	bar.bar_height = 25;
	ball.radius = 10;
	field_shape = 1;		game_mode = 1;
	couple_game_mode = 1;
	bar.pos.x = width / 2; bar.pos.y = height / 5;
	ball.pos.x = bar.pos.x; ball.pos.y = bar.pos.y + ball.radius;

	couple.ball[0].pos.x = width / 4; couple.ball[0].pos.y = height / 4;
	couple.ball[1].pos.x = width / 4 * 3; couple.ball[1].pos.y = height / 4;
	couple.ball[0].ball_speed(0.125, 0.075); couple.ball[1].ball_speed(0.125, 0.075);
	couple.bar[0].bar_width = 100; couple.bar[1].bar_width = 100;
	//time(&start);	//time(&end);
	srand((unsigned int)time(NULL));
}

void main_screen()
{
	std::string game_start, game_exit, control_key, select;
	game_start = "start";
	const unsigned char* s = (unsigned char*)game_start.c_str();
	if(select_num == 0)
		glColor3f(1.0f, 0.0f, 0.0f);
	else
		glColor3f(1.0f, 1.0f, 1.0f);
	glRasterPos2i(width / 2, height / 2);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, s);

	control_key = "control key";
	const unsigned char* c = (unsigned char*)control_key.c_str();
	if (select_num == 1)
		glColor3f(1.0f, 0.0f, 0.0f);
	else
		glColor3f(1.0f, 1.0f, 1.0f);
	glRasterPos2i(width / 2, height / 2 - 50);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, c);

	game_exit = "exit";
	const unsigned char* e = (unsigned char*)game_exit.c_str();
	if (select_num == 2)
		glColor3f(1.0f, 0.0f, 0.0f);
	else
		glColor3f(1.0f, 1.0f, 1.0f);
	glRasterPos2i(width / 2, height / 2 - 100);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, e);

	select = "->";
	const unsigned char* t = (unsigned char*)select.c_str();
	glColor3f(1.0f, 1.0f, 1.0f);
	if(select_num == 0)
		glRasterPos2i(width / 2 - 40, height / 2);
	else if(select_num == 1)
		glRasterPos2i(width / 2 - 40, height / 2 - 50);
	else if(select_num == 2)
		glRasterPos2i(width / 2 - 40, height / 2 - 100);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, t);
}

void control_key()
{
	glColor3f(1.0f, 1.0f, 1.0f);
	const unsigned char* a = reinterpret_cast<const unsigned char*>(R"(field
0     2playr mode
1     rectangle
2     rhombus
3     circle
4     user)");
	glRasterPos2i(20, height / 10 * 8);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, a);

	const unsigned char* b = reinterpret_cast<const unsigned char*>(R"(game mode
F1     nomal
F2     nomal + draw
F3     zelda
F4     zelda + draw
F5     no bar + draw
F6     zelda + draw(limit)
F7     no bar + draw(limit)
)");
	glRasterPos2i(20, height / 10 * 5);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, b);

	const unsigned char* c = reinterpret_cast<const unsigned char*>(R"(       nomal mode
KEY_LEFT     <--bar   
KEY_RIGHT     bar-->   

       zelda mode
KEY_LEFT     <--bar   
KEY_RIGHT     bar-->   
KEY_UP     bar_up
KEY_DOWN     bar_down
space_bar     shoot

       control key
u     ball speed_up
d     ball speed_down
n     ball stop
r     reset
)");
	glRasterPos2i(400, height / 10 * 8);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, c);

	const unsigned char* d = reinterpret_cast<const unsigned char*>(R"(2player mode
     nomal mode
a     p1 <--bar
b     p1 bar-->

     zelda mode
a     p1 <--ball
d     p1 ball-->
w     p1 ball up
s     p1 ball down
b     creat wall
space_bar   shoot

     nomal mode
4     p2 <--bar
6     p2 bar-->

     zelda mode
4     p2 <--ball
6     p2 ball-->
8     p2 ball up
5     p2 ball down
'     creat wall
enter    shoot

)");
	glRasterPos2i(780, height - 20);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, d);
}

void game_over()
{
	if (ball.pos.y <= 0 || (field_shape == 4 &&(ball.pos.x < 0 || ball.pos.x > width || ball.pos.y > height)))
	{
		main_num = 0;
		flag = 2;
		std::string stage_print, time_print, time_min, time_sec;

		glColor3f(1.0f, 1.0f, 1.0f);
		const unsigned char* a = reinterpret_cast<const unsigned char*>("     Game Over");
		glRasterPos2i(width / 2, height / 2 + 100);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, a);

		if (field_shape == 4)
		{
			stage_print = "stage : " + std::to_string(field.stage);
			const unsigned char* s = (unsigned char*)stage_print.c_str();
			glRasterPos2i(width / 2, height / 2 + 50);
			glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, s);
		}
		if (current_time / 60 < 10)
			time_min = "0" + std::to_string(current_time / 60);
		else
			time_min = std::to_string(current_time / 60);
		if (current_time % 60 < 10)
			time_sec = "0" + std::to_string(current_time % 60);
		else
			time_sec = std::to_string(current_time % 60);
		time_print = "time : " + time_min + " : " + time_sec;

		const unsigned char* t = (unsigned char*)time_print.c_str();
		glRasterPos2i(width / 2, height / 2);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, t);

		const unsigned char* p = reinterpret_cast<const unsigned char*>("Press r key to restart...");
		glRasterPos2i(width / 2, height / 2 - 50);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, p);
	}
}

void game_clear()
{
	if (!rect.brick_num || !rhombus.brick_num || !circle.brick_num || field.stage == 50)
	{
		main_num = 0;
		flag = 2;
		std::string stage_print, time_print, time_min, time_sec;

		glColor3f(1.0f, 1.0f, 1.0f);
		const unsigned char* a = reinterpret_cast<const unsigned char*>("     Game Clear");
		glRasterPos2i(width / 2, height / 2 + 100);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, a);

		if (field_shape == 4 && field.stage == 50)
		{
			stage_print = "stage : 50";
			const unsigned char* s = (unsigned char*)stage_print.c_str();
			glRasterPos2i(width / 2, height / 2 + 50);
			glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, s);
		}
		if (current_time / 60 < 10)
			time_min = "0" + std::to_string(current_time / 60);
		else
			time_min = std::to_string(current_time / 60);
		if (current_time % 60 < 10)
			time_sec = "0" + std::to_string(current_time % 60);
		else
			time_sec = std::to_string(current_time % 60);
		time_print = "time : " + time_min + " : " + time_sec;

		const unsigned char* t = (unsigned char*)time_print.c_str();
		glRasterPos2i(width / 2, height / 2);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, t);

		const unsigned char* p = reinterpret_cast<const unsigned char*>("Press r key to restart...");
		glRasterPos2i(width / 2, height / 2 - 50);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, p);
	}
	else if (couple.ball[0].pos.y < 0 || couple.brick_num[0] == 0 || couple.ball[1].pos.y < 0
		|| couple.brick_num[1] == 0)
	{
		main_num = 0;
		flag = 2;
		std::string time_print, time_min, time_sec, winner;

		glColor3f(1.0f, 1.0f, 1.0f);
		const unsigned char* a = reinterpret_cast<const unsigned char*>("     Game Over");
		glRasterPos2i(width / 2, height / 2 + 100);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, a);

		winner = "winner : ";
		if (couple.ball[0].pos.y < 0 || couple.brick_num[1] == 0)
			winner += "player 2";
		else
			winner += "player 1";
		const unsigned char* s = (unsigned char*)winner.c_str();
		glRasterPos2i(width / 2, height / 2 + 50);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, s);

		if (current_time / 60 < 10)
			time_min = "0" + std::to_string(current_time / 60);
		else
			time_min = std::to_string(current_time / 60);
		if (current_time % 60 < 10)
			time_sec = "0" + std::to_string(current_time % 60);
		else
			time_sec = std::to_string(current_time % 60);
		time_print = "time : " + time_min + " : " + time_sec;

		const unsigned char* t = (unsigned char*)time_print.c_str();
		glRasterPos2i(width / 2, height / 2);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, t);

		const unsigned char* p = reinterpret_cast<const unsigned char*>("Press r key to restart...");
		glRasterPos2i(width / 2, height / 2 - 50);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, p);
	}

}

void loading()
{
	angle += 0.2;
	float	delta;
	glLineWidth(10);
	delta = 2 * PI / polygon_num;
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < polygon_num; i++)
		glVertex2f(width / 2 + 100 * cos(delta * i), height / 2 + 100 * sin(delta * i));
	glEnd();
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0);
	glTranslatef(width / 2, height / 2, 0.0);
	glRotatef(angle, 0.0, 0.0, 1.0);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 12; i++)
		glVertex2f(100 * cos(delta * i), 100 * sin(delta * i));
	glEnd();
	glPopMatrix();
	glLineWidth(1);

	glColor3f(1.0, 1.0, 1.0);
	const unsigned char* p = reinterpret_cast<const unsigned char*>("Loading...");
	glRasterPos2i(width / 2 - 50, height / 2 - 10);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, p);
}

void MyReshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width + 300, 0, height);
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
	std::string time_print, time_min, time_sec, bgm_name, game_mode_name, field_shape_name, stage_name;

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

	if (game_mode != 0)
	{
		switch (game_mode)
		{
		case 0: game_mode_name += "2player mode"; break;
		case 1: game_mode_name += "nomal"; break;
		case 2: game_mode_name += "nomal + draw"; break;
		case 3: game_mode_name += "zelda"; break;
		case 4: game_mode_name += "zelda + draw"; break;
		case 5: game_mode_name += "no bar + draw"; break;
		case 6: game_mode_name += "zelda + draw"; break;
		case 7: game_mode_name += "no bar + draw"; break;
		}
	}
	else
	{
		switch (couple_game_mode)
		{
		case 1: game_mode_name += "nomal"; break;
		case 2: game_mode_name += "nomal + draw"; break;
		case 3: game_mode_name += "zelda"; break;
		case 4: game_mode_name += "zelda + draw"; break;
		case 5: game_mode_name += "zelda + draw"; break;
		}
	}

	const unsigned char* mode_name = (unsigned char*)game_mode_name.c_str();
	glRasterPos2i(width + 10, height / 4);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, mode_name);

	field_shape_name = "field : ";

	switch (field_shape)
	{
	case 0: field_shape_name += "rectangle"; break;
	case 1: field_shape_name += "rectangle"; break;
	case 2: field_shape_name += "rhombus"; break;
	case 3: field_shape_name += "circle"; break;
	case 4: field_shape_name += "nothing"; break;
	}

	const unsigned char* map_name = (unsigned char*)field_shape_name.c_str();
	glRasterPos2i(width + 10, height / 5);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, map_name);

	if (field_shape == 4)
	{
		stage_name = "stage : " + std::to_string(field.stage);
		const unsigned char* s = (unsigned char*)stage_name.c_str();
		glRasterPos2i(width, height / 10 * 8);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, s);
	}
}

void field_select()
{
	switch (field_shape)
	{
	case 0:							// 커플
		couple.field();
		break;
	case 1:							// 사각형
		rect.field();
		rect.draw_brick();
		rect.collision_wall();
		rect.collision_brick();
		break;
	case 2:
		rhombus.field();			// 마름모
		rhombus.draw_brick();
		rhombus.collision_wall();
		rhombus.collision_brick();
		break;
	case 3:
		circle.field();				// 원
		circle.draw_brick();
		circle.collision_wall();
		circle.collision_brick();
		break;
	case 4:							// 사용자가 직접 만드는거
		field.stage_mode();
		field.draw_brick();
		field.collision_brick();
		break;
	default: break;
	}
}

void game_mode_select()
{
	switch (game_mode)
	{
	case 0:
		switch (couple_game_mode)
		{
		case 1:
			couple.bar[0].draw(); couple.bar[1].draw();
			couple.bar[0].collision_bar(couple.ball[0]);
			couple.bar[1].collision_bar(couple.ball[1]);
			break;
		case 2:
			couple.bar[0].draw(); couple.bar[1].draw();
			couple.bar[0].collision_bar(couple.ball[0]);
			couple.bar[1].collision_bar(couple.ball[1]);
			field.draw();
			field.collision_wall(couple.ball[0]);
			field.collision_wall(couple.ball[1]);
			break;
		case 3:
			couple.bar[0].draw(); couple.bar[1].draw();
			couple.bar[0].zelda_mode(couple.ball[0]);
			couple.bar[1].zelda_mode(couple.ball[1]);
			break;
		case 4:
			couple.bar[0].draw(); couple.bar[1].draw();
			couple.bar[0].zelda_mode(couple.ball[0]);
			couple.bar[1].zelda_mode(couple.ball[1]);
			field.draw();
			field.collision_wall(couple.ball[0]);
			field.collision_wall(couple.ball[1]);
			break;
		case 5:
			couple.bar[0].draw(); couple.bar[1].draw();
			couple.bar[0].zelda_mode(couple.ball[0]);
			couple.bar[1].zelda_mode(couple.ball[1]);
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
	case 1:
		bar.draw();
		bar.collision_bar(ball);
		break;
	case 2:
		bar.draw();
		bar.collision_bar(ball);
		field.draw();
		field.collision_wall(ball);
		break;
	case 3:
		bar.draw();
		bar.zelda_mode(ball);
		break;
	case 4:
		bar.draw();
		bar.zelda_mode(ball);
		field.draw();
		field.collision_wall(ball);
		break;
	case 5:
		field.draw();
		field.collision_wall(ball);
		break;
	case 6:
		field.draw();
		field.collision_wall(ball);
		if (field_shape != 4)
		{
			bar.draw();
			bar.zelda_mode(ball);
		}
		else
		{
			if (field.bar_flag)
			{
				bar.draw();
				bar.zelda_mode(ball);
			}
		}
		break;
	case 7:
		field.draw();
		field.collision_wall(ball);
		break;
	default: break;
	}
}

void moving_ball_draw()
{
	if (game_mode == 0)
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
		if (game_mode == 0)
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
	
	if (angle <= 360)
		loading();
	else
	{
		if (!main_num)
		{
			if (flag == 0)
				main_screen();
			else if (flag == 1)
				control_key();
		}
		else
		{
			move_ball();
			field_select();
			game_mode_select();
			moving_ball_draw();
			text();
		}
		game_over();
		game_clear();
	}
	glutSwapBuffers();
	glFlush();
}

void Reset() // 초기상태로 돌리기
{
	bar.pos.x = width / 2; bar.pos.y = height / 5;
	ball.pos.x = bar.pos.x; ball.pos.y = bar.pos.y + ball.radius;
	ball.ball_speed(0.125, 0.075);
	rect.reset();
	rhombus.reset();
	circle.reset();
	field.reset();
	clear_time = 0;
	if (game_mode == 3 || game_mode == 4 || game_mode == 6)
		bar.zelda_mode(ball);
	field.num = 0;
	time(&start);
}

void couple_reset()
{
	couple.ball[0].pos.x = width / 4; couple.ball[0].pos.y = height / 4;
	couple.ball[1].pos.x = width / 4 * 3; couple.ball[1].pos.y = height / 4;
	couple.ball[0].ball_speed(0.125, 0.075); couple.ball[1].ball_speed(0.125, 0.075);
	std::fill(&couple.brick[0][0][0], &couple.brick[5][4][2], 1);
	couple.reset();
	clear_time = 0;
	if (couple_game_mode == 3 || couple_game_mode == 4 || couple_game_mode == 5)
		bar.zelda_mode(ball);
	field.num = 0;
	time(&start);
}

void clear_brick()
{
	if (field_shape == 0)
	{
		std::fill(&couple.brick[0][0][0], &couple.brick[5][4][2], 0);
		couple.brick_num[0] = couple.brick_num[1] = 0;
	}
	else if (field_shape == 1)
	{
		std::fill(&rect.brick[0][0], &rect.brick[14][10], 0);
		rect.brick_num = 0;
	}
	else if (field_shape == 2)
	{
		std::fill(&rhombus.brick[0][0], &rhombus.brick[5][6], 0);
		rhombus.brick_num = 0;
	}
	else if (field_shape == 3)
	{
		std::fill(&circle.brick[0][0], &circle.brick[5][6], 0);
		circle.brick_num = 0;
	}
	else if (field_shape == 4)
	{
		std::fill(&field.brick[0][0], &field.brick[14][20], 0);
		field.brick_num = 0;
	}
}

void MyKey(unsigned char key, int x, int y)
{
	if (!main_num)
	{
		switch (key)
		{
		case 13:
			if (select_num == 0)
			{
				main_num = 1;
				time(&start);
			}
			else if (select_num == 1)
			{
				flag = 1;
				time(&start);
			}
			else
				exit(0);
			break;
		case 32:
			if (select_num == 0)
				main_num = 1;
			else if (select_num == 1)
				flag = 1;
			else
				exit(0);
			break;
		case 27:
			main_num = 0;
			select_num = 0;
			flag = 0;
			break;
		case 'r':
			main_num = 1;
			select_num = 0;
			flag = 0;
			Reset();
			couple_reset();
			break;
		}
	}
	else
	{
		if (game_mode == 0)
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
				case '0': field_shape = 1; game_mode = 1; couple_game_mode = 1; time(&start); break;
				case 'm': bgm(++bgm_mode); break;
				case 27:
					main_num = 0;
					select_num = 0;
					flag = 0;
					break;
				case 8: clear_brick(); break;
				}
			}
			else
			{
				switch (key)
				{
				case 'a': couple.bar[0].pos.x -= 30;			break;
				case 'd': couple.bar[0].pos.x += 30;			break;
				case 'r': couple_reset();						break;
				case 'u':
					couple.ball[0].v.x *= 1.1; couple.ball[0].v.y *= 1.1;
					couple.ball[1].v.x *= 1.1; couple.ball[1].v.y *= 1.1;
					break;
				case '\'': couple.p1 = !couple.p1;				break;
				case 'b': couple.p2 = !couple.p2;				break;
				case 'n':	n += 1;								break;			// n을 눌러서 공을 멈추기
				case '0': field_shape = 1; game_mode = 1; couple_game_mode = 1; time(&start); break;
				case 'm': bgm(++bgm_mode); break;
				case 27:
					main_num = 0;
					select_num = 0;
					flag = 0;
					break;
				case 8: clear_brick(); break;
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
			case '0': field_shape = 0; game_mode = 0; time(&start);
				break;
			case '1': field_shape = 1; time(&start);	// 사각형
				break;
			case '2': field_shape = 2; time(&start);	// 마름모
				break;
			case '3': field_shape = 3; time(&start);	// 원
				break;
			case '4': field_shape = 4; game_mode = 6;  time(&start);
				bar.pos.x = width / 2; bar.pos.y = height / 5;
				ball.pos.x = bar.pos.x; ball.pos.y = bar.pos.y + ball.radius;
				ball.ball_speed(0.125, 0.075);
				bar.zelda_mode(ball);
				break;
			case 32:														// 스페이스 바(2번, 3번 모드일때만 작동)
				if (bar.bar_move) break;
				if (field_shape == 4 && game_mode == 6)
					field.bar_flag = 0;
				ball.v.change(ball.tem_v);
				ball.tem_v.change(0, 0);
				bar.bar_move = true;
				break;
			case 'm': bgm(++bgm_mode); break;
			case 27: main_num = 0; break;
			case 8: clear_brick(); break;
			default: break;
			}
		}
	}
	glutPostRedisplay();
}

void SpecialKey(int key, int x, int y)
{
	if (!main_num)
	{
		if (0 <= select_num && select_num <= 2)
		{
			switch (key)
			{
			case GLUT_KEY_UP: if(select_num != 0) select_num--; break;
			case GLUT_KEY_DOWN: if (select_num != 2) select_num++; break;
			}
		}
	}
	else
	{
		if (game_mode != 0)
		{
			if (field_shape != 4)
			{
				if (bar.bar_move)
				{
					switch (key)
					{
					case GLUT_KEY_LEFT:		bar.pos.x -= 30;		break;
					case GLUT_KEY_RIGHT:	bar.pos.x += 30;		break;
					case GLUT_KEY_F1:		game_mode = 1;			break;
					case GLUT_KEY_F2:		game_mode = 2;			break;
					case GLUT_KEY_F3:		game_mode = 3;			break;
					case GLUT_KEY_F4:		game_mode = 4;			break;
					case GLUT_KEY_F5:		game_mode = 5;			break;
					case GLUT_KEY_F6:		game_mode = 6;
						bar.pos.x = width / 2; bar.pos.y = height / 5;
						ball.pos.x = bar.pos.x; ball.pos.y = bar.pos.y + ball.radius;
						ball.ball_speed(0.125, 0.075);
						break;
					case GLUT_KEY_F7:		game_mode = 7;
						bar.pos.x = width / 2; bar.pos.y = height / 5;
						ball.pos.x = bar.pos.x; ball.pos.y = bar.pos.y + ball.radius;
						ball.ball_speed(0.125, 0.075);
						break;
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
					case GLUT_KEY_F5:		game_mode = 5;			break;
					case GLUT_KEY_F6:		game_mode = 6;
						bar.pos.x = width / 2; bar.pos.y = height / 5;
						ball.pos.x = bar.pos.x; ball.pos.y = bar.pos.y + ball.radius;
						ball.ball_speed(0.125, 0.075);
						break;
					case GLUT_KEY_F7:		game_mode = 7;
						bar.pos.x = width / 2; bar.pos.y = height / 5;
						ball.pos.x = bar.pos.x; ball.pos.y = bar.pos.y + ball.radius;
						ball.ball_speed(0.125, 0.075);
						break;
					default: break;
					}
				}
			}
			else
			{
				if (bar.bar_move)
				{
					switch (key)
					{
					case GLUT_KEY_LEFT:		bar.pos.x -= 30;		break;
					case GLUT_KEY_RIGHT:	bar.pos.x += 30;		break;
					case GLUT_KEY_F6:		game_mode = 6;
						bar.pos.x = width / 2; bar.pos.y = height / 5;
						ball.pos.x = bar.pos.x; ball.pos.y = bar.pos.y + ball.radius;
						ball.ball_speed(0.125, 0.075);
						break;
					case GLUT_KEY_F7:		game_mode = 7;
						bar.pos.x = width / 2; bar.pos.y = height / 5;
						ball.pos.x = bar.pos.x; ball.pos.y = bar.pos.y + ball.radius;
						ball.ball_speed(0.125, 0.075);
						break;
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
					case GLUT_KEY_F6:		game_mode = 6;
						bar.pos.x = width / 2; bar.pos.y = height / 5;
						ball.pos.x = bar.pos.x; ball.pos.y = bar.pos.y + ball.radius;
						ball.ball_speed(0.125, 0.075);
						break;
					case GLUT_KEY_F7:		game_mode = 7;
						bar.pos.x = width / 2; bar.pos.y = height / 5;
						ball.pos.x = bar.pos.x; ball.pos.y = bar.pos.y + ball.radius;
						ball.ball_speed(0.125, 0.075);
						break;
					}
				}
			}
		}
		else
		{
			switch (key)
			{
			case GLUT_KEY_F1:		couple_game_mode = 1;			break;
			case GLUT_KEY_F2:		couple_game_mode = 2;			break;
			case GLUT_KEY_F3:		couple_game_mode = 3;			break;
			case GLUT_KEY_F4:		couple_game_mode = 4;			break;
			case GLUT_KEY_F5:		couple_game_mode = 5;			break;
			}
		}
	}
	glutPostRedisplay();
}

void MyMouse(int button, int state, int x, int y)
{
	if ((game_mode == 2 || game_mode == 4 || game_mode == 5 || game_mode == 6
		|| game_mode == 7 || couple_game_mode == 2 || couple_game_mode == 4 || couple_game_mode == 5) && x <= width)		// 3번, 4번 모드일때 맵을 그릴 수 있음
	{
		if (field_shape == 4 && field.num / 2 >= field.stage * 3) return;
		else if ((game_mode == 6 || game_mode == 7 || couple_game_mode == 5) && field.num / 2 > 4) return;

		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
			field.wall[field.num++].change(x, height - y);
		if (field.num % 2 == 1)
			if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
				field.wall[field.num++].change(x, height - y);
		if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
			field.num = 0;
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
}