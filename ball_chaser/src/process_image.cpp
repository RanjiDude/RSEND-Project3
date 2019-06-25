#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
	srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the safe_move service and pass the requested joint angles
	if (!client.call(srv))
		ROS_ERROR("Failed to call service command_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    int white_pixel = 255;
    
    // Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    int left = 0, mid = 0, right = 0, i = 0;

    for (int row=0; row<img.height; row++)
    {
        for (int step=0; step<img.step; step+=3)
        {
            i = (row * img.step) + step;
            if (img.data[i] == 255 && img.data[i+1] == 255 && img.data[i+2] ==  255)
            {
                if ((step/3) <= img.width * 0.33)
                {
                    left += 1;
                }
                else if ((step/3) > img.width * 0.33 && (step/3) <= img.width * 0.67)
                {
                    mid += 1;
                }
                else if ((step/3) > img.width * 0.67 && (step/3) <= img.width)
                {
                    right += 1;
                }
            }
            
        }
    }

    ROS_INFO("left: %i, mid: %i, right: %i", left, mid, right);

    if (left > mid && left > right)
    // More white pixels in the left section of the image
    {
        drive_robot(0.1, 0.2);
    }
    else if (mid >= left && mid >= right and mid != 0)
    // More white pixels in the middle section of the image
    {
        drive_robot (0.5, 0.0);
    }
    else if (right > left && right > mid)
    // More white pixels in the right section of the image
    {
        drive_robot(0.1, -0.2);
    }
    else if (left == 0 and mid == 0 and right == 0)
    {
        drive_robot(0.0, 0.0);
    }

}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;
    
    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/drive_bot/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}