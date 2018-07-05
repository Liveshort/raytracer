#include <stdio.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#endif
#ifdef WIN32
#include <windows.h>
#endif

#include "raytracing.h"

using namespace std;

//temporary variables
//these are only used to illustrate
//a simple debug drawing. A ray
Vec3Df testRayOrigin;
Vec3Df testRayDestination;

// some needed global variables
vector<AABB> boundingBoxes;

void init()
{
    // load mesh
    cout << "Loading mesh..." << endl;
    MyMesh.loadMesh("dodgeColorTest.obj", true);
	MyMesh.computeVertexNormals();

	MyLightPositions.push_back(MyCameraPosition);

    cout << "Calculating bounding boxes..." << endl;
    init_AABB(boundingBoxes);
}

//return the color of your pixel.
Vec3Df performRayTracing(const Vec3Df & origin, const Vec3Df & dest)
{
	return Vec3Df(dest[0],dest[1],dest[2]);
}



void yourDebugDraw()
{
	//draw open gl debug stuff
	//this function is called every frame

	//let's draw the mesh
	MyMesh.draw();

	//let's draw the lights in the scene as points
	glPushAttrib(GL_ALL_ATTRIB_BITS); //store all GL attributes
	glDisable(GL_LIGHTING);
	glColor3f(1,1,1);
	glPointSize(10);
	glBegin(GL_POINTS);
	for (int i=0;i<MyLightPositions.size();++i)
		glVertex3fv(MyLightPositions[i].pointer());
	glEnd();
	glPopAttrib();//restore all GL attributes
	//The Attrib commands maintain the state.
	//e.g., even though inside the two calls, we set
	//the color to white, it will be reset to the previous
	//state after the pop.


	//as an example: we draw the test ray, which is set by the keyboard function
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);
	glColor3f(0,1,1);
	glVertex3f(testRayOrigin[0], testRayOrigin[1], testRayOrigin[2]);
	glColor3f(0,0,1);
	glVertex3f(testRayDestination[0], testRayDestination[1], testRayDestination[2]);
	glEnd();
	glPointSize(10);
	glBegin(GL_POINTS);
	glVertex3fv(MyLightPositions[0].pointer());
	glEnd();
	glPopAttrib();

    // draw boundingboxes
    for (AABB boundingBox : boundingBoxes) draw_AABB(boundingBox);
}

// function that deals with keyboard input
void yourKeyboardFunc(char t, int x, int y, const Vec3Df & rayOrigin, const Vec3Df & rayDestination)
{

	//here, as an example, I use the ray to fill in the values for my upper global ray variable
	//I use these variables in the debugDraw function to draw the corresponding ray.
	//try it: Press a key, move the camera, see the ray that was launched as a line.
	testRayOrigin=rayOrigin;
	testRayDestination=rayDestination;

	switch(t) {
        case 'i':
            {
                Ray r = {rayOrigin, (rayDestination - rayOrigin)};
                r.direction.normalize();
                calc_inv_direction(r);

                if (intersect_AABB(r, boundingBoxes[0])) cout << "Ray intersected bounding box" << endl;
                else cout << "Ray did not intersect bounding box" << endl;
            }
            break;
        case 't':
            {
                Ray r = {rayOrigin, (rayDestination - rayOrigin)};
                r.direction.normalize();

                Intersection intersect;

                if (intersect_mesh(0, r, Triangle(), boundingBoxes, intersect)) {
                    cout << "Found intersection on point " << intersect.point << endl;
                    testRayDestination = intersect.point;
                }
            }
            break;
        default:
            cout << "You pressed a key, but not one that does something in particular..." << endl;
    }
}
