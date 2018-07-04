#include <stdio.h>

#include "raytracing.h"

using namespace std;

void calc_inv_direction(Ray & r) {
    r.inv_direction[0] = 1/r.direction[0];
    r.inv_direction[1] = 1/r.direction[1];
    r.inv_direction[2] = 1/r.direction[2];

    return;
}
