

#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__


#define KEY_DOWN	1
#define KEY_UP		0

#define KEY_ESC		27
#define KEY_W		'w'
#define KEY_A		'a'
#define KEY_S		's'
#define KEY_D		'd'
#define KEY_Z		'z'
#define KEY_SP		' '
#define KEY_X		'x'
#define KEY_C		'c'
#define KEY_PLUS	'+'
#define KEY_MINUS	'-'


// コントローラデータ構造体
typedef struct {
	int	up;
	int down;
	int left;
	int right;
	int W;
	int A;
	int S;
	int D;
	int sp;
	int Z;
	int X;
	int C;
	int plus;
	int minus;
} CONTROLLER_DATA;

void GetControllerData( CONTROLLER_DATA *cntl );
void KeyDown( unsigned char key, int x, int y );
void KeyUp( unsigned char key, int x, int y );
void SpecialKeyDown(int key, int x, int y);
void SpecialKeyUp(int key, int x, int y);

#endif