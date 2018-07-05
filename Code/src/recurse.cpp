#include <stdio.h>
#include <list>
#include <vector>

#include "raytracing.h"

using namespace std;

// triangle normal is the average of the vertex normals
Vec3Df compute_surface_normal(const Triangle & t) {
	Vec3Df U = MyMesh.vertices[t.v[1]].p - MyMesh.vertices[t.v[0]].p;
	Vec3Df V = MyMesh.vertices[t.v[2]].p - MyMesh.vertices[t.v[0]].p;

	float normalX = U.p[1] * V.p[2] - U.p[2] * V.p[1];
	float normalY = U.p[2] * V.p[0] - U.p[0] * V.p[2];
	float normalZ = U.p[0] * V.p[1] - U.p[1] * V.p[0];

	return Vec3Df(normalX, normalY, normalZ);
}

// Moller Trumbore method
bool intersect_triangle(const Ray & r, const Triangle & t, const Triangle & ignoreTriangle, Vec3Df & point, float & distance) {
    if (t == ignoreTriangle) return false;

    float eps = 0.00001;

    // translate the triangle to the origin
    Vec3Df E1 = MyMesh.vertices[t.v[1]].p - MyMesh.vertices[t.v[0]].p;
    Vec3Df E2 = MyMesh.vertices[t.v[2]].p - MyMesh.vertices[t.v[0]].p;

    // calculate determinant of solution matrix
    Vec3Df P = Vec3Df::crossProduct(r.direction, E2);
    float det = Vec3Df::dotProduct(P, E1);

    // if determinant is near zero, no solution
    if (det < eps && det > -eps) return false;

    float inv_det = 1.0f / det;

    // set up some vectors we will need later
    Vec3Df T = r.origin - MyMesh.vertices[t.v[0]].p;
    Vec3Df Q = Vec3Df::crossProduct(T, E1);

    // calculate barycentric coordinate u and check bounds
    float u = Vec3Df::dotProduct(T, P)*inv_det;
    if (u < 0.0f || u > 1.0f) return false;
    // do the same for v
    float v = Vec3Df::dotProduct(r.direction, Q)*inv_det;
    if (v < 0.0f || u + v > 1.0f) return false;

    // finally calculate t
    distance = Vec3Df::dotProduct(E2, Q)*inv_det;

    // determine point of intersect from t
    point = r.origin + distance*r.direction;

    return true;
}

// returns whether the ray hit something or not
bool intersect_mesh(const unsigned int level, Ray & r, const Triangle & ignoreTriangle, const vector<AABB> & boundingBoxes, Intersection & intersect) {
	intersect.distance = INFINITY;
    calc_inv_direction(r);

    unsigned counter = 0;

	for (AABB box : boundingBoxes) {
        if (intersect_AABB(r, box)) {
            counter++;
            for (Triangle t : box.triangles) {
                Vec3Df intersectionPoint;
                float distance;
                // if an intersection gets found, put the resulting point and triangle in the result vars
                if (intersect_triangle(r, t, ignoreTriangle, intersectionPoint, distance)) {
                    // check if distance is smaller than previous result and larger than zero
                    if (distance < intersect.distance && distance > 0) {
                        intersect.point = intersectionPoint;
                        intersect.triangle = t;
                        intersect.distance = distance;
                        intersect.material = MyMesh.materials[MyMesh.triangleMaterials[t.i]];

                        Vec3Df n = compute_surface_normal(t);
                        intersect.schlickCosTheta = fabs(Vec3Df::dotProduct(n, r.direction));
                    }
                }
            }
        }
    }

    // cout << "Intersected with " << counter << " boxes" << endl;

	// if (intersect.distance < 1000000 && level == 0 && drawRecurseRays) {
	// 	Vec3Df intensity = getLit(intersect.point, intersect.triangle);
	// 	std::cout << "Intensity at point " << intersect.point << " is " << intensity << std::endl;
	// }

	if (intersect.distance < 1000000) return true;
	else return false;
}
