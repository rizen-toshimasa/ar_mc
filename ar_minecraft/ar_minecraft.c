#include <windows.h>
#include <stdio.h>
#include <locale.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <AR/ar.h>
#include <AR/param.h>
#include <AR/video.h>
#include <AR/gsub.h>


#include "controller.h"	// キーボードによる制御
#include "GLMetaseq.h"	// モデルローダ

//#include "DxLib/DxLib.h"
//#include "DxLib/DxDirectX.h"


// 画面消去の定数
#define CLS_SPACE ((WORD)(0x20))
#define CLS_COLOR ((WORD)(0x0F))

// ライブラリの設定
#ifdef _DEBUG
	#pragma comment(lib,"libARd.lib")
	#pragma comment(lib,"libARgsubd.lib")
	#pragma comment(lib,"libARvideod.lib")
	#pragma comment(linker,"/NODEFAULTLIB:libcmtd.lib")
#else
	#pragma comment(lib,"libAR.lib")
	#pragma comment(lib,"libARgsub.lib")
	#pragma comment(lib,"libARvideo.lib")
	#pragma comment(linker,"/NODEFAULTLIB:libcmt.lib")
#endif


// キャラクタデータ構造体
typedef struct {
	MQO_MODEL	model;		//静止モデル
	MQO_SEQUENCE seq;		//アニメモデル
	int			frame;		//アニメフレーム
	int			maxframe;	//アニメ最大フレーム数
	GLfloat		rot;		//角度
	GLfloat		x;			//x座標
	GLfloat		y;			//y座標
	GLfloat		z;			//z座標
} CHARACTER_DATA;

// グローバル変数
static CHARACTER_DATA	g_char =  { NULL, 0, 0, 0, 24, 0.0, 0.0, 0.0, 0.0 };	// 主人公キャラデータ
static CHARACTER_DATA	g_mob =   { NULL, 0, 0, 0,  0, 0.0, -50.0, 0.0, 0.0 };	// モブデータ
static CHARACTER_DATA	g_world = { NULL, 0, 0, 0,  0, 0.0, 0.0, 0.0, 0.0 };	// ワールドデータ
static CHARACTER_DATA	g_world2 = { NULL, 0, 0, 0,  0, 0.0, 0.0, 0.0, 0.0 };	// ワールドデータ
static char	*g_char_name		= "char/char.mqo";			// 主人公キャラファイル
static char	*g_charwalk_name	= "char/walk/charwalk_%06d.mqo";//主人公歩く

static char	*g_mob_name		= "mob/creeper.mqo";		// モブファイル

static char	*g_world_name	= "world/village.mqo";		// ワールドファイル
static char	*g_world_name1	= "world/world.mqo";		// ワールドファイル
static char	*g_world_name2	= "world/city.mqo";		// ワールドファイル


int isFirstDetect = 1;//ジッタ防止用処理の、初回フラグ

double g_char_size = 3;//主人公キャラ大きさ
double g_mob_size = 3;//モブキャラ大きさ
double g_world_size = 20;//ワールド大きさ
//double g_world_size = 0.1;//ワールド大きさ
double g_all_size = 0;//全体の大きさを変更する値

static char	*g_vconf_name	= "Data/WDM_camera_flipV.xml";	// ビデオデバイスの設定ファイル
static char	*g_cparam_name  = "Data/camera_para.dat";		// カメラパラメータファイル
static char	*g_patt_name    = "Data/patt.creeper";		// パターンファイル


static int		g_patt_id;							// パターンのID
static double	g_patt_trans[3][4];					// 座標変換行列
static double	g_patt_center[2]	= { 0.0, 0.0 };	// パターンの中心座標
static double	g_patt_width		= 80.0;			// パターンのサイズ（単位：mm）
static int		g_thresh			= 100;			// 2値化の閾値

static int		g_light_on  = TRUE;							// シェーディングのOn/Off
static int		g_optcmf_on = FALSE;						// 光学迷彩のOn/Off


// プロトタイプ宣言
static void MainLoop(void);
static void Cleanup(void);
static void DrawObject( ARUint8 *image );
static void CalcState(void);
static void SetLight( GLenum light );
static void OpticalCamouflage( unsigned char *image );
static void GetObjectDepth( GLfloat *depth, int width, int height, GLfloat *depth_min, GLfloat *depth_max );
static void DrawImage( unsigned char *color_buf, int width, int height );
static void menu( int value );
static void openSkin();
static void openMob();
static void cls( void );
static void PrintInfo(void);
static void play_music(int flg);

int music_flg = 0; //音楽の再生と停止用フラグ 

// ===========================================================================
// main関数
// ===========================================================================
//int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
//         LPSTR lpCmdLine, int nCmdShow ){
//		PlayMusic( "music/ha1.ogg" , DX_PLAYTYPE_LOOP );
//}
int main( int argc, char **argv )
{
	ARParam	cparam;			// カメラパラメータ
	ARParam	wparam;			// カメラパラメータ（作業用変数）
	int		xsize, ysize;	// 画像サイズ


	//音楽再生
	play_music(1);


	// GLUTの初期化
	glutInit( &argc, argv );

	// ビデオデバイスの設定
	if ( arVideoOpen( g_vconf_name ) < 0 ) {
		printf("ビデオデバイスのエラー");
		return -1;
	}
	// カメラパラメータの設定
	if ( arVideoInqSize( &xsize, &ysize ) < 0 ) {
		printf("画像サイズを取得できませんでした\n");
		return -1;
	}
	if ( arParamLoad( g_cparam_name, 1, &wparam ) < 0 ) {
		printf("カメラパラメータの読み込みに失敗しました\n");
		return -1;
	}
	arParamChangeSize( &wparam, xsize, ysize, &cparam );
	arInitCparam( &cparam );

	// パターンファイルのロード
	if ( (g_patt_id = arLoadPatt(g_patt_name)) < 0 ) {
		printf("パターンファイルの読み込みに失敗しました\n");
		return -1;
	}

	// ウィンドウの設定
	argInit( &cparam, 1.0, 0, 0, 0, 0 );
	glutSetWindowTitle("MineCraft AR Tool Kit");

	// GLMetaseqの初期化
	mqoInit();

	// モデルの読み込み
	if ( ( g_char.model = mqoCreateModel( g_char_name, g_char_size )) == NULL ) {
		printf("キャラの読み込みに失敗しました\n");
		//return -1;
	}
	g_char.seq = mqoCreateSequence( g_charwalk_name, g_char.maxframe ,g_char_size );

	if ( ( g_mob.model = mqoCreateModel( g_mob_name, g_mob_size )) == NULL ) {
		printf("モブの読み込みに失敗しました\n");
		return -1;
	}
	if ( ( g_world.model = mqoCreateModel( g_world_name, g_world_size )) == NULL ) {
		printf("ワールドの読み込みに失敗しました\n");
		return -1;
	}


	// ビデオキャプチャの開始
	arVideoCapStart();

	// 終了時に呼ぶ関数を指定
	atexit( Cleanup );

	// キーボード関数の登録
	glutKeyboardUpFunc( KeyUp );
	glutSpecialFunc( SpecialKeyDown );
	glutSpecialUpFunc( SpecialKeyUp );

	//コンソール上に情報を出す
	PrintInfo();


    glutCreateMenu( menu );
		glutAddMenuEntry( "スキンを開く", 1 );
		glutAddMenuEntry( "モブを開く", 2 );
	glutAttachMenu( GLUT_RIGHT_BUTTON );


	// メインループの開始
	argMainLoop( NULL, KeyDown, MainLoop );

	return 0;
}


// ===========================================================================
// メインループ関数
// ===========================================================================
static void MainLoop(void)
{
	ARUint8			*image;
	ARMarkerInfo	*marker_info;
	int				marker_num;
	int				j, k;

	// カメラ画像の取得
	if ( (image = arVideoGetImage()) == NULL ) {
		arUtilSleep( 2 );
		return;
	}

	// カメラ画像の描画
	argDrawMode2D();
	argDispImage( image, 0, 0 );

	// マーカの検出と認識
	if ( arDetectMarker( image, g_thresh, &marker_info, &marker_num ) < 0 ) {
		exit(0);
	}

	// 次の画像のキャプチャ指示
	arVideoCapNext();

	// マーカの信頼度の比較
	k = -1;
	for( j = 0; j < marker_num; j++ ) {
		if ( g_patt_id == marker_info[j].id ) {
			if ( k == -1 ) k = j;
			else if ( marker_info[k].cf < marker_info[j].cf ) k = j;
		}
	}

	if ( k != -1 ) {
		// マーカの位置・姿勢（座標変換行列）の計算
		if( isFirstDetect ) {
			arGetTransMat( &marker_info[k], g_patt_center, g_patt_width+g_all_size, g_patt_trans );
		} else {
			arGetTransMatCont( &marker_info[k],g_patt_trans, g_patt_center, g_patt_width+g_all_size, g_patt_trans );
		}
		isFirstDetect = 0;//初回フラグの無効

		

		// 3Dオブジェクトの描画
		DrawObject( image );

		//コンソールに情報を表示
		PrintInfo();

	}
	// バッファの内容を画面に表示
	argSwapBuffers();
}


// ===========================================================================
// 終了処理関数
// ===========================================================================
static void Cleanup(void)
{
    arVideoCapStop();	// ビデオキャプチャの停止
    arVideoClose();		// ビデオデバイスの終了
    argCleanup();		// グラフィック処理の終了

	mqoDeleteModel( g_char.model );	// モデルの削除
	mqoDeleteModel( g_mob.model );	// モデルの削除
	mqoDeleteModel( g_world.model );	// モデルの削除

	mqoCleanup();							// GLMetaseqの終了処理
}


// ===========================================================================
// 3Dオブジェクトの描画を行う関数
// ===========================================================================
static void DrawObject( ARUint8 *image )
{
	double	gl_para[16];

	// オブジェクトの状態を計算
	CalcState();

	// 3Dオブジェクトを描画するための準備
	argDrawMode3D();
	argDraw3dCamera( 0, 0 );

	// 光源の設定
	glPushMatrix();
		SetLight(GL_LIGHT0);
	glPopMatrix();

	// 座標変換行列の適用
	argConvGlpara( g_patt_trans, gl_para );
	glMatrixMode( GL_MODELVIEW );
	glLoadMatrixd( gl_para );

	// 3Dオブジェクトの描画
	glClear( GL_DEPTH_BUFFER_BIT );		// Zバッファの初期化
	glEnable( GL_DEPTH_TEST );			// 隠面処理の適用

	// シェーディング
	//if ( g_light_on ) {	
	if ( 1 ) {
		glEnable( GL_LIGHTING );	// ライトOn
	} else {
		glDisable( GL_LIGHTING );	// ライトOff
	}

	glEnable(GL_CULL_FACE);
	//モブ
	glPushMatrix();
		glTranslatef( g_mob.x, g_mob.y, g_mob.z ) ;		// モブの位置
		glRotatef( g_mob.rot, 0.0, 0.0, 1.0);			// モブの向き
		glRotatef( 90.0, 1.0, 0.0, 0.0 );				// モブを立たせる
		glCullFace(GL_BACK);
		mqoCallModel( g_mob.model );					// モブの描画

	glPopMatrix();

	//主人公
	glPushMatrix();
		glTranslatef( g_char.x, g_char.y, g_char.z ) ;	// 主人公の位置
		glRotatef( g_char.rot, 0.0, 0.0, 1.0);			// 主人公の向き
		glRotatef( 90.0, 1.0, 0.0, 0.0 );				// 主人公を立たせる
		mqoCallSequence( g_char.seq,g_char.frame );		// 主人公の描画
	glPopMatrix();

	//ワールド
	glPushMatrix();
		glTranslatef( g_world.x, g_world.y, g_world.z ) ;	// ワールドの位置
		glRotatef( g_world.rot, 0.0, 0.0, 1.0);				// ワールドの向き
		glRotatef( 90.0, 1.0, 0.0, 0.0 );					// ワールドを立たせる
		glCullFace(GL_BACK);
		mqoCallModel( g_world.model );						// ワールドの描画
		
	glPopMatrix();





	 


	glDisable( GL_LIGHTING );
	glDisable( GL_DEPTH_TEST );
		glDisable(GL_CULL_FACE);


	// 光学迷彩
	if ( g_optcmf_on ) {
		OpticalCamouflage( image );
	}
}


// ===========================================================================
// オブジェクトの状態を計算する関数
// ===========================================================================
static void CalcState(void)
{
	CONTROLLER_DATA	control;	// コントローラ（キーボード）
	static GLfloat v_char = 0;			// 主人公の進行方向の速度
	static int char_jump_flag = 0;		// 主人公のジャンプフラグ
	static GLfloat v_char_jump = 0;		// 主人公の縦方向の速度

	static GLfloat v_mob = 0;			// モブの進行方向の速度
	static int worldNum = 0;
	// コントローラのデータを取得
	GetControllerData( &control );

//================
// 主人公の動き
//================
	// 加速と減速
	if ( control.W == KEY_DOWN ) {
		// 正方向に加速
		v_char = 5;
		//アニメーションする
		g_char.frame+=2;
	} else if ( control.S  == KEY_DOWN ) { 
		// 逆方向に加速
		v_char = -5;
		//アニメーションする
		g_char.frame+=2;
	} else {
		// 減速
		if (v_char > 0) { v_char-=0.5; } 
		else if (v_char< 0 ) {v_char+=0.5;	}

		//アニメーション終了し、直立へ移行
		if( !(g_char.frame==0||g_char.frame==12) ){
			g_char.frame++;
		}

	}
	if(g_char.frame >= g_char.maxframe){
		g_char.frame = 0;
	}
	

	// 回転
	if ( control.D == KEY_DOWN ) {
		g_char.rot -= 10.0;
		if(g_char.rot <= -360){ g_char.rot=0;}
	}
	if ( control.A  == KEY_DOWN ) {
		g_char.rot += 10.0;
		if(g_char.rot >= 360){ g_char.rot=0;}
	}

	// 移動
	g_char.x +=  v_char * sin(g_char.rot*3.14/180);
	g_char.y += -v_char * cos(g_char.rot*3.14/180);

	//ジャンプ
	if ( control.sp == KEY_DOWN && char_jump_flag == 0) {
		char_jump_flag = 1;
		v_char_jump = 8;//ジャンプ力
	}

	//上下移動
	if( char_jump_flag == 1 ){
		g_char.z += v_char_jump;
		v_char_jump -= 1;//重力加速度
		if( g_char.z <= 0){
			char_jump_flag = 0;
			v_char_jump = 0;
		}
	} 


//================
// モブの動き
//================
	// 加速と減速
	if ( control.up == KEY_DOWN ) {
		// 正方向に加速
		v_mob = 3;
	} else if ( control.down  == KEY_DOWN ) { 
		// 逆方向に加速
		v_mob = -3;
	} else {
		// 減速
		if (v_mob>0) { v_mob--; }
		else if (v_mob<0) { v_mob++; }
	}

	// 回転
	if ( control.right == KEY_DOWN ) { 
		g_mob.rot -= 10.0; 
		if(g_mob.rot <= -360){ g_mob.rot=0;}
	}
	if ( control.left  == KEY_DOWN ) { 
		g_mob.rot += 10.0; 
		if(g_mob.rot >= 360){ g_mob.rot=0;}
	}

	// 移動
	g_mob.x +=  v_mob * sin(g_mob.rot*3.14/180);
	g_mob.y += -v_mob * cos(g_mob.rot*3.14/180);

//================
// 全体サイズ
//================
	// 拡大
	if ( control.minus == KEY_DOWN ) {
		g_all_size += 2;
	} else if( control.plus  == KEY_DOWN ) {
		g_all_size -= 2;
		if(g_patt_width + g_all_size <=20){
			g_all_size = 20 - g_patt_width;
		}
	} 
//================
// リセット
//================
	// 初期位置にリセット
	if ( control.X == KEY_DOWN ) {
		g_char.rot = 0;
		g_char.x = 0;
		g_char.y = 0;
		g_char.z = 0;
		g_mob.rot = 0;
		g_mob.x = 0;
		g_mob.y = 0;
		g_mob.z = 0;
		g_all_size = 0;
		control.X = KEY_UP;
	}

//================
// モデルチェンジ
//================
	// ワールドチェンジ
	if ( control.Z == KEY_DOWN ) {
		switch(worldNum){
		case 0:
			g_world.model = mqoCreateModel( g_world_name1, g_world_size );
			worldNum = 1;
			break;
		case 1:
			g_world.model = mqoCreateModel( g_world_name2, g_world_size );
			worldNum = 2;
			break;
		case 2:
			g_world.model = mqoCreateModel( g_world_name, g_world_size );
			worldNum = 0;
			break;
		}
	}
//================
// シェーディング
//================
	// 光学迷彩のOn/Off
	if ( control.C == KEY_DOWN ) {
		g_optcmf_on = !g_optcmf_on;
		control.C = KEY_UP;
	}



}


// ===========================================================================
// 光源の設定を行う関数
// ===========================================================================
static void SetLight( GLenum light )
{
	GLfloat light_diffuse[]  = { 0.9, 0.9, 0.9, 1.0 };	// 拡散反射光
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };	// 鏡面反射光
	GLfloat light_ambient[]  = { 0.3, 0.3, 0.3, 0.1 };	// 環境光
	GLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };	// 位置と種類

	// 光源の設定
	glLightfv( light, GL_DIFFUSE,  light_diffuse );	 // 拡散反射光の設定
	glLightfv( light, GL_SPECULAR, light_specular ); // 鏡面反射光の設定
	glLightfv( light, GL_AMBIENT,  light_ambient );	 // 環境光の設定
	glLightfv( light, GL_POSITION, light_position ); // 位置と種類の設定

	glEnable( light );	// 光源の有効化
}


// ===========================================================================
// 光学迷彩処理を行う関数
// ===========================================================================
static void OpticalCamouflage( unsigned char *image )
{
	GLfloat			*depth_buf, *pDepth;			// デプスバッファ
	unsigned char	*color_buf, *pColor;			// カラーバッファ
	GLfloat			depth_obj_min, depth_obj_max;	// 物体のデプス値の最小値・最大値
	GLfloat			depth, depth_obj_n;				// デプス値、物体の正規化デプス値
	unsigned char	cmfR, cmfG, cmfB;				// 光学迷彩の色
	int				width, height;					// 画像サイズ
	int				i, x, y, dx, dy;

	// 迷彩の色（迷彩の領域に薄っすら色をつけるための設定。それぞれ0～255）
	cmfR = 255;
	cmfG = 255;
	cmfB = 255;

	// 画像サイズの取得（ARToolKitのグローバル変数から取得）
	width = arImXsize;
	height = arImYsize;

	// デプスバッファの取得
	depth_buf = (GLfloat*)malloc( width * height * sizeof(GLfloat) );
	glReadPixels( 0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth_buf);

	// 描画用のバッファを作成
	color_buf = (unsigned char*)malloc( width * height * 4 );

	if ( color_buf == NULL || depth_buf == NULL ) return;

	// オブジェクトのデプス値の最大最小を求める
	GetObjectDepth( depth_buf, width, height, &depth_obj_min, &depth_obj_max);

	// ラスタスキャン用のポインタの準備
	pDepth = depth_buf;
	pColor = color_buf;

	for (y=0; y<height; y++) {
		for (x=0; x<width; x++) {

			// デプス値
			depth = *pDepth++;

			if ( depth < 1.0 ) {
				// オブジェクト領域の場合

				// デプス値を正規化
				depth_obj_n = ( depth - depth_obj_min )/( depth_obj_max - depth_obj_min );

				// デプス値を元に読み取り座標をずらす（＝歪ませる）
				dx = (int)( x -0.05 * width * depth_obj_n * ((x-width/2.0)*0.01) );
				dy = (int)( y -0.05 * height * depth_obj_n * ((y-height/2.0)*0.01) );
				if ( dx < 0 ) dx = x;
				if ( dy < 0 ) dy = y;

				// インデックスを求める
				i = 4 * ( (height-1-dy) * width + dx );

				// 画素値の書き込み
				*pColor++ = (unsigned char)( image[i+2] * 0.9 + cmfR * 0.1 );	// R
				*pColor++ = (unsigned char)( image[i+1] * 0.9 + cmfG * 0.1 );	// G
				*pColor++ = (unsigned char)( image[i]   * 0.9 + cmfB * 0.1 );	// B
				*pColor++ = 255;												// A

			} else {
				// 背景の場合
				pColor += 3;
				*pColor++ = 0;
			}
		}
	}

	// 画像の描画
	DrawImage( color_buf, width ,height );

	// バッファの開放
	free( depth_buf );
	free( color_buf );
}


// ===========================================================================
// オブジェクトのデプス値の最小値、最大値を求める関数
// depth         : デプスバッファ
// width, height : 画像サイズ
// depth_min     : デプス値の最小値
// depth_max     : デプス値の最大値（背景=1.0を除く）
// ===========================================================================
static void GetObjectDepth( GLfloat *depth, int width, int height,
							GLfloat *depth_min, GLfloat *depth_max )
{
	GLfloat min = 0;
	GLfloat max = 0;
	GLfloat d;
	int i;

	for ( i=0; i<width*height; i++ ) {

		d = *depth++;

		if ( i==0 ) {
			min = d;
			max = 0;
		} else {
			if ( d > max && d <1.0 ) max = d;
			if ( d < min ) min = d;
		}
	}

	*depth_min = min;
	*depth_max = max;
}


// ===========================================================================
// 画像の描画を行う関数
// color_buf     : カラーバッファ
// width, height : 画像サイズ
// ===========================================================================
void DrawImage( unsigned char *color_buf, int width, int height )
{
	// 射影行列を保存
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();

		// 射影行列を変更
		glLoadIdentity();
		gluOrtho2D( 0, width, 0, height );

		// モデルビュー行列を保存
		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();

			// 画像の描画
			glLoadIdentity();
			glRasterPos2i( 0, 0 );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			glEnable( GL_BLEND);
			glDrawPixels( width, height, GL_RGBA, GL_UNSIGNED_BYTE, color_buf );
			glDisable( GL_BLEND );

		// モデルビュー行列を復元
		glPopMatrix();

	// 射影行列を復元
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
}

// ===========================================================================
// メニューの項目が選択されたとき
// ===========================================================================
void menu( int value )
{
	switch( value )
	{
		case 1 :	// モデル読込み
			openSkin();
			break;

		case 2 :	// モーション読込み
			openMob();
			break;


	}
}
void openSkin(){
}
void openMob(){
//	#ifdef	_WIN32
//	OPENFILENAME	ofn;
//	wchar_t			wszFileName[256],
//					wszFile[64];
//	char			szFileName[256];
//
//	ZeroMemory( &wszFileName, 256 );
//	ZeroMemory( &wszFile,      64 );
//	ZeroMemory( &ofn, sizeof(OPENFILENAME) );
//	ofn.lStructSize = sizeof(OPENFILENAME);
//	ofn.hwndOwner = NULL;
//	ofn.lpstrFilter = L"Metasequoia File(*.mqo)\0*.mqo\0\0";
//	ofn.lpstrFile = wszFileName;
//	ofn.lpstrFileTitle = wszFile;
//	ofn.nMaxFile = MAX_PATH;
//	ofn.nMaxFileTitle = sizeof(wszFile);
//	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
//	ofn.lpstrDefExt = L"mqo";
//	ofn.lpstrTitle = L"Open";
//if( GetOpenFileName( &ofn ) )
//	{
//		SetCurrentDirectory( wszFileName );
//
//		setlocale( LC_ALL, "Japanese_Japan.932" );
//	    wcstombs( szFileName, wszFileName, 255 );
//
//	}

//#endif
}

// ===========================================================================
// コンソール画面消去関数
// system("cls")とほぼ同じだが、チラツキが減る事を期待している
// ===========================================================================
static void cls( void )
{
 CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };
 COORD pos = { 0 };
 HANDLE hStderr;

 hStderr = GetStdHandle( STD_ERROR_HANDLE );
 GetConsoleScreenBufferInfo( hStderr, &csbi );
 SetConsoleCursorPosition( hStderr, pos );
 FillConsoleOutputCharacter( hStderr, CLS_SPACE, (csbi.dwSize.X * csbi.dwSize.Y), pos, NULL );
 FillConsoleOutputAttribute( hStderr, CLS_COLOR, (csbi.dwSize.X * csbi.dwSize.Y), pos, NULL );
}

// ===========================================================================
// コンソールに情報表示
// ===========================================================================
static void PrintInfo(void){
	cls();//一度コンソール画面を消去
	//取説
	printf("<<操作方法>>\n");
	printf("WASC     : 主人公移動\n");
	printf("方向キー : モブ移動\n");
	printf("   %c     : 全体拡大\n",KEY_PLUS);
	printf("   %c     : 全体縮小\n",KEY_MINUS);
	printf("   %c     : 初期位置にリセット\n", KEY_X);
	printf("   %c     : シェーディングOn/Off\n", KEY_Z);
	printf("   %c     : 透明ポーションOn/Off \n", KEY_C);
	printf("  ESC    : 終了\n\n");
	
	//描画情報
	printf("<<主人公>>\n");
	printf("[ファイル]%s\n",g_charwalk_name);
	printf("[座標]x:%.2f y:%.2f z:%.2f rot:%.2f\n",g_char.x,g_char.y,g_char.z,g_char.rot);
	printf("[アニメ]frame:%d maxframe%d\n",g_char.frame,g_char.maxframe);
	printf("\n");
	printf("<<敵>>\n");
	printf("[ファイル]%s\n",g_mob_name);
	printf("[座標]x:%.2f y:%.2f z:%.2f rot:%.2f\n",g_mob.x,g_mob.y,g_mob.z,g_mob.rot);
	printf("[アニメ]frame:%d maxframe%d\n",g_mob.frame,g_mob.maxframe);
	printf("\n");
	printf("[World]\n");
	printf("[全体サイズ] %.2f\n",80/(g_patt_width+g_all_size));

}

//--------------------------------------------------------
//  音楽(waveファイル)を再生する関数
//--------------------------------------------------------
static void play_music(int flg){

	if(flg==0){//再生終了
		PlaySound(NULL,NULL,0);//再生の停止
	}
	if(flg==1){//再生開始
		PlaySound("music/bgm.wav" ,//wave(音楽)ファイルを再生開始
			NULL,
			SND_FILENAME|SND_ASYNC|SND_LOOP);
		//pazzSoundパラメータをファイル名,非同期,ループで再生する
		//※同期させると終わるまで次の処理にいけません
	}

}