#include <uWS/uWS.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <eigen3/Eigen/Dense>
#include "helpers.h"
#include "json.hpp"
#include "pred_helpers.h"
#include "classifier.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::vector;

// for convenience
using nlohmann::json;

int main()
{
    // training the classifier module used for prediction
    vector<vector<double>> X_train = Load_State("../prediction/data/train_states.txt");
    vector<vector<double>> X_test = Load_State("../prediction/data/test_states.txt");
    vector<string> Y_train = Load_Label("../prediction/data/train_labels.txt");
    vector<string> Y_test = Load_Label("../prediction/data/test_labels.txt");

    // Since we are not using the test data in this project,
    // I use both train and test data for training
    X_train.insert(X_train.end(), X_test.begin(), X_test.end());
    Y_train.insert(Y_train.end(), Y_test.begin(), Y_test.end());

    // remove the first column (s) from the training data since it
    // does not matter in this case
    double lane_width = 4.0;

    vector<vector<double>> new_X_train;
    for (size_t i = 0; i < X_train.size(); i++)
    {
        double d = X_train[i][2];
        while (d >= lane_width)
            d -= lane_width;
        vector<double> data = {X_train[i][1], d, X_train[i][3]};
        new_X_train.push_back(data);
    }

    vector<vector<double>> new_X_test;
    for (size_t i = 0; i < X_test.size(); i++)
    {
        double d = X_test[i][2];
        while (d >= lane_width)
            d -= lane_width;
        vector<double> data = {X_test[i][1], d, X_test[i][3]};
        new_X_test.push_back(data);
    }

    GNB gnb = GNB(new_X_train[0].size());
    // gnb.train(X_train, Y_train);
    gnb.train(new_X_train, Y_train);

    // int score = 0;
    // for (size_t i = 0; i < X_test.size(); ++i)
    // {
    //     vector<double> coords = new_X_test[i];
    //     string predicted = gnb.predict(coords);
    //     if (predicted.compare(Y_test[i]) == 0)
    //     {
    //         score += 1;
    //     }
    //     else
    //     {
    //         std::cout << coords[0] << " " << coords[1] << " " << coords[2] << " " << Y_test[i] << " " << predicted << std::endl;
    //     }
    // }

    // float fraction_correct = float(score) / Y_test.size();
    // cout << "You got " << (100 * fraction_correct) << " correct" << endl;

    /*
    3.85571 1.85377 -0.00121711 left keep
    3.79661 1.3316 -0.250598 left keep
    3.84321 2.7517 0.0509706 right keep
    3.87752 2.06317 -0.182705 left keep
    0.745291 1.45959 0.126613 right keep
    8.38098 0.392378 -0.244422 left keep
    7.60718 2.23067 -0.0316146 left keep
    3.93369 0.761905 0 right keep
    0.100461 3.11442 0.377205 keep right
    7.5123 3.03248 -0.210782 left keep
    7.86942 1.02242 -0.041073 left keep
    3.38572 2.17459 -0.264937 left keep
    8.10905 0.93527 -0.0673411 left keep
    0.389043 2.43149 0.000745116 right keep
    4.11586 0.247316 -0.0429188 left keep
    4.07807 0.52905 -0.097825 left keep
    4.30126 0.799575 0.23584 right keep
    3.63453 0.803032 -0.248826 left keep
    0.630686 1.80231 0.153005 right keep
    -0.143644 2.12424 0.273786 right keep
    8.05263 3.11649 -0.512923 keep left
    7.7451 1.98236 -0.0250004 left keep
    7.78392 2.34924 -0.192519 left keep
    -0.750311 1.89452 0.122721 right keep
    0.185427 0.34335 0.395668 keep right
    8.76734 3.21164 0.572518 keep right
    7.817 2.57645 -0.0989045 left keep
    8.06871 0.694288 -0.222771 left keep
    8.52363 1.56877 -0.0760171 left keep
    3.96915 1.94976 -0.0659206 left keep
    7.97352 0.271012 0.428962 right keep
    4.09728 1.65265 -0.37946 keep left
    3.22569 2.78406 0.213099 right keep
    0.0941329 2.09891 0 right keep
    7.98319 3.20852 -0.410937 keep left
    8.57856 2.83934 0 left keep
    8.32933 1.59098 -0.0302153 left keep
    -0.00949 0.751981 0.0329068 right keep
    -0.12055 2.60188 0.425395 keep right
    */

    uWS::Hub h;

    // Load up map values for waypoint's x,y,s and d normalized normal vectors
    vector<double> map_waypoints_x;
    vector<double> map_waypoints_y;
    vector<double> map_waypoints_s;
    vector<double> map_waypoints_dx;
    vector<double> map_waypoints_dy;

    // Waypoint map to read from
    string map_file_ = "../data/highway_map.csv";
    // The max s value before wrapping around the track back to 0
    double max_s = 6945.554;

    std::ifstream in_map_(map_file_.c_str(), std::ifstream::in);

    string line;
    while (getline(in_map_, line))
    {
        std::istringstream iss(line);
        double x;
        double y;
        float s;
        float d_x;
        float d_y;
        iss >> x;
        iss >> y;
        iss >> s;
        iss >> d_x;
        iss >> d_y;
        map_waypoints_x.push_back(x);
        map_waypoints_y.push_back(y);
        map_waypoints_s.push_back(s);
        map_waypoints_dx.push_back(d_x);
        map_waypoints_dy.push_back(d_y);
    }

    h.onMessage([&map_waypoints_x, &map_waypoints_y, &map_waypoints_s,
                 &map_waypoints_dx, &map_waypoints_dy](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length,
                                                       uWS::OpCode opCode)
                {
                    // "42" at the start of the message means there's a websocket message event.
                    // The 4 signifies a websocket message
                    // The 2 signifies a websocket event
                    if (length && length > 2 && data[0] == '4' && data[1] == '2')
                    {

                        auto s = hasData(data);

                        if (s != "")
                        {
                            auto j = json::parse(s);

                            string event = j[0].get<string>();

                            if (event == "telemetry")
                            {
                                // j[1] is the data JSON object

                                // Main car's localization Data
                                double car_x = j[1]["x"];
                                double car_y = j[1]["y"];
                                double car_s = j[1]["s"];
                                double car_d = j[1]["d"];
                                double car_yaw = j[1]["yaw"];
                                double car_speed = j[1]["speed"];

                                // Previous path data given to the Planner
                                auto previous_path_x = j[1]["previous_path_x"];
                                auto previous_path_y = j[1]["previous_path_y"];
                                // Previous path's end s and d values
                                double end_path_s = j[1]["end_path_s"];
                                double end_path_d = j[1]["end_path_d"];

                                // Sensor Fusion Data, a list of all other cars on the same side
                                //   of the road.
                                // The data format for each car is: [ id, x, y, vx, vy, s, d]. The id is a unique identifier for that car. The x, y values are in global map coordinates, and the vx, vy values are the velocity components, also in reference to the global map. Finally s and d are the Frenet coordinates for that car. The vx, vy values can be useful for predicting where the cars will be in the future. For instance, if you were to assume that the tracked car kept moving along the road, then its future predicted Frenet s value will be its current s value plus its (transformed) total velocity (m/s) multiplied by the time elapsed into the future (s).
                                auto sensor_fusion = j[1]["sensor_fusion"];

                                json msgJson;

                                vector<double> next_x_vals;
                                vector<double> next_y_vals;

                                /**
           * TODO: define a path made up of (x,y) points that the car will visit
           *   sequentially every .02 seconds
           */
                                double dist_inc = 0.5;
                                for (int i = 0; i < 50; ++i)
                                {
                                    next_x_vals.push_back(car_x + (dist_inc * i) * cos(deg2rad(car_yaw)));
                                    next_y_vals.push_back(car_y + (dist_inc * i) * sin(deg2rad(car_yaw)));
                                }
                                msgJson["next_x"] = next_x_vals;
                                msgJson["next_y"] = next_y_vals;

                                auto msg = "42[\"control\"," + msgJson.dump() + "]";

                                ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
                            } // end "telemetry" if
                        }
                        else
                        {
                            // Manual driving
                            std::string msg = "42[\"manual\",{}]";
                            ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
                        }
                    } // end websocket if
                });   // end h.onMessage

    h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req)
                   { std::cout << "Connected!!!" << std::endl; });

    h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code,
                           char *message, size_t length)
                      {
                          ws.close();
                          std::cout << "Disconnected" << std::endl;
                      });

    int port = 4567;
    if (h.listen(port))
    {
        std::cout << "Listening to port " << port << std::endl;
    }
    else
    {
        std::cerr << "Failed to listen to port" << std::endl;
        return -1;
    }

    h.run();
}