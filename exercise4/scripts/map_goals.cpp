//rosrun rqt_reconfigure rqt_reconfigure
//speed_lim_w = 5.7 -> star parameter
//speed_lim_v = 0.8 -> star parameter
//accel_lim_w = 2 -> star parameter
//accel_lim_v = 1 -> star parameter


#include "ros/ros.h"

#include "std_msgs/String.h"
//#include "exercise4/koordinate_markers.h"

#include <nav_msgs/GetMap.h>
#include <geometry_msgs/Quaternion.h>
#include <geometry_msgs/PoseStamped.h>
#include <tf/transform_datatypes.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <visualization_msgs/Marker.h>
#include <geometry_msgs/Pose.h>
#include <math.h>
#include <std_msgs/Int8.h>

# define PI 3.14159265358979323846 /* pi */

using namespace std;
using namespace cv;

Mat cv_map;
float map_resolution = 0;
int size_y;
int size_x;
tf::Transform map_transform;

float array[100][3];
int array_counter = 0;

ros::Subscriber array_sub;

ros::Publisher goal_pub;
ros::Subscriber map_sub;

ros::Publisher arm_pub;


typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

void mapCallback(const nav_msgs::OccupancyGridConstPtr& msg_map) {
    size_x = msg_map->info.width;
    size_y = msg_map->info.height;

    if ((size_x < 3) || (size_y < 3) ) {
        //ROS_INFO("Map size is only x: %d,  y: %d . Not running map to image conversion", size_x, size_y);
        return;
    }

    // resize cv image if it doesn't have the same dimensions as the map
    if ( (cv_map.rows != size_y) && (cv_map.cols != size_x)) {
        cv_map = cv::Mat(size_y, size_x, CV_8U);
    }

    map_resolution = msg_map->info.resolution;
	tf::poseMsgToTF(msg_map->info.origin, map_transform);
    
    const std::vector<int8_t>& map_msg_data (msg_map->data);

    unsigned char *cv_map_data = (unsigned char*) cv_map.data;

    //We have to flip around the y axis, y for image starts at the top and y for map at the bottom
    int size_y_rev = size_y-1;

   for (int y = size_y_rev; y >= 0; --y) {

//        int idx_map_y = size_x * (size_y -y);
  // for (int y = 0; y < size_y; ++y) {

        int idx_map_y = size_x * (size_y -y);
        //int idx_map_y = size_x * y;
        int idx_img_y = size_x * y;

        for (int x = 0; x < size_x; ++x) {

            int idx = idx_img_y + x;

            switch (map_msg_data[idx_map_y + x])
            {
            case -1:
                cv_map_data[idx] = 127;
                break;

            case 0:
                cv_map_data[idx] = 255;
                break;        

            case 100:
                cv_map_data[idx] = 0;
                break;
            }
        }
    }

}



void mouseCallback(int event, int x, int y, int, void* data) {

}

void markersRecieved(const geometry_msgs::Pose msg){
    //ROS_INFO("V MARKERS RECIEVED CALLBACKU");
    array[array_counter][0] = msg.position.x;
    array[array_counter][1] = msg.position.y;
    array[array_counter][2] = msg.position.z;
    ROS_INFO("x=%f y=%f ------- array", array[array_counter][0], array[array_counter][1]);
    ROS_INFO("x=%f y=%f ------- message", msg.position.x, msg.position.y);
    array_counter++;
}

int pozicije_roke [3][7] = {
    {0, 0, 1, 2, 9, 10, 3},
    {3, 3, 4, 5, 9, 10, 6},
    {6, 6, 7, 8, 9, 10, 0}
};

int which = 0;

void kovanec(){
    std_msgs::Int8 msg;

    for(int i = 0; i < 7; i++){
        msg.data = pozicije_roke[which][i];
        arm_pub.publish(msg);
        if (i < 3)
            ros::Duration(1).sleep();
        else
            ros::Duration(1.5).sleep();
    }

    which++;

    if(which>3)
        which = 0;
}

void premikanje() {
    //ros::init(argc, argv, "one_meter");
	MoveBaseClient ac("move_base", true);

    while(!ac.waitForServer(ros::Duration(5.0))){
        //ROS_INFO("Waiting for the move_base action server to come up");
    }

    /* mapa2
    int koordinate [14][3] = {
        {27,219, -90},
        {25,196, -90},
        {22,181, 0},
        {40,168, -120},
        {60,165, 90},
        {60,165, 160},
        {60,165, -130},
        {52,183, -30},
        {52,183, 0},
        {52,183, 180},
        {48,220, 30},
        {25,240, 0},
        {28,246, -140},
        {27,219, 90}
    };*/

    /*
    int koordinate [14][3] = {
        {52,183, -30},
        {52,183, 0},
        {52,183, 180},
        {48,220, 30},
        {25,240, 0},
        {28,246, -140},
        {27,219, 90},
		{27,219, 180},
        {25,196, -90},
        {22,181, 0},
        {40,168, -120},
        {60,165, 90},
        {60,165, 160},
        {60,165, -130}
    };
    */
    int num_of_goals = 20;

    int koordinate [num_of_goals][3] = {
        {52,183, -30},
        {52,183, 0},
        {52,183, 180},
        {41, 198, -10},
        {41, 198, 100},
        {55,220, -45},
        {55,220, 40},
        {27,243, -45},
        {27,243, -90},
        {27,243, -140},
        {27,243, -220},
        {27,219, 90},
		{27,219, 180},
        {25,196, -90},
        {22,181, 0},
        {32, 168, 90},
        {40,168, -120},
        {60,165, 90},
        {60,165, 160},
        {60,165, -130}
    };


    // i < dolzina int koordinate[]
    int i = 0;

    //float pi = 3.14159265358979323846;

    while(i < num_of_goals) {

		int x = koordinate[i][0];
		int y = koordinate[i][1];
        float kot = koordinate[i][2]*PI/180;
		

        //ROS_INFO("neki4");
    
        tf::Point pt((float)x * map_resolution, (float)(size_y-y) * map_resolution, 0.0);
        tf::Point transformed = map_transform * pt;
        ROS_INFO("Moving to (x: %f, y: %f), i = %d", transformed.x(), transformed.y(), i);
        //ROS_INFO("Moving to (x: %d, y: %d), i = %d", x, y, i); 

        if (transformed.x() == 0.0 && transformed.y() == 0.0) {
            break;
        }

        //image_converter_23809_1522578337739
        //image_converter_3935_1522578647698


        move_base_msgs::MoveBaseGoal goal;
        goal.target_pose.header.stamp = ros::Time::now();
        goal.target_pose.header.frame_id = "map";
        goal.target_pose.pose.orientation.x = 0;
        goal.target_pose.pose.orientation.y = 0;
        goal.target_pose.pose.orientation.z = 1*sin(kot/2);
        goal.target_pose.pose.orientation.w = cos(kot/2);
        goal.target_pose.pose.position.x = transformed.x();
        goal.target_pose.pose.position.y = transformed.y();

        
        //ROS_INFO("Sending goal");
        ac.sendGoal(goal);
        
        ac.waitForResult();


        if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
                //ROS_INFO("Hooray, the base moved");
                //system("rosrun sound_play say.py 'position'");
                //i++;
                ros::Duration(0.75).sleep();
               
        }
        else{
            //ROS_INFO("The base failed to move");
        }
        i++;
        if (i >= num_of_goals){
            system("rosnode kill /cylinder_segmentation");
        }
    }
    
    //ros::spinOnce();

	return;
}

void pozdravljanje() {
    //ros::init(argc, argv, "one_meter");
	MoveBaseClient ac("move_base", true);


    while(!ac.waitForServer(ros::Duration(5.0))){
        //ROS_INFO("Waiting for the move_base action server to come up");
    }

    // i < dolzina int koordinate[]
    int i = 0;
    while(i < array_counter) {

        ROS_INFO("grem pozdravljat %d od %d", i, array_counter);
		float x = array[i][0];
		float y = array[i][1];
        float kot = array[i][2]; 
        ROS_INFO("pozdravljam iz map_goals x = %f", x);
        ROS_INFO("pozdravljam iz map_goals y = %f", y);

        move_base_msgs::MoveBaseGoal goal;
        goal.target_pose.header.stamp = ros::Time::now();
        goal.target_pose.header.frame_id = "map";
        goal.target_pose.pose.orientation.x = 0;
        goal.target_pose.pose.orientation.y = 0;
        goal.target_pose.pose.orientation.z = 1*sin(kot/2);
        goal.target_pose.pose.orientation.w = cos(kot/2);
        goal.target_pose.pose.position.x = x;
        goal.target_pose.pose.position.y = y;

        
        ROS_INFO("grem pozdravljat");
        ac.sendGoal(goal);
        
        ac.waitForResult();


        if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
                //ROS_INFO("Hooray, the base moved");
                kovanec();
                system("rosrun sound_play say.py 'hello circle'");
                //i++;
        }
        else{
            system("rosrun sound_play say.py 'failed to reach goal'");
            //i++;
            //ROS_INFO("The base failed to move");
        }
        i++;
        if (i >= array_counter && array_counter != 0){
            system("rosnode kill /map_goals");
        }
	}

	return;
}

int main(int argc, char** argv) {

    ros::init(argc, argv, "map_goals");
    ros::NodeHandle n;
    // nav_msgs::OccupancyGrid map;

    map_sub = n.subscribe("map", 10, &mapCallback);
	goal_pub = n.advertise<geometry_msgs::PoseStamped>("goal", 10);

    ros::NodeHandle n2;
    array_sub = n2.subscribe("/markers2",1000,markersRecieved);
    //ROS_INFO("array_sub subscriban");

    ros::NodeHandle n3;
    arm_pub = n3.advertise<std_msgs::Int8>("set_manipulator_position", 10);
    
    //namedWindow("Map");

    //setMouseCallback("Map", mouseCallback, NULL);

    //sleep(5);
    

    while(ros::ok()) {

        //if (!cv_map.empty()) imshow("Map", cv_map);
		
		premikanje();
        ros::spinOnce();
        //ROS_INFO("KONCAL Z PREISKOVANJEM> POZDRAVLJANJE");
        pozdravljanje();
        
        //break;
    }
    return 0;

}