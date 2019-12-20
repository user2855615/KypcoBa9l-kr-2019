#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;







//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}





void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  


/*	//Начало рисования квадратика станкина
	double A[2] = { -4, -4 };
	double B[2] = { 4, -4 };
	double C[2] = { 4, 4 };
	double D[2] = { -4, 4 };

	glBindTexture(GL_TEXTURE_2D, texId);

	glColor3d(0.6, 0.6, 0.6);
	glBegin(GL_QUADS);

	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);

	glEnd();
	*///конец рисования квадратика станкина







	//моя рыбка
	



	
		//пол 1
		double A[] = { 0, 0, 0 };
		double B[] = { -12, -3, 0 };
		double C[] = { -5, 3, 0 };



		glBegin(GL_TRIANGLES);

		glNormal3d(0, 0, -1);
		glColor3d(0.9, 0.2, 0.6);
		glVertex3dv(A);
		glVertex3dv(B);
		glVertex3dv(C);
		

		// 2пол
		double A1[] = { -12, -3, 0 };
		double B1[] = { -5, 3, 0 };
		double C1[] = { -15, 3, 0 };



		


		glVertex3dv(A1);
		glVertex3dv(B1);
		glVertex3dv(C1);
		

		//3пол

		double A2[] = { -15, 3, 0 };
		double B2[] = { -5, 3, 0 };
		double C2[] = { -15, 5, 0 };



		


		glVertex3dv(A2);
		glVertex3dv(B2);
		glVertex3dv(C2);
		

		//4пол

		double A3[] = { -15, 5, 0 };
		double B3[] = { -5, 3, 0 };
		double C3[] = { -12, 9, 0 };



		


		glVertex3dv(A3);
		glVertex3dv(B3);
		glVertex3dv(C3);
		

		//5пол

		double A4[] = { -12, 9, 0 };
		double B4[] = { -5, 3, 0 };
		double C4[] = { -1, 7, 0 };



		


		glVertex3dv(A4);
		glVertex3dv(B4);
		glVertex3dv(C4);
		


		//6пол

		double A5[] = { -19, 0, 0 };
		double B5[] = { -15, 3, 0 };
		double C5[] = { -15, 5, 0 };



		


		glVertex3dv(A5);
		glVertex3dv(B5);
		glVertex3dv(C5);
		

		//7пол

		double A6[] = { -19, 0, 0 };
		double B6[] = { -15, 5, 0 };
		double C6[] = { -19, 8, 0 };



		


		glVertex3dv(A6);
		glVertex3dv(B6);
		glVertex3dv(C6);
		

		//пол сверху

		double A7[] = { 0, 0, 3 };
		double B7[] = { -12, -3, 3 };
		double C7[] = { -5, 3, 3 };



		
		glNormal3d(0, 0, 1);
		glColor3d(0.6, 0.7, 0.4);
		glVertex3dv(A7);
		glVertex3dv(B7);
		glVertex3dv(C7);
		

		// 2сверху

		double A8[] = { -12, -3, 3 };
		double B8[] = { -5, 3, 3 };
		double C8[] = { -15, 3, 3 };



		


		glVertex3dv(A8);
		glVertex3dv(B8);
		glVertex3dv(C8);
		

		//3сверху

		double A9[] = { -15, 3, 3 };
		double B9[] = { -5, 3, 3 };
		double C9[] = { -15, 5, 3 };



		


		glVertex3dv(A9);
		glVertex3dv(B9);
		glVertex3dv(C9);
		

		//4сверху

		double A0[] = { -15, 5, 3 };
		double B0[] = { -5, 3, 3 };
		double C0[] = { -12, 9, 3 };



		


		glVertex3dv(A0);
		glVertex3dv(B0);
		glVertex3dv(C0);

		//5сверху

		double A11[] = { -12, 9, 3 };
		double B11[] = { -5, 3, 3 };
		double C11[] = { -1, 7, 3 };



		

		glVertex3dv(A11);
		glVertex3dv(B11);
		glVertex3dv(C11);
		

		//6сверху

		double A12[] = { -19, 0, 3 };
		double B12[] = { -15, 3, 3 };
		double C12[] = { -15, 5, 3 };



		


		glVertex3dv(A12);
		glVertex3dv(B12);
		glVertex3dv(C12);
		

		//7сверху

		double A13[] = { -19, 0, 3 };
		double B13[] = { -15, 5, 3 };
		double C13[] = { -19, 8, 3 };



		


		glVertex3dv(A13);
		glVertex3dv(B13);
		glVertex3dv(C13);
		glEnd();


		// стена 1

		double A14[] = { 0, 0, 0 };
		double B14[] = { 0, 0, 3 };
		double C14[] = { -12, -3, 3 };
		double D14[] = { -12, -3, 0 };



		glBegin(GL_QUADS);

		glColor3d(0.6, 0.6, 0.6);
		glNormal3d(9 / 37.1, -36/37.1, 0);
		glVertex3dv(A14);
		glVertex3dv(B14);
		glVertex3dv(C14);
		glVertex3dv(D14);


		//стена2

		double A15[] = { -12, -3, 0 };
		double B15[] = { -12, -3, 3 };
		double C15[] = { -15, 3, 3 };
		double D15[] = { -15, 3, 0 };

		glColor3d(0.9, 0.2, 0.5);
		//glNormal3d(18 / 20.1, -9 / 20.1, 0);
		glNormal3d(-45, -25, 6);
		glVertex3dv(A15);
		glVertex3dv(B15);
		glVertex3dv(C15);
		glVertex3dv(D15);


		//3стена

		double A16[] = { -15, 3, 0 };
		double B16[] = { -15, 3, 3 };
		double C16[] = { -19, 0, 3 };
		double D16[] = { -19, 0, 0 };

		glColor3d(0.1, 0.8, 0.2);
		glNormal3d(-2, -32, 2);
		glVertex3dv(A16);
		glVertex3dv(B16);
		glVertex3dv(C16);
		glVertex3dv(D16);


		//4стена

		double A17[] = { -19, 0, 0 };
		double B17[] = { -19, 0, 3 };
		double C17[] = { -19, 8, 3 };
		double D17[] = { -19, 8, 0 };

		glColor3d(0.5, 0.1, 0.1);
		glNormal3d(-24, 3, 3);
		glVertex3dv(A17);
		glVertex3dv(B17);
		glVertex3dv(C17);
		glVertex3dv(D17);

		//5стена

		double A18[] = { -19, 8, 0 };
		double B18[] = { -19, 8, 3 };
		double C18[] = { -15, 5, 3 };
		double D18[] = { -15, 5, 0 };

		glColor3d(0.9, 0.3, 0.4);
		glNormal3d(-2, 32, 2);
		glVertex3dv(A18);
		glVertex3dv(B18);
		glVertex3dv(C18);
		glVertex3dv(D18);

		//6стена

		double A19[] = { -15, 5, 0 };
		double B19[] = { -15, 5, 3 };
		double C19[] = { -12, 9, 3 };
		double D19[] = { -12, 9, 0 };

		glColor3d(0.6, 0.2, 0.2);
		glNormal3d(-26, 16, 2);
		glVertex3dv(A19);
		glVertex3dv(B19);
		glVertex3dv(C19);
		glVertex3dv(D19);

		//7стена

		double A20[] = { -12, 9, 0 };
		double B20[] = { -12, 9, 3 };
		double C20[] = { -1, 7, 3 };
		double D20[] = { -1, 7, 0 };

		glColor3d(0.3, 0.8, 0.8);
		glNormal3d(4.7, 27, 2);
		glVertex3dv(A20);
		glVertex3dv(B20);
		glVertex3dv(C20);
		glVertex3dv(D20);

		//8стена

		double A21[] = { -1, 7, 0 };
		double B21[] = { -1, 7, 3 };
		double C21[] = { -5, 3, 3 };
		double D21[] = { -5, 3, 0 };

		glColor3d(0.2, 0.2, 0.9);
		glNormal3d(12, -7, 2.3);
		glVertex3dv(A21);
		glVertex3dv(B21);
		glVertex3dv(C21);
		glVertex3dv(D21);

		//9стена
		double A22[] = { -5, 3, 0 };
		double B22[] = { -5, 3, 3 };
		double C22[] = { 0, 0, 3 };
		double D22[] = { 0, 0, 0 };

		glColor3d(0.1, 0.5, 0.5);
		glNormal3d(11, 9, 2.5);
		glVertex3dv(A22);
		glVertex3dv(B22);
		glVertex3dv(C22);
		glVertex3dv(D22);
		glEnd();


		int x = 5, y = 5, r = 5, t = 50;
		glNormal3d(0, 0, 30);
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(x, y);  //центр окружности
		for (int i = 0; i <= t; i++) {
			float a = (float)i / (float)t * 3.1415f * 2.0f;
			glVertex2f(x - cos(a) * r, y - sin(a) * r);
		}
		glEnd();


		glFinish();




	


	
	//конец рыбки










   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}