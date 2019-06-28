#ifndef inspector_gcs_plugin_H
#define inspector_gcs_plugin_H

// C++
#include <string>
#include <thread>
// ROS
#include <ros/ros.h>
#include "std_msgs/Empty.h"
#include "std_msgs/String.h"
#include <std_msgs/Int8.h>
#include "std_srvs/Empty.h"
#include "geometry_msgs/Twist.h"

#include <nav_msgs/Path.h>

// QT & RQT
#include <rqt_gui_cpp/plugin.h>
#include <inspector_gcs/ui_gcs_plugin.h>
#include <QWidget>

#include <QBasicTimer>
#include <QString>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QButtonGroup>
// #include <QComboBox>
#include <QList>
#include <QVector> 

// #include <QLineF>
// #include <QPolygonF>
// #include <QGeoCoordinate> 

// Services
#include <inspector_gcs/MissionService.h>
#include <inspector_gcs/StbyActionService.h>
#include <inspector_gcs/PausedStActionService.h>
#include <inspector_gcs/StopService.h>
#include <inspector_gcs/gcsCreateMission.h>
#include <inspector_gcs/gcsSendMission.h>

// Messages
#include <inspector_gcs/UavList.h>

// TEST
#include "sensor_msgs/PointCloud2.h"
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TwistStamped.h>
#include "sensor_msgs/Imu.h"
#include "sensor_msgs/NavSatFix.h"

#include <uav_abstraction_layer/ual.h>
#include <uav_abstraction_layer/GoToWaypoint.h>
#include <uav_abstraction_layer/Land.h>
#include <uav_abstraction_layer/TakeOff.h>
#include <uav_abstraction_layer/SetVelocity.h>

namespace inspector_gcs
{

// class cloudSignal : public QObject
// {
//   Q_OBJECT

// public:
//   cloudSignal(Ui::GcsPluginWidget ui)
//   {
//     m_value = 0;
//     ui_ = ui;
//   }

//   int value() const { return m_value; }

// public slots:
//   void setValue(int value);

// signals:
//   void valueChanged(int newValue);

// private:
//   int m_value;
//   Ui::GcsPluginWidget ui_;
// };

class GcsPlugin : public rqt_gui_cpp::Plugin
{
  Q_OBJECT
public:
  // QString qfilelocation;

  // QList<QGeoCoordinate> PolygonCoordinates;
  // QList<QLineF> resultLinesNED;
  // QList<QList<QGeoCoordinate>> resultTransectsGeo;
  // QList<QList<QPointF>> droneWayPointsNED;
  // QList<QList<QGeoCoordinate>> droneWayPointsGeo;
  // std::vector<nav_msgs::Path> missionPaths;
  

//
  GcsPlugin();
  virtual void initPlugin(qt_gui_cpp::PluginContext &context);
  virtual void shutdownPlugin();
  virtual void saveSettings(qt_gui_cpp::Settings &plugin_settings, qt_gui_cpp::Settings &instance_settings) const;
  virtual void restoreSettings(const qt_gui_cpp::Settings &plugin_settings, const qt_gui_cpp::Settings &instance_settings);
  Ui::GcsPluginWidget ui_;
  // --------------------------------------------------------------------------
protected slots:

  virtual void press_CreateMission();
  virtual void press_SendMission();

  virtual void press_StartMission();
  virtual void press_StopMission();
  virtual void press_ResumeMission();
  virtual void press_AbortMission();

  virtual void press_StartMission_2();
  virtual void press_StopMission_2();
  virtual void press_ResumeMission_2();
  virtual void press_AbortMission_2();

  // virtual void on_uav_selection_Box_currentIndexChanged(const QString &arg1);
  // virtual void on_uav_selection_Box_currentIndexChanged(int index);

  virtual void press_takeOff();
  virtual void press_land();
  virtual void press_goToWaypoint();
  virtual void press_setVelocity();

protected:

  // DJI_SDK 
  virtual void gps_pos_cb(const sensor_msgs::NavSatFix);
  // GCS
  virtual void uav_list_cb(const inspector_gcs::UavList);
  // virtual void uav_services_update();
  // ADL
  virtual void adl_state_cb(const std_msgs::String msg);


  // UAL //
  virtual void ual_state_cb(const std_msgs::String msg);
  virtual void pose_callback(const geometry_msgs::PoseStamped);
  virtual void velocity_callback(const geometry_msgs::TwistStamped);
  // UAL //

  // Comment in to signal that the plugin has a way to configure it
  // bool hasConfiguration() const;
  // void triggerConfiguration();

private slots:
  // void on_pushButton_OpenJsonFile_clicked();

  // void on_pushButton_nuevo_clicked();

  // void on_pushButton_CreateMission_clicked();


  // void on_pushButton_SendMission_clicked();


private:
  QWidget *widget_;
  // Node
  ros::NodeHandle n_;
  // cloudSignal* cloudUpdate;

  // Private variables
  QStringList uav_list;

  // Thread
  std::thread gui_thread;
  void guiThread();


  //////////////////////////////////////////
  // Inspector GCS //
  ros::ServiceClient mission_srv, stby_action_srv, paused_st_action_srv, stop_srv;
  inspector_gcs::MissionService mission_service;
  inspector_gcs::StbyActionService stby_action_service;
  inspector_gcs::PausedStActionService paused_state_action_service;
  inspector_gcs::StopService stop_service;

  ros::ServiceClient createMission_srv;
  ros::ServiceClient sendMission_srv;
  inspector_gcs::gcsCreateMission create_mission_service;
  inspector_gcs::gcsSendMission send_mission_service;

  ros::Subscriber uav_list_sub;

  //////////////////////////////////////////

  // Subscribers
    // DJI_SDK
  ros::Subscriber gps_pos_sub;
    // ADL
  ros::Subscriber adl_state_sub;
    // UAL
  ros::Subscriber ual_state_sub, pose_sub, velocity_sub;
  
  // UAL //
  ros::ServiceClient srvTakeOff, srvLand, srvGoToWaypoint, srvSetVelocity;
  uav_abstraction_layer::TakeOff take_off;
  uav_abstraction_layer::Land land;
  uav_abstraction_layer::GoToWaypoint go_to_waypoint;
  uav_abstraction_layer::SetVelocity set_velocity;
  geometry_msgs::TwistStamped vel;
  geometry_msgs::PoseStamped wp;

  // ros::Publisher resolution;
  // --------------------------------------------------------------------------
};
} // namespace inspector_gcs
#endif // inspector_gcs_plugin_H