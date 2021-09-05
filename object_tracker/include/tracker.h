#ifndef OBJECT_TRACKER_H
#define OBJECT_TRACKER_H
#include <iostream>
#include <map>
#include <algorithm>
#include <string>
#include <vector>
#include "vehicle.h"

using std::map;
using std::string;
using std::vector;

class ObjectTracker
{
public:
    bool lbs_occupied;  // left blind spot (from s-10 to s+10)
    bool rbs_occupied;  // right blind spot (from s-10 to s+10)
    int front_id;       // front car id
    int rear_id;        // rear car id
    int front_left_id;  // left front car id
    int rear_left_id;   // left rear car id
    int front_right_id; // right front car id
    int rear_right_id;  // right rear car id
    map<int, Vehicle> objects;

    void update_object(unsigned int now, vector<double> sf_object);
    void analyze_scene(double ego_s, double ego_d);

private:
    int history_ms = 1000; // I only keep 1000 msec of history for each object

    vector<int> occupied_lanes(double d);

    const int BS_FRONT_MARGIN = 10; // margin for blind spot in the back
    const int BS_REAR_MARGIN = 10;  // margin for blind spot in the front
};

#endif