#include <iostream>
#include <fstream>
#include <vector>
#include <glut.h>
#include <cmath>
#include <cstdlib>
#include "HE.h"
#include "Operation.h"

#ifndef CALLBACK
#define CALLBACK
#endif

const double PI = acos(-1.0);

//glutess callback function
void CALLBACK tessBeginFcn(GLenum which);
void CALLBACK tessEndFcn();
void CALLBACK tessVertexFcn(const GLvoid *data);

//glut callback function
void displayFcn();
void reshapeFcn(GLint newWidth, GLint newHeight);

//global variables
GLint winWidth = 800, winHeight = 600;//Initial display-window size
GLdouble eyex = 15.0, eyey = 0.0, eyez = 0.0;//Viewing-coordinate origin
GLdouble xref = 0.0, yref = 0.0, zref = 0.0;//Look-at point
GLdouble Vx = 0.0, Vy = 0.0, Vz = 1.0;//View-up vector
GLdouble xwMin = -8.0, xwMax = 8.0, ywMin = -6.0, ywMax = 6.0;//Coordinate limits of clipping window
GLdouble dnear = 1.0, dfar = 30.0;//Position of near and far clipping planes
Solid *sol;//solid to be displayed
std::vector<GLuint> displayListIds;//vector of display lists
std::vector<GLdouble *> points;
GLdouble phi = 0.0, theta = PI / 2, r = 15.0;//spherical coordinate parameters
GLint mouseDown = 0;//whether left button is clicked
GLfloat xclick, yclick;//click position

void initGL();
void initGLUT(int argc, char **argv);
void initLight();
void EnterPoint(int index, float point[3], int level);
Solid *buildSolid();//build a solid
void Solid2List(Solid *s);//solid -> display list, point list
void showSolid(Solid *s);
const char* getPrimitiveType(GLenum type);
void setModelView();
void leftBtnClick(GLint button, GLint action, GLint xMouse, GLint yMouse);//when left mouse button clicked
void mouseRotate(GLint xMouse, GLint yMouse);//rotate viewing coordinate when mouse move
void keyFcn(unsigned char key, int x, int y);//keyboard function
GLint manualInput = 1;

int main(int argc,char **argv)
{
	initGLUT(argc, argv);
	initGL();
	initLight();

	std::cout << "Read from file?(y/n)";
	char tmpc;
	std::cin >> tmpc;
	if (tmpc == 'y' || tmpc == 'Y') {
		manualInput = 0;
		char filename[70];
		std::cout << "Enter file name(including full path): ";
		std::cin >> filename;
		std::streambuf *backup;
		std::ifstream fin;
		fin.open(filename);
		backup = std::cin.rdbuf();
		std::cin.rdbuf(fin.rdbuf());//assign file's streambuf to cin
		sol = buildSolid();
		std::cin.rdbuf(backup);//restore cin's original streambuf
	}
	else if (tmpc == 'n' || tmpc == 'N') {
		manualInput = 1;
		sol = buildSolid();
	}
	else {
		exit(0);
	}
	std::cout << "Solid information:\n";
	showSolid(sol);
	Solid2List(sol);

	glutMainLoop();

	return 0;
}

void EnterPoint(int index, float point[3], int level)
{
	if (manualInput) {
		while (level--) std::cout << "\t";
		std::cout << "Enter the coordinate of the " << index;
		switch (index % 10)
		{
		case 1:
			std::cout << "st";
			break;
		case 2:
			std::cout << "nd";
			break;
		case 3:
			std::cout << "rd";
			break;
		default:
			std::cout << "th";
			break;
		}
		std::cout << " point(float x, y, z): ";
	}
	std::cin >> point[0] >> point[1] >> point[2];
}

Solid *buildSolid()
{
	Solid *s;
	std::vector<Vertex *> v;
	Face *f0, *f1;
	Edge *ed;
	std::vector<Loop *> ilps;//inner loops
	std::vector<Face *> ifs;//inner faces
	float point[3] = {};

	int ovnum;
	
	if(manualInput)
		std::cout << "Enter number of vertex in outer loop: ";
	std::cin >> ovnum;
	v.resize(ovnum);
	EnterPoint(1, point, 1);
	EulerOperation::mvfs(&s, &v[0], &f0, point);

	for (int i = 1; i < ovnum; i++) {
		EnterPoint(i + 1, point, 1);
		EulerOperation::mev(v[i - 1], &v[i], &ed, f0->floops, point);
	}

	EulerOperation::mef(v[ovnum - 1], v[0], f0->floops, &ed, &f1);

	int ilnum;
	if(manualInput)
		std::cout << "Enter number of inner loop: ";
	std::cin >> ilnum;
	ilps.resize(ilnum);
	ifs.resize(ilnum);

	for (int i = 0; i < ilnum; i++) {
		int ivnum;
		if (manualInput) {
			std::cout << "\tInner loop #" << i + 1 << ":\n";
			std::cout << "\t\tEnter number of vertex in the loop: ";
		}
		std::cin >> ivnum;
		int tmpvnum = v.size();
		v.resize(tmpvnum + ivnum);
		EnterPoint(1, point, 3);
		EulerOperation::mev(v[0], &v[tmpvnum], &ed, f0->floops, point);
		for (int j = 1; j < ivnum; j++) {
			EnterPoint(j + 1, point, 3);
			EulerOperation::mev(v[tmpvnum + j - 1], &v[tmpvnum + j], &ed, f0->floops, point);
		}
		EulerOperation::kemr(v[0], v[tmpvnum], f0->floops, &ilps[i]);
		EulerOperation::mef(v[tmpvnum + ivnum - 1], v[tmpvnum], ilps[i], &ed, &ifs[i]);
	}

	float ve[3];
	if (manualInput)
		std::cout << "Enter sweeping vector(float x, y, z): ";
	std::cin >> ve[0] >> ve[1] >> ve[2];
	float dis;
	if (manualInput)
		std::cout << "Enter sweeping distance(float d): ";
	std::cin >> dis;

	
	Sweeping::translate_sweep(f1, ve, dis);
	for (int i = 0; i < (int)ifs.size(); i++) {
		Sweeping::translate_sweep(ifs[i], ve, 10);
		EulerOperation::kfmrh(f1, ifs[i]);
	}

	return s;
}

void showSolid(Solid *s)
{
	Face *pf = s->sfaces;
	int fid = 1;
	while (pf != nullptr) {
		std::cout << "\nFace#" << fid << ":\n";
		Loop *plp = pf->floops;
		int lpid = 1;
		while (plp != nullptr) {
			std::cout << "\tLoop#" << lpid << ":\n";
			HalfEdge *phe = plp->ledg;
			std::cout << "\t\t";
			do {
				std::cout << phe->vtx->id;
				std::cout << "(" << phe->vtx->x << ", " << phe->vtx->y << ", " << phe->vtx->z << ") ";
				phe = phe->next;
			} while (phe != plp->ledg);
			std::cout << std::endl;
			lpid++;
			plp = plp->nextl;
		}
		fid++;
		pf = pf->nextf;
	}
}

void Solid2List(Solid *s)
{
	//initial coordinates list
	points.resize(Vertex::num);
	for (int i = 0; i < Vertex::num; i++) {
		GLdouble *pcoord = new GLdouble[3];
		pcoord[0] = Vertex::vList[i]->x;
		pcoord[1] = Vertex::vList[i]->y;
		pcoord[2] = Vertex::vList[i]->z;
		points[i] = pcoord;
	}

	Face *pf = s->sfaces;
	GLuint id = glGenLists(Face::num);//generate list ids
	while (pf != nullptr) {
		Loop *lp = pf->floops;
		//face with holes
		if (lp->nextl != nullptr) {
			GLUtesselator *tess = gluNewTess();
			gluTessCallback(tess, GLU_TESS_BEGIN, (void (CALLBACK *)())tessBeginFcn);
			gluTessCallback(tess, GLU_TESS_END, (void (CALLBACK *)())tessEndFcn);
			gluTessCallback(tess, GLU_TESS_VERTEX, (void (CALLBACK *)())tessVertexFcn);

			glNewList(id, GL_COMPILE);
				glColor3f(0.0, 1.0, 0.0);
				gluTessBeginPolygon(tess, 0);
					while (lp != nullptr) {
						HalfEdge *he = lp->ledg;
						gluTessBeginContour(tess);
							do {
								Vertex *tv = he->vtx;
								gluTessVertex(tess, points[tv->id], points[tv->id]);
								he = he->next;
							} while (he != lp->ledg);
						gluTessEndContour(tess);
						lp = lp->nextl;
					}
				gluTessEndPolygon(tess);
			glEndList();
			displayListIds.push_back(id);

			gluDeleteTess(tess);
		}
		//face without holes
		else {
			HalfEdge *he = lp->ledg;
			glNewList(id, GL_COMPILE);
				glColor3f(0.0, 1.0, 0.0);
				glBegin(GL_POLYGON);
				do {
					Vertex *tv = he->vtx;
					glVertex3d(tv->x, tv->y, tv->z);
					he = he->next;
				} while (he != lp->ledg);
				glEnd();
			glEndList();
			displayListIds.push_back(id);
		}

		id++;
		pf = pf->nextf;
	}
}

void initGLUT(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(winWidth, winHeight);
	glutCreateWindow("view");

	glutDisplayFunc(displayFcn);
	glutReshapeFunc(reshapeFcn);
	glutMouseFunc(leftBtnClick);
	glutMotionFunc(mouseRotate);
	glutKeyboardFunc(keyFcn);
}

void initGL()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	//Set the transformation function between global-coordinate & viewing-coordinate
	glMatrixMode(GL_MODELVIEW);
	setModelView();

	//Set the projection function
	glMatrixMode(GL_PROJECTION);
	glFrustum(xwMin, xwMax, ywMin, ywMax, dnear, dfar);

	glEnable(GL_DEPTH_TEST);
}

void initLight()
{
	GLfloat ambientLight[] = { 0.2f,  0.2f,  0.2f,  1.0f };//ambient light
	GLfloat diffuseLight[] = { 0.9f,  0.9f,  0.9f,  1.0f };//diffuse light
	GLfloat specularLight[] = { 1.0f,  1.0f,  1.0f,  1.0f };//specular light
	GLfloat lightPos[] = { eyex, eyey, eyez, 1.0f };//light position

	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glEnable(GL_LIGHT0);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specularLight);
	glMateriali(GL_FRONT, GL_SHININESS, 50);
}

void displayFcn()
{
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	for (int i = 0; i < (int)displayListIds.size(); i++) {
		glCallList(displayListIds[i]);
	}
	//draw coordinate axis
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
		glVertex3d(0, 0, 0);
		glVertex3d(15, 0, 0);
		glVertex3d(0, 0, 0);
		glVertex3d(0, 15, 0);
		glVertex3d(0, 0, 0);
		glVertex3d(0, 0, 15);
	glEnd();

	glFlush();
}

void reshapeFcn(GLint newWidth, GLint newHeight)
{
	glViewport(0, 0, newWidth, newHeight);

	winWidth = newWidth;
	winHeight = newHeight;
}

void CALLBACK tessBeginFcn(GLenum which)
{
	glBegin(which);
	//Debug
	//std::cout << "glBegin(" << getPrimitiveType(which) << ");\n";
}

void CALLBACK tessEndFcn()
{
	glEnd();
	//Debug
	//std::cout << "glEnd();\n";
}

void CALLBACK tessVertexFcn(const GLvoid *data)
{
	const GLdouble *pdata = (const GLdouble*)data;
	glVertex3dv(pdata);
	//Debug
	//std::cout << "glVertex3d(" << pdata[0] << ", " << pdata[1] << ", " << pdata[2] << ");\n";
}

const char* getPrimitiveType(GLenum type)
{
	switch (type)
	{
	case 0x0000:
		return "GL_POINTS";
		break;
	case 0x0001:
		return "GL_LINES";
		break;
	case 0x0002:
		return "GL_LINE_LOOP";
		break;
	case 0x0003:
		return "GL_LINE_STRIP";
		break;
	case 0x0004:
		return "GL_TRIANGLES";
		break;
	case 0x0005:
		return "GL_TRIANGLE_STRIP";
		break;
	case 0x0006:
		return "GL_TRIANGLE_FAN";
		break;
	case 0x0007:
		return "GL_QUADS";
		break;
	case 0x0008:
		return "GL_QUAD_STRIP";
		break;
	case 0x0009:
		return "GL_POLYGON";
		break;
	default:
		return nullptr;
	}
}

void setModelView()
{
	//Spherical coordinates -> Cartesian coordinates
	eyex = xref + r * sin(theta)*cos(phi);
	eyey = yref + r * sin(theta)*sin(phi);
	eyez = zref + r * cos(theta);

	gluLookAt(eyex, eyey, eyez, xref, yref, zref, Vx, Vy, Vz);
}

void leftBtnClick(GLint button, GLint action, GLint xMouse, GLint yMouse)
{
	if (button == GLUT_LEFT_BUTTON && action == GLUT_DOWN) {
		mouseDown = 1;
		xclick = xMouse;
		yclick = yMouse;
	}
	else if (button == GLUT_LEFT_BUTTON && action == GLUT_UP) {
		mouseDown = 0;
		//Debug
		//printf("eyex: %.2f, eyey: %.2f, eyez: %.2f, phi: %.2f, theta: %.2f, radius: %.2f\n", eyex, eyey, eyez, phi, theta, r);
	}
}

void keyFcn(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
	else if (key == 'w') {
		r -= 0.5;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		setModelView();
		glutPostRedisplay();
		//Debug
		//printf("eyex: %.2f, eyey: %.2f, eyez: %.2f, phi: %.2f, theta: %.2f, radius: %.2f\n", eyex, eyey, eyez, phi, theta, r);
	}
	else if (key == 's') {
		r += 0.5;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		setModelView();
		glutPostRedisplay();
		//Debug
		//printf("eyex: %.2f, eyey: %.2f, eyez: %.2f, phi: %.2f, theta: %.2f, radius: %.2f\n", eyex, eyey, eyez, phi, theta, r);
	}
}

void mouseRotate(GLint xMouse, GLint yMouse)
{
	if (mouseDown == 1) {
		phi -= 0.0005 * (xMouse - xclick);
		if (phi > 2 * PI) phi -= 2 * PI;
		theta -= 0.0005 * (yMouse - yclick);
		//abs(theta) > 5degree
		if (theta < PI / 36) theta = PI / 36;
		else if (theta > PI - PI / 36) theta = PI - PI / 36;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		setModelView();
		glutPostRedisplay();
	}
}