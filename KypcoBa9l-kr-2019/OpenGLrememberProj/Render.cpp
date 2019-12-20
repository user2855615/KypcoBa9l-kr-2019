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







//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;

	
	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
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
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}

	
	//������ ����� � ����� ��� ���������� �����, ���������� �������
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
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
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

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����




//������ ���������� ����
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//������� ���� �� ���������, � ����� ��� ����
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

//����������� ����� ������ ��������
void initRender(OpenGL *ogl)
{
	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);
	

	//������ ����������� ���������  (R G B)
	RGBTRIPLE *texarray;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//���������� �� ��� ��������
	glGenTextures(1, &texId);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH); 


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

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


	//��������������
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//��������� ���������
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//������ �����
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//���� ���� �������, ��� ����������� (����������� ���������)
	glShadeModel(GL_SMOOTH);
	//===================================
	//������� ���  


/*	//������ ��������� ���������� ��������
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
	*///����� ��������� ���������� ��������







	//��� �����
	



	
		//��� 1
		double A[] = { 0, 0, 0 };
		double B[] = { -12, -3, 0 };
		double C[] = { -5, 3, 0 };



		glBegin(GL_TRIANGLES);

		glNormal3d(0, 0, -1);
		glColor3d(0.9, 0.2, 0.6);
		glVertex3dv(A);
		glVertex3dv(B);
		glVertex3dv(C);
		

		// 2���
		double A1[] = { -12, -3, 0 };
		double B1[] = { -5, 3, 0 };
		double C1[] = { -15, 3, 0 };



		


		glVertex3dv(A1);
		glVertex3dv(B1);
		glVertex3dv(C1);
		

		//3���

		double A2[] = { -15, 3, 0 };
		double B2[] = { -5, 3, 0 };
		double C2[] = { -15, 5, 0 };



		


		glVertex3dv(A2);
		glVertex3dv(B2);
		glVertex3dv(C2);
		

		//4���

		double A3[] = { -15, 5, 0 };
		double B3[] = { -5, 3, 0 };
		double C3[] = { -12, 9, 0 };



		


		glVertex3dv(A3);
		glVertex3dv(B3);
		glVertex3dv(C3);
		

		//5���

		double A4[] = { -12, 9, 0 };
		double B4[] = { -5, 3, 0 };
		double C4[] = { -1, 7, 0 };



		


		glVertex3dv(A4);
		glVertex3dv(B4);
		glVertex3dv(C4);
		


		//6���

		double A5[] = { -19, 0, 0 };
		double B5[] = { -15, 3, 0 };
		double C5[] = { -15, 5, 0 };



		


		glVertex3dv(A5);
		glVertex3dv(B5);
		glVertex3dv(C5);
		

		//7���

		double A6[] = { -19, 0, 0 };
		double B6[] = { -15, 5, 0 };
		double C6[] = { -19, 8, 0 };



		


		glVertex3dv(A6);
		glVertex3dv(B6);
		glVertex3dv(C6);
		

		//��� ������

		double A7[] = { 0, 0, 3 };
		double B7[] = { -12, -3, 3 };
		double C7[] = { -5, 3, 3 };



		
		glNormal3d(0, 0, 1);
		glColor3d(0.6, 0.7, 0.4);
		glVertex3dv(A7);
		glVertex3dv(B7);
		glVertex3dv(C7);
		

		// 2������

		double A8[] = { -12, -3, 3 };
		double B8[] = { -5, 3, 3 };
		double C8[] = { -15, 3, 3 };



		


		glVertex3dv(A8);
		glVertex3dv(B8);
		glVertex3dv(C8);
		

		//3������

		double A9[] = { -15, 3, 3 };
		double B9[] = { -5, 3, 3 };
		double C9[] = { -15, 5, 3 };



		


		glVertex3dv(A9);
		glVertex3dv(B9);
		glVertex3dv(C9);
		

		//4������

		double A0[] = { -15, 5, 3 };
		double B0[] = { -5, 3, 3 };
		double C0[] = { -12, 9, 3 };



		


		glVertex3dv(A0);
		glVertex3dv(B0);
		glVertex3dv(C0);

		//5������

		double A11[] = { -12, 9, 3 };
		double B11[] = { -5, 3, 3 };
		double C11[] = { -1, 7, 3 };



		

		glVertex3dv(A11);
		glVertex3dv(B11);
		glVertex3dv(C11);
		

		//6������

		double A12[] = { -19, 0, 3 };
		double B12[] = { -15, 3, 3 };
		double C12[] = { -15, 5, 3 };



		


		glVertex3dv(A12);
		glVertex3dv(B12);
		glVertex3dv(C12);
		

		//7������

		double A13[] = { -19, 0, 3 };
		double B13[] = { -15, 5, 3 };
		double C13[] = { -19, 8, 3 };



		


		glVertex3dv(A13);
		glVertex3dv(B13);
		glVertex3dv(C13);
		glEnd();


		// ����� 1

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


		//�����2

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


		//3�����

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


		//4�����

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

		//5�����

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

		//6�����

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

		//7�����

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

		//8�����

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

		//9�����
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
		glVertex2f(x, y);  //����� ����������
		for (int i = 0; i <= t; i++) {
			float a = (float)i / (float)t * 3.1415f * 2.0f;
			glVertex2f(x - cos(a) * r, y - sin(a) * r);
		}
		glEnd();


		glFinish();




	


	
	//����� �����










   //��������� ������ ������

	
	glMatrixMode(GL_PROJECTION);	//������ �������� ������� ��������. 
	                                //(���� ��������� ��������, ����� �� ������������.)
	glPushMatrix();   //��������� ������� ������� ������������� (������� ��������� ������������� ��������) � ���� 				    
	glLoadIdentity();	  //��������� ��������� �������
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //������� ����� ������������� ��������

	glMatrixMode(GL_MODELVIEW);		//������������� �� �����-��� �������
	glPushMatrix();			  //��������� ������� ������� � ���� (��������� ������, ����������)
	glLoadIdentity();		  //���������� �� � ������

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //������� ����� ��������� ��� ������� ������ � �������� ������.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - ���/���� �������" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	ss << "G - ������� ���� �� �����������" << std::endl;
	ss << "G+��� ������� ���� �� ���������" << std::endl;
	ss << "�����. �����: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "�����. ������: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "��������� ������: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //��������������� ������� �������� � �����-��� �������� �� �����.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}