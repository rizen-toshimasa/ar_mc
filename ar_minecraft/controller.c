
#include <stdlib.h>
#include "controller.h"
#include <gl/glut.h>

static CONTROLLER_DATA	g_cntl = { KEY_UP, KEY_UP, KEY_UP, KEY_UP,KEY_UP, KEY_UP, KEY_UP, KEY_UP,KEY_UP,KEY_UP,KEY_UP,KEY_UP ,KEY_UP,KEY_UP};


//===========================================================================
// �R���g���[���̏�Ԃ��擾����֐�
//===========================================================================
void GetControllerData( CONTROLLER_DATA *cntl )
{
	*cntl = g_cntl;
}



//===========================================================================
// �L�[�������������ꍇ�ɌĂ΂��֐�
//===========================================================================
void KeyDown( unsigned char key, int x, int y )
{
	switch ( key ) {
	case KEY_ESC:	exit(0);				break;	// ESC
	case KEY_W:		g_cntl.W = KEY_DOWN;	break;	
	case KEY_A:		g_cntl.A = KEY_DOWN;	break;	
	case KEY_S:		g_cntl.S = KEY_DOWN;	break;	
	case KEY_D:		g_cntl.D = KEY_DOWN;	break;	
	case KEY_SP:	g_cntl.sp = KEY_DOWN;	break;	
	case KEY_Z:		g_cntl.Z = KEY_DOWN;	break;
	case KEY_X:		g_cntl.X = KEY_DOWN;	break;
	case KEY_C:		g_cntl.C = KEY_DOWN;	break;
	case KEY_PLUS:	g_cntl.plus = KEY_DOWN;	break;
	case KEY_MINUS:	g_cntl.minus = KEY_DOWN;	break;
	default:	break;
	}
}


//===========================================================================
// �L�[�������ꂽ�ꍇ�ɌĂ΂��֐�
//===========================================================================
void KeyUp( unsigned char key, int x, int y )
{
	switch ( key ) {
	case KEY_W:		g_cntl.W = KEY_UP;		break;
	case KEY_A:		g_cntl.A = KEY_UP;		break;
	case KEY_S:		g_cntl.S = KEY_UP;		break;
	case KEY_D:		g_cntl.D = KEY_UP;		break;
	case KEY_SP:	g_cntl.sp = KEY_SP;	break;
	case KEY_Z:		g_cntl.Z = KEY_UP;		break;
	case KEY_X:		g_cntl.X = KEY_UP;		break;
	case KEY_C:		g_cntl.C = KEY_UP;		break;
	case KEY_PLUS:		g_cntl.plus = KEY_UP;		break;
	case KEY_MINUS:		g_cntl.minus = KEY_UP;		break;

	default:	break;
	}
}




//===========================================================================
// ����L�[�������ꂽ�ꍇ�ɌĂ΂��֐�
//===========================================================================
void SpecialKeyDown(int key, int x, int y)
{
	switch ( key ) {
	case GLUT_KEY_UP:		g_cntl.up	 = KEY_DOWN;	break;
	case GLUT_KEY_DOWN:		g_cntl.down  = KEY_DOWN;	break;
	case GLUT_KEY_LEFT:		g_cntl.left	 = KEY_DOWN;	break;
	case GLUT_KEY_RIGHT:	g_cntl.right = KEY_DOWN;	break;
	default:	break;
	}
}


//===========================================================================
// ����L�[�������ꂽ�ꍇ�ɌĂ΂��֐�
//===========================================================================
void SpecialKeyUp(int key, int x, int y)
{
	switch ( key ) {
	case GLUT_KEY_UP:		g_cntl.up	 = KEY_UP;	break;
	case GLUT_KEY_DOWN:		g_cntl.down  = KEY_UP;	break;
	case GLUT_KEY_LEFT:		g_cntl.left	 = KEY_UP;	break;
	case GLUT_KEY_RIGHT:	g_cntl.right = KEY_UP;	break;
	default:	break;
	}
}
