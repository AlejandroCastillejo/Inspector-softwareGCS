#!/usr/bin/env python

import rospy
from sensor_msgs.msg import NavSatFix

def test_publisher():
    uav_gps_pos_pub_1 = rospy.Publisher('uav_2/dji_sdk/gps_position', NavSatFix, queue_size=10)
    uav_gps_pos_pub_2 = rospy.Publisher('uav_3/dji_sdk/gps_position', NavSatFix, queue_size=10)
    rospy.init_node('test_publisher')

    while not rospy.is_shutdown():
        gps_position_1 = NavSatFix()
        gps_position_2 = NavSatFix()
        gps_position_1.latitude = 37.091136
        gps_position_1.longitude =  -5.872402
        uav_gps_pos_pub_1.publish(gps_position_1)
        gps_position_2.latitude = 37.091241
        gps_position_2.longitude =  -5.872474
        uav_gps_pos_pub_2.publish(gps_position_2)
        rospy.sleep(1)


if __name__ == '__main__':
    try:
        test_publisher()
    except rospy.ROSInterruptException:
        pass