#ifndef RAYTRACING_H_1
#define RAYTRACING_H_1
#include <vector>
#include "mesh.h"

#define RECURSION_LEVEL 1

extern Mesh MyMesh; //Main mesh
extern std::vector<Vec3Df> MyLightPositions;
extern std::vector<float> MyLightPositionPower;
extern Vec3Df MyCameraPosition; //currCamera
extern unsigned int WindowSize_X;//window resolution width
extern unsigned int WindowSize_Y;//window resolution height
extern unsigned int RayTracingResolutionX;  // largeur fenetre
extern unsigned int RayTracingResolutionY;  // largeur fenetre

// init function to set everything up
void init();

// final trace function that produces the image
void produceRay(int x_I, int y_I, Vec3Df & origin, Vec3Df & dest);

//your main function to rewrite
Vec3Df performRayTracing(const Vec3Df & origin, const Vec3Df & dest);

//a function to debug --- you can draw in OpenGL here
void yourDebugDraw();

//want keyboard interaction? Here it is...
void yourKeyboardFunc(char t, int x, int y, const Vec3Df & rayOrigin, const Vec3Df & rayDestination);

// axis aligned bounding boxes, rays, intersections
struct AABB {
    Vec3Df min;
    Vec3Df max;
    std::vector<Triangle> triangles;
};

struct Ray {
    Vec3Df origin;
    Vec3Df direction;
    Vec3Df inv_direction;
};

struct TestRay {
    Vec3Df origin;
    Vec3Df destination;
};

struct Intersection {
	Vec3Df point;
	Triangle triangle;
	Material material;
	float distance;
	float schlickCosTheta;
};

// important shared functions
void calc_inv_direction(Ray & r);

void init_AABB(std::vector<AABB> & boundingBoxes);
void draw_AABB(AABB & boundingBox);
bool intersect_AABB(const Ray & r, const AABB & b);

Vec3Df compute_surface_normal(const Triangle & t);
bool intersect_triangle(const Ray & r, const Triangle & t, const Triangle & ignoreTriangle, Vec3Df & point, float & distance);
bool intersect_mesh(const unsigned int level, Ray & r, const Triangle & ignoreTriangle, const std::vector<AABB> & boundingBoxes, Intersection & intersect);

void compute_direct_light(const Intersection & intersect, const std::vector<AABB> & boundingBoxes, Vec3Df & intensity);
void trace_ray(unsigned int level, Ray & ray, Vec3Df & color, const Triangle & ignoreTriangle);
void compute_shading(unsigned int level, const Ray & origRay, const Intersection & intersect, Vec3Df & color);

#endif
