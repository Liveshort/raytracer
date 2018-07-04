#include <stdio.h>
#include <list>
#include <vector>

#include "raytracing.h"

// Moller Trumbore method
bool intersect_triangle(Ray & r, Triangle & t, Vec3Df & point) {
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
    float a = Vec3Df::dotProduct(E2, Q)*inv_det;

    // determine point of intersect from t
    point = r.origin + a*r.direction;

    return true;
}
