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
vector<TestRay> testRays;

bool drawTestRays;

void init()
{
    // set up globals
    drawTestRays = false;

    // load mesh
    cout << "Loading mesh..." << endl;
    MyMesh.loadMesh("mug.obj", true);
	MyMesh.computeVertexNormals();

    MyLightPositions.push_back(Vec3Df(-0.6,0.7,0));
	MyLightPositionPower.push_back(350.0f);
	MyLightPositions.push_back(Vec3Df(-0.1,0.9,0.5));
	MyLightPositionPower.push_back(150.0f);

    cout << "Calculating bounding boxes..." << endl;
    init_AABB(boundingBoxes);
}

void compute_shading(unsigned int level, const Ray & origRay, const Intersection & intersect, Vec3Df & color) {
	Vec3Df lightIntensity, reflectedColor, refractedColor, specularLuminance;
	lightIntensity = Vec3Df(0,0,0);
	reflectedColor = Vec3Df(0,0,0);
	refractedColor = Vec3Df(0,0,0);
	Ray reflectedRay, refractedRay;

	bool computeDirect, computeReflect, computeRefract, computeSpecular;
	computeDirect = computeReflect = computeRefract = computeSpecular = false;
	Vec3Df reflectance = Vec3Df(0,0,0);
	Vec3Df refractance = Vec3Df(0,0,0);
	Vec3Df diffusiveness = Vec3Df(1,1,1);

	// determine which contributions need to be computed for which materials
	switch (intersect.material.illum()) {
		// color on, ambient on (basically only diffuse)
		case 1:
			computeDirect = true;
			break;
		// highlights on (diffuse + specular highlights)
		case 2:
			computeDirect = true;
			computeSpecular = true;
			break;
		// pure mirror
		case 3:
			computeDirect = true;
			computeReflect = true;
			computeSpecular = true;
			reflectance = intersect.material.Ka();
			diffusiveness = Vec3Df(1,1,1) - reflectance;
			break;
		// general glossy material (reflection/refraction, ray trace on, fresnel off)
		// TODO: look at partially opaque materials (frosted glass etc)
		case 6:
			{
				computeReflect = true;
				computeRefract = true;
				computeSpecular = true;
				diffusiveness = Vec3Df(0,0,0);
				reflectance = intersect.material.Ka();

				float R0 = (intersect.material.Ni() - 1.0f) / (intersect.material.Ni() + 1.0f);
				R0 *= R0;
				float sc = (1 - intersect.schlickCosTheta);
				float schlickReflectance = R0 + (1 - R0)*sc*sc*sc*sc*sc;

				refractance = Vec3Df(1,1,1) - reflectance;

				reflectance += refractance*schlickReflectance;
				refractance *= (1 - schlickReflectance);
				break;
			}
		// by default compute nothing and give a warning in the terminal
		default:
			printf("Warning: unknown material type %d, please check...", intersect.material.illum());
			break;
	}

	if (computeDirect) compute_direct_light(intersect, boundingBoxes, lightIntensity);

	// if ((computeReflect || computeSpecular) && level + 1 <= RECURSION_LEVEL) {
	// 	if (ComputeReflectedRay(origRay, intersect.point, intersect.triangle, reflectedRay)) {
	// 		if (computeReflect) Trace(level + 1, reflectedRay, reflectedColor, intersect.triangle);
	// 		if (computeSpecular) {
	// 			Vec3Df normal = calculateSurfaceNormal(intersect.triangle);
    //
	// 			specularLuminance = specularFunction(intersect.point, normal, intersect.material);
	// 			//BounceLight(reflectedRay, specularLuminance, intersect.triangle);
	// 		}
	// 	}
	// }
    //
	// if (computeRefract && level + 1 <= RECURSION_LEVEL) {
	// 	if (ComputeRefractedRay(origRay, intersect, refractedRay)) Trace(level + 1, refractedRay, refractedColor, intersect.triangle);
	// }

    diffusiveness = Vec3Df(1,1,1);

	// TODO: figure out proper reflectance/specular usage, both should be handled differently
	color = diffusiveness*intersect.material.Kd()*lightIntensity + specularLuminance*intersect.material.Ks() + reflectance*reflectedColor + refractance*refractedColor;

	for (unsigned int i=0; i < 3; i++) {
		if (color.p[i] > 1.0) color.p[i] = 1.0f;
	}

	if (level == RECURSION_LEVEL) {
		for (unsigned int i=0; i < 3; i++) color.p[i] = 0.5f;
	}

	if (drawTestRays && level == 0) std::cout << "Got color " << color << " from level " << level << std::endl;

	return;
}

void trace_ray(unsigned int level, Ray & ray, Vec3Df & color, const Triangle & ignoreTriangle) {
	Intersection intersect;

	if (intersect_mesh(level, ray, ignoreTriangle, boundingBoxes, intersect)) {
		// if (drawTestRays) {
		// 	recurseTestRayOrigins[recurseTestRayCount] = ray.origin;
		// 	recurseTestRayDestinations[recurseTestRayCount] = intersect.point;
		// 	recurseTestRayCount++;
        //
		// 	cout << "  Traced a ray on level " << level << " from " << recurseTestRayOrigins[recurseTestRayCount - 1] << " to " << recurseTestRayDestinations[recurseTestRayCount - 1] << ". Travelled " << intersect.distance << endl;
		// }

		compute_shading(level, ray, intersect, color);
	} else {
		color = Vec3Df(0,0,0);
	}

	return;
}

float intensity_falloff(const float &distance, const float &power, const float &minimum) {
    // http://www.softschools.com/formulas/physics/inverse_square_law_formula/82/
	double intensity = 1 / (4 * M_PI * distance * distance * (1 / power) + 1);
	if (intensity > minimum) {
		return intensity;
	}
	else {
		return minimum;
	}
}

void compute_direct_light(const Intersection & intersect, const vector<AABB> & boundingBoxes, Vec3Df & intensity) {
    intensity = Vec3Df(0,0,0);

	for (unsigned int x = 0; x < MyLightPositions.size(); x++) {
		float minDist = INFINITY;
        float power = MyLightPositionPower[x];

		Ray r = {intersect.point, MyLightPositions[x] - intersect.point};
        float lightIntersectDist = r.direction.getLength();
        r.direction.normalize();
        calc_inv_direction(r);

        for (AABB box : boundingBoxes) {
            if (intersect_AABB(r, box)) {
                for (Triangle t : box.triangles) {
                    Vec3Df intersectionPoint;
                    float distance;
                    // if an intersection gets found, put the resulting point and triangle in the result vars
                    if (intersect_triangle(r, t, intersect.triangle, intersectionPoint, distance)) {
                        // check if distance is smaller than the distance to the light and larger than zero
                        if (distance < lightIntersectDist && distance > 0) goto nextsource;
                    }
                }
            }
        }

		{
			Vec3Df triangleNormal = compute_surface_normal(intersect.triangle);
			triangleNormal.normalize();
			intensity += Vec3Df(1,1,1)*intensity_falloff(lightIntersectDist, power, 0.0f)*fabs(Vec3Df::dotProduct(triangleNormal, r.direction));
		}

		nextsource:
			;
	}

    return;
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
