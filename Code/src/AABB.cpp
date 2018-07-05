#include <stdio.h>
#include <list>
#include <vector>

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

// function that nicely fits a box around the given set of triangles
void _snug_fit_AABB(AABB & boundingBox, vector<Triangle> & triangles, bool pushTriangles) {
    boundingBox.min = Vec3Df(INFINITY, INFINITY, INFINITY);
    boundingBox.max = Vec3Df(-INFINITY, -INFINITY, -INFINITY);

    for (Triangle t : triangles) {
        Vec3Df v = MyMesh.vertices[t.v[0]].p;
        if (v[0] < boundingBox.min[0]) boundingBox.min[0] = v[0];
        if (v[1] < boundingBox.min[1]) boundingBox.min[1] = v[1];
        if (v[2] < boundingBox.min[2]) boundingBox.min[2] = v[2];
        if (v[0] > boundingBox.max[0]) boundingBox.max[0] = v[0];
        if (v[1] > boundingBox.max[1]) boundingBox.max[1] = v[1];
        if (v[2] > boundingBox.max[2]) boundingBox.max[2] = v[2];

        v = MyMesh.vertices[t.v[1]].p;
        if (v[0] < boundingBox.min[0]) boundingBox.min[0] = v[0];
        if (v[1] < boundingBox.min[1]) boundingBox.min[1] = v[1];
        if (v[2] < boundingBox.min[2]) boundingBox.min[2] = v[2];
        if (v[0] > boundingBox.max[0]) boundingBox.max[0] = v[0];
        if (v[1] > boundingBox.max[1]) boundingBox.max[1] = v[1];
        if (v[2] > boundingBox.max[2]) boundingBox.max[2] = v[2];

        v = MyMesh.vertices[t.v[2]].p;
        if (v[0] < boundingBox.min[0]) boundingBox.min[0] = v[0];
        if (v[1] < boundingBox.min[1]) boundingBox.min[1] = v[1];
        if (v[2] < boundingBox.min[2]) boundingBox.min[2] = v[2];
        if (v[0] > boundingBox.max[0]) boundingBox.max[0] = v[0];
        if (v[1] > boundingBox.max[1]) boundingBox.max[1] = v[1];
        if (v[2] > boundingBox.max[2]) boundingBox.max[2] = v[2];

        if (pushTriangles) boundingBox.triangles.push_back(t);
    }

    return;
}

// wrappers for overloading this function
void snug_fit_AABB(AABB & boundingBox) {
    _snug_fit_AABB(boundingBox, boundingBox.triangles, false);
}

void snug_fit_AABB(AABB & boundingBox, vector<Triangle> & triangles) {
    _snug_fit_AABB(boundingBox, triangles, true);
}

// calculates median of a vector of floats
float get_median(vector<float> & array) {
    if (array.size() == 0) return 0.0f;

    list<float> sortedArray;

    for (float f : array) {
        // save size to check for edge cases later
        unsigned int currentSize = sortedArray.size();

        // find where the number is suppesod to be and insert in the sorted list
        for (list<float>::iterator it=sortedArray.begin(); it != sortedArray.end(); it++) {
            if (f < *it) {
                sortedArray.insert(it, f);
                break;
            }
        }

        // if the number is supposed to go at the end, we need this exception
        if (currentSize == sortedArray.size()) sortedArray.push_back(f);
    }

    // find the middle element of the list
    unsigned int N = array.size()/2 - 1;

    // there must be a nicer way to do this, have to find out at some point
    if (array.size() % 2 == 0) {
        list<float>::iterator it = sortedArray.begin();
        advance(it, N);
        float temp = *it;
        it++;
        return ( (*it + temp)/2 );
    } else {
        list<float>::iterator it = sortedArray.begin();
        advance(it, N + 1);
        return (*it);
    }
}

// calculates mean of a vector of floats
float get_mean(vector<float> & array) {
    double total = 0.0f;

    for (float f : array) {
        total += f;
    }

    return (total/array.size());
}

// refines a given box with triangles into two smaller boxes
bool refine_AABB(AABB & originalBox, AABB & firstBox, AABB & secondBox, unsigned int level, unsigned int limitTriangles) {
    // if number of triangles is too low, return false
    if (originalBox.triangles.size() < limitTriangles) return false;

    unsigned int dim = level % 3;

    // first calculate the median in the desired dimension
    vector<float> borderInput;
    for (Triangle t : originalBox.triangles) {
        borderInput.push_back(MyMesh.vertices[t.v[0]].p[dim]);
    }

    float border = get_mean(borderInput);

    // cout << "Median: " << border << endl;

    // put triangles in either of the two child boxes according to where the majority of the vertices is located
    for (Triangle t : originalBox.triangles) {
        float d[3] = {MyMesh.vertices[t.v[0]].p[dim], MyMesh.vertices[t.v[1]].p[dim], MyMesh.vertices[t.v[2]].p[dim]};

        unsigned int noInsideLeft = 0;

        for (int i=0; i < 3; i++) {
            if (d[i] < border) noInsideLeft++;
        }

        switch (noInsideLeft) {
            case 0:
                secondBox.triangles.push_back(t);
                break;
            case 1:
                secondBox.triangles.push_back(t);
                break;
            case 2:
                firstBox.triangles.push_back(t);
                break;
            case 3:
                firstBox.triangles.push_back(t);
                break;
            default:
                cout << "Fatal error: splitting boxes failed" << endl;
        }
    }

    // cout << "Divided up " << originalBox.triangles.size() << " triangles into " << firstBox.triangles.size() << " triangles and " << secondBox.triangles.size() << " triangles" << endl;

    if (firstBox.triangles.size() == 0 || secondBox.triangles.size() == 0) return false;

    snug_fit_AABB(firstBox);
    snug_fit_AABB(secondBox);

    return true;
}

// initializes the bounding boxes in the scene
void init_AABB(vector<AABB> & boundingBoxes) {
    // set up the original bounding box by checking the bounds of all the triangles in the scene
    AABB initialBox;
    snug_fit_AABB(initialBox, MyMesh.triangles);

    // from this point onwards, refine the boxes to a preset level
    vector<AABB> currentBoxes;
    vector<AABB> refinedBoxes;

    currentBoxes.push_back(initialBox);

    unsigned int oldSize;
    unsigned int level = 0;

    // keep refining until a desired level of detail is reached for the boxes
    do {
        oldSize = currentBoxes.size();
        refinedBoxes.clear();

        // loop over all current boxes and check if they need to be refined
        for (AABB b : currentBoxes) {
            AABB firstBox, secondBox;
            if (refine_AABB(b, firstBox, secondBox, level, 50)) {//round(sqrt(initialBox.triangles.size()/2)))) {
                refinedBoxes.push_back(firstBox);
                refinedBoxes.push_back(secondBox);
            } else {
                refinedBoxes.push_back(b);
            }
        }

        cout << "AABB level " << level << ": Went from " << currentBoxes.size() << " boxes to " << refinedBoxes.size() << " boxes" << endl;

        currentBoxes = refinedBoxes;

        level++;
    } while (oldSize < currentBoxes.size());

    boundingBoxes = refinedBoxes;
}

void draw_AABB(AABB & boundingBox) {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);

	glBegin(GL_LINES);
	glColor3f(0,1,0);
	glVertex3f(boundingBox.min[0], boundingBox.min[1], boundingBox.min[2]);
	glVertex3f(boundingBox.max[0], boundingBox.min[1], boundingBox.min[2]);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0,1,0);
	glVertex3f(boundingBox.min[0], boundingBox.min[1], boundingBox.min[2]);
	glVertex3f(boundingBox.min[0], boundingBox.max[1], boundingBox.min[2]);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0,1,0);
	glVertex3f(boundingBox.min[0], boundingBox.min[1], boundingBox.min[2]);
	glVertex3f(boundingBox.min[0], boundingBox.min[1], boundingBox.max[2]);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0,1,0);
	glVertex3f(boundingBox.max[0], boundingBox.max[1], boundingBox.max[2]);
	glVertex3f(boundingBox.min[0], boundingBox.max[1], boundingBox.max[2]);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0,1,0);
	glVertex3f(boundingBox.max[0], boundingBox.max[1], boundingBox.max[2]);
	glVertex3f(boundingBox.max[0], boundingBox.min[1], boundingBox.max[2]);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0,1,0);
	glVertex3f(boundingBox.max[0], boundingBox.max[1], boundingBox.max[2]);
	glVertex3f(boundingBox.max[0], boundingBox.max[1], boundingBox.min[2]);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0,1,0);
	glVertex3f(boundingBox.max[0], boundingBox.min[1], boundingBox.min[2]);
	glVertex3f(boundingBox.max[0], boundingBox.max[1], boundingBox.min[2]);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0,1,0);
	glVertex3f(boundingBox.max[0], boundingBox.min[1], boundingBox.min[2]);
	glVertex3f(boundingBox.max[0], boundingBox.min[1], boundingBox.max[2]);
	glEnd();

    glBegin(GL_LINES);
	glColor3f(0,1,0);
	glVertex3f(boundingBox.min[0], boundingBox.max[1], boundingBox.min[2]);
	glVertex3f(boundingBox.max[0], boundingBox.max[1], boundingBox.min[2]);
	glEnd();

    glBegin(GL_LINES);
	glColor3f(0,1,0);
	glVertex3f(boundingBox.min[0], boundingBox.max[1], boundingBox.min[2]);
	glVertex3f(boundingBox.min[0], boundingBox.max[1], boundingBox.max[2]);
	glEnd();

    glBegin(GL_LINES);
	glColor3f(0,1,0);
	glVertex3f(boundingBox.min[0], boundingBox.min[1], boundingBox.max[2]);
	glVertex3f(boundingBox.max[0], boundingBox.min[1], boundingBox.max[2]);
	glEnd();

    glBegin(GL_LINES);
	glColor3f(0,1,0);
	glVertex3f(boundingBox.min[0], boundingBox.min[1], boundingBox.max[2]);
	glVertex3f(boundingBox.min[0], boundingBox.max[1], boundingBox.max[2]);
	glEnd();

	glPopAttrib();
}

bool intersect_AABB(const Ray & r, const AABB & b) {
    double tx1 = (b.min[0] - r.origin[0]) * r.inv_direction[0];
    double tx2 = (b.max[0] - r.origin[0]) * r.inv_direction[0];

    double tmin = min(tx1, tx2);
    double tmax = max(tx1, tx2);

    double ty1 = (b.min[1] - r.origin[1]) * r.inv_direction[1];
    double ty2 = (b.max[1] - r.origin[1]) * r.inv_direction[1];

    tmin = max(tmin, min(ty1, ty2));
    tmax = min(tmax, max(ty1, ty2));

    if (tmax < tmin) return false;

    double tz1 = (b.min[2] - r.origin[2]) * r.inv_direction[2];
    double tz2 = (b.max[2] - r.origin[2]) * r.inv_direction[2];

    tmin = max(tmin, min(tz1, tz2));
    tmax = min(tmax, max(tz1, tz2));

    return (tmax >= tmin);
}
