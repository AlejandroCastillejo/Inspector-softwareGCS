#include "inspector_gcs/gcs_plugin.h"
#include <inspector_gcs/gcs_services.h>
#include <QtCore>


namespace inspector_gcs
{



GcsPlugin::GcsPlugin()
    : rqt_gui_cpp::Plugin(), widget_(0)
{
  // Constructor is called first before initPlugin function, needless to say.

  // give QObjects reasonable names
  setObjectName("GcsPlugin");
}


void WorkerThread::run() 
{
  std::cout << "WorkerThread running" << std::endl;

  while(ros::ok){
    // std::cout << "updateButtons" << std::endl;
    emit updateButtons();
    ros::Duration(1).sleep();
  }
}

void GcsPlugin::initPlugin(qt_gui_cpp::PluginContext &context)
{
  ROS_INFO("GUI running on Ground Control Station");

  // access standalone command line arguments
  QStringList argv = context.argv();
  // create QWidget
  widget_ = new QWidget();
  // extend the widget with all attributes and children from UI file
  ui_.setupUi(widget_);

  // add widget to the user interface
  context.addWidget(widget_);

  //
  // QGuiApplication app(argc, argv);

  // QQmlApplicationEngine engine;
  // engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

  // return app.exec();

  //////////////////////////////////////////////
  // Inspector GCS //  
  ros::start();

  // initialize variables
  mission_sent = false;
  mission_created = false;
  
    // start thread
  // gui_thread = std::thread(&GcsPlugin::guiThread, this);

  WorkerThread *workerThread = new WorkerThread();
  connect(workerThread, &WorkerThread::updateButtons, this, &GcsPlugin::guiThread);
  connect(workerThread, &WorkerThread::finished, workerThread, &QObject::deleteLater);
  workerThread->start();

    // CONNECT
  connect(ui_.pushButton_EditMissionFile, SIGNAL(pressed()), this, SLOT(press_EditMissionFile()));
  connect(ui_.pushButton_EditCamerasFile, SIGNAL(pressed()), this, SLOT(press_EditCamerasFile()));
  connect(ui_.pushButton_OpenMapView, SIGNAL(pressed()), this, SLOT(press_OpenMapView()));
  
  connect(ui_.pushButton_CreateMission, SIGNAL(pressed()), this, SLOT(press_CreateMission()));
  connect(ui_.pushButton_SendMission, SIGNAL(pressed()), this, SLOT(press_SendMission()));
  connect(ui_.pushButton_StartMission, SIGNAL(pressed()), this, SLOT(press_StartMission()));
  connect(ui_.pushButton_StopMission, SIGNAL(pressed()), this, SLOT(press_StopMission()));
  connect(ui_.pushButton_ResumeMission, SIGNAL(pressed()), this, SLOT(press_ResumeMission()));
  connect(ui_.pushButton_AbortMission, SIGNAL(pressed()), this, SLOT(press_AbortMission()));
  connect(ui_.pushButton_StartMission_2, SIGNAL(pressed()), this, SLOT(press_StartMission_2()));
  connect(ui_.pushButton_StopMission_2, SIGNAL(pressed()), this, SLOT(press_StopMission_2()));
  connect(ui_.pushButton_ResumeMission_2, SIGNAL(pressed()), this, SLOT(press_ResumeMission_2()));
  connect(ui_.pushButton_AbortMission_2, SIGNAL(pressed()), this, SLOT(press_AbortMission_2()));
  
  
  //GCS Services
  createMission_srv = n_.serviceClient<inspector_gcs::gcsCreateMission>("create_mission_service");
  sendMission_srv = n_.serviceClient<inspector_gcs::gcsSendMission>("send_mission_service");

  //GCS Subscribers
  uav_list_sub = n_.subscribe("uav_list", 0, &GcsPlugin::uav_list_cb,this);

  // Inspector GCS // 
  ////////////////////////////////////////////////

}



void GcsPlugin::shutdownPlugin()
{
  // unregister all publishers here
  n_.shutdown();
}

void GcsPlugin::saveSettings(qt_gui_cpp::Settings &plugin_settings,
                             qt_gui_cpp::Settings &instance_settings) const
{
  // instance_settings.setValue(k, v)
}

void GcsPlugin::restoreSettings(const qt_gui_cpp::Settings &plugin_settings,
                                const qt_gui_cpp::Settings &instance_settings)
{
  // v = instance_settings.value(k)
}


//--------
// Inspector GCS //

void GcsPlugin::guiThread()
{
  // ROS_INFO("guiThread");
  std::string uav_id;

  // while (ros::ok()){
    uav_id = ui_.uav_selection_Box->currentText().toStdString();

    pose_sub = n_.subscribe(uav_id + "/ual/pose", 0, &GcsPlugin::pose_callback, this);
    gps_pos_sub = n_.subscribe(uav_id + "/dji_sdk/gps_position", 0, &GcsPlugin::gps_pos_cb, this);
    ual_state_sub = n_.subscribe(uav_id + "/ual/state", 0, &GcsPlugin::ual_state_cb, this);
    adl_state_sub = n_.subscribe(uav_id + "/adl_state", 0, &GcsPlugin::adl_state_cb, this);
    battery_sub = n_.subscribe(uav_id + "/battery_percentage", 0, &GcsPlugin::battery_cb, this);

    if (uav_list.count() == 0) {
      ui_.pushButton_CreateMission->setVisible(false);
    } else {
      ui_.pushButton_CreateMission->setVisible(true);
    }
    if (mission_created) {
      ui_.pushButton_SendMission->setVisible(true);
    } else {
      ui_.pushButton_SendMission->setVisible(false);
    }

    if (uav_id == ""){
      ui_.pushButton_StartMission_2->setVisible(true);
      ui_.pushButton_StopMission_2->setVisible(true);
      ui_.pushButton_ResumeMission_2->setVisible(true);
      ui_.pushButton_AbortMission_2->setVisible(true);
      ui_.getUalState->del();
      ui_.getAdlState->del();
      ui_.getLat->del();
      ui_.getLong->del();
      ui_.getAlt->del();
      ui_.getPosePx->del();
      ui_.getPosePy->del();
      ui_.getPosePz->del();
    } 
    else {
      if (mission_sent && ros::service::exists(uav_id + "/stby_action_service", false)) {
        ui_.pushButton_StartMission_2->setVisible(true);
      } else {
        ui_.pushButton_StartMission_2->setVisible(false);
      }

      if (mission_sent && ros::service::exists(uav_id + "/stop_service", false)) {
        ui_.pushButton_StopMission_2->setVisible(true);
      } else {
        ui_.pushButton_StopMission_2->setVisible(false);
      }

      if (ros::service::exists(uav_id + "/paused_state_action_service", false)) {
        ui_.pushButton_ResumeMission_2->setVisible(true);
        ui_.pushButton_AbortMission_2->setVisible(true);
      } else {
        ui_.pushButton_ResumeMission_2->setVisible(false);
        ui_.pushButton_AbortMission_2->setVisible(false);
      }

      stby_action_service_count = 0;
      stop_service_count = 0;
      paused_state_action_service_count = 0;
      for (int i = 0; i < uav_list.count(); ++i) {
        uav_id = uav_list[i].toStdString();
        if (ros::service::exists(uav_id + "/stby_action_service", false)) {stby_action_service_count ++;}
        if (ros::service::exists(uav_id + "/stop_service", false)) {stop_service_count ++;}
        if (ros::service::exists(uav_id + "/paused_state_action_service", false)) {paused_state_action_service_count ++;}
      }

      if (mission_sent && (stby_action_service_count == uav_list.count()) ) {
        ui_.pushButton_StartMission->setVisible(true);
      } else {
        ui_.pushButton_StartMission->setVisible(false);
      }      
      
      if (mission_sent && (stop_service_count == uav_list.count()) ) {
        ui_.pushButton_StopMission->setVisible(true);
      } else {
        ui_.pushButton_StopMission->setVisible(false);
      }
      
      if (paused_state_action_service_count == uav_list.count()) {
        ui_.pushButton_ResumeMission->setVisible(true);
        ui_.pushButton_AbortMission->setVisible(true);
      } else {
        ui_.pushButton_ResumeMission->setVisible(false);
        ui_.pushButton_AbortMission->setVisible(false);
      }
    }
    // ros::Duration(0.5).sleep();
  // }
}

void GcsPlugin::uav_list_cb(const inspector_gcs::UavList msg)
{
  std::vector<std::string> vector = msg.uavs;
  bool update_uav_list_view = false;

  // ui_.uav_list_view->clear();
  
  QStringList uav_list_2;
  uav_list_2.reserve(vector.size());
  for(int i = 0; i < vector.size(); ++i) {
    uav_list_2 << vector[i].c_str();
  }

  for (int i = 0; i < uav_list_2.count(); ++i) { //add new items to selection Box
    if (!uav_list.contains(uav_list_2[i])) {
      ui_.uav_selection_Box->addItem(uav_list_2[i]);
      update_uav_list_view = true;
    }
  }
  for (int i = 0; i < uav_list.count(); ++i) { //remove items from selection Box
    if (!uav_list_2.contains(uav_list[i])) {
      int index = ui_.uav_selection_Box->findText(uav_list[i]);
      ui_.uav_selection_Box->removeItem(index);
      update_uav_list_view = true;
    }
  }
  // vector.clear();
  uav_list = uav_list_2;
  uav_list_2.clear();

  if (update_uav_list_view) {
    ui_.uav_list_view->clear();
    ui_.uav_list_view->addItems(uav_list);
  }
}

void GcsPlugin::press_EditMissionFile()
{
std::string mission_file_location = ros::package::getPath("inspector_gcs") + "/json_files/mission_file.json";
system(("gedit " + mission_file_location).c_str());
}

void GcsPlugin::press_EditCamerasFile()
{
std::string cameras_file_location = ros::package::getPath("inspector_gcs") + "/json_files/cameras.json";
system(("gedit " + cameras_file_location).c_str());
}

void GcsPlugin::press_OpenMapView()
{
std::string rviz_file_location = ros::package::getPath("inspector_gcs") + "/launch/map_visualization.rviz";
// system("roslaunch inspector_gcs map_visualization.launch");
system(("rosrun rviz rviz -d " + rviz_file_location).c_str());
}

void GcsPlugin::press_CreateMission()
{
  // qDebug() << "create_misssion clicked" << endl;
  ROS_INFO("GUI: create_misssion clicked");

  mission_created = createMission_srv.call(create_mission_service);
}

void GcsPlugin::press_SendMission()
{
  ROS_INFO("GUI: send_misssion clicked");
  mission_sent = sendMission_srv.call(send_mission_service);
} 

void GcsPlugin::press_StartMission()
{
  ROS_INFO("StartMissionAll clicked");
  std::string uav_id;
  stby_action_service.request.stby_action = inspector_gcs::StbyActionService::Request::START_NEW_MISSION;
  
  for (int i = 0; i < uav_list.count(); ++i){
    uav_id = uav_list[i].toStdString();

    stby_action_srv = n_.serviceClient<inspector_gcs::StbyActionService>(uav_id + "/stby_action_service");
    // ros::service::waitForService(uav_id + "/stby_action_service");
    ROS_INFO("Calling StartMission for %s", uav_id.c_str());
    stby_action_srv.call(stby_action_service);
  }
}

void GcsPlugin::press_StopMission()
{
  ROS_INFO("StopMissionAll clicked");
  std::string uav_id;

  for (int i = 0; i < uav_list.count(); ++i){
    uav_id = uav_list[i].toStdString();

    stop_srv = n_.serviceClient<inspector_gcs::StopService>(uav_id + "/stop_service");
    ROS_INFO("Calling StopMission for %s", uav_id.c_str());
    stop_srv.call(stop_service);
  }
}

void GcsPlugin::press_ResumeMission()
{
  ROS_INFO("ResumeMissionAll clicked");
  std::string uav_id;
  paused_state_action_service.request.paused_action = inspector_gcs::PausedStActionService::Request::RESUME_PAUSED_MISSION;

  for (int i = 0; i < uav_list.count(); ++i){
    uav_id = uav_list[i].toStdString();

    paused_st_action_srv = n_.serviceClient<inspector_gcs::PausedStActionService>(uav_id + "/paused_state_action_service");
    ROS_INFO("Calling ResumeMission for %s", uav_id.c_str());
    paused_st_action_srv.call(paused_state_action_service);
  }
}

void GcsPlugin::press_AbortMission()
{
  ROS_INFO("AbortMissionAll clicked");
  std::string uav_id;
  paused_state_action_service.request.paused_action = inspector_gcs::PausedStActionService::Request::START_NEW_MISSION;

  for (int i = 0; i < uav_list.count(); ++i){
    uav_id = uav_list[i].toStdString();

    paused_st_action_srv = n_.serviceClient<inspector_gcs::PausedStActionService>(uav_id + "/paused_state_action_service");
    ROS_INFO("Calling AbortMission for %s", uav_id.c_str());
    paused_st_action_srv.call(paused_state_action_service);
  }
}

void GcsPlugin::press_StartMission_2()
{
  std::string uav_id = ui_.uav_selection_Box->currentText().toStdString();

  stby_action_service.request.stby_action = inspector_gcs::StbyActionService::Request::START_NEW_MISSION; 
  stby_action_srv = n_.serviceClient<inspector_gcs::StbyActionService>(uav_id + "/stby_action_service");

  ROS_INFO("Calling StartMission for %s", uav_id.c_str());
  stby_action_srv.call(stby_action_service);
}

void GcsPlugin::press_StopMission_2()
{
  std::string uav_id = ui_.uav_selection_Box->currentText().toStdString();

  stop_srv = n_.serviceClient<inspector_gcs::StopService>(uav_id + "/stop_service");
  ROS_INFO("Calling StopMission for %s", uav_id.c_str());
  stop_srv.call(stop_service);
}

void GcsPlugin::press_ResumeMission_2()
{
  std::string uav_id = ui_.uav_selection_Box->currentText().toStdString();

  paused_state_action_service.request.paused_action = inspector_gcs::PausedStActionService::Request::RESUME_PAUSED_MISSION;
  paused_st_action_srv = n_.serviceClient<inspector_gcs::PausedStActionService>(uav_id + "/paused_state_action_service");

  ROS_INFO("Calling ResumeMission for %s", uav_id.c_str());
  paused_st_action_srv.call(paused_state_action_service);
}

void GcsPlugin::press_AbortMission_2()
{
  std::string uav_id = ui_.uav_selection_Box->currentText().toStdString();

    paused_state_action_service.request.paused_action = inspector_gcs::PausedStActionService::Request::START_NEW_MISSION;
    paused_st_action_srv = n_.serviceClient<inspector_gcs::PausedStActionService>(uav_id + "/paused_state_action_service");
    
    ROS_INFO("Calling AbortMission for %s", uav_id.c_str());
    paused_st_action_srv.call(paused_state_action_service);
  
  // QMessageBox::StandardButton ret;
  // int ret;
  // ret = QMessageBox::question(this,"Message Title","Something happened. Do you want to do something about it ?", QMessageBox::Ok | QMessageBox::Cancel);
  // if ( ret == QMessageBox::Ok) {
  //   qDebug() << "User clicked on OK";
  //   paused_state_action_service.request.paused_action = inspector_gcs::PausedStActionService::Request::START_NEW_MISSION;
  //   paused_st_action_srv = n_.serviceClient<inspector_gcs::PausedStActionService>(uav_id + "/paused_state_action_service");
  //   ROS_INFO("Calling AbortMission for %s", uav_id.c_str());
  //   paused_st_action_srv.call(paused_state_action_service);
  // } 
  // else if ( ret == QMessageBox::Cancel) {
  //   qDebug() << "User clicked on Cancel";
  // }
}

void GcsPlugin::battery_cb(const sensor_msgs::BatteryState msg)
{
  int percentage = msg.percentage * 100;
  ui_.progressBar_Battery->setValue(percentage);
}

// void GcsPlugin::on_uav_selection_Box_currentIndexChanged(int index)
// {
//   std::string uav_id = ui_.uav_selection_Box->currentText().toStdString();
//   ROS_INFO("selected %s", uav_id.c_str());

// }

// Inspector GCS //

// --------------------------------------------------------------------------
// UAL //
void GcsPlugin::ual_state_cb(const inspector_gcs::UalState msg)
{
  QString txt = QString::fromStdString(ual_states[msg.state]);
  // ui_.label_State->setText(txt);
  ui_.getUalState->setText(txt);
}
void GcsPlugin::adl_state_cb(const std_msgs::String msg)
{
  QString txt = QString::fromStdString(msg.data);
  ui_.getAdlState->setText(txt);
}

void GcsPlugin::gps_pos_cb(const sensor_msgs::NavSatFix msg)
{
  ui_.getLat->setText(QString::number(msg.latitude, 'f', 5));
  ui_.getLong->setText(QString::number(msg.longitude, 'f', 5));
  ui_.getAlt->setText(QString::number(msg.altitude, 'f', 2));
}

void GcsPlugin::pose_callback(const geometry_msgs::PoseStamped msg)
{
  double q_x, q_y, q_z, q_w, siny_cosp, cosy_cosp, yaw;
  ui_.getPosePx->setText(QString::number(msg.pose.position.x, 'f', 2));
  ui_.getPosePy->setText(QString::number(msg.pose.position.y, 'f', 2));
  ui_.getPosePz->setText(QString::number(msg.pose.position.z, 'f', 2));
  // ui_.getPoseOx->setText(QString::number(msg.pose.orientation.x, 'f', 2));
  // ui_.getPoseOy->setText(QString::number(msg.pose.orientation.y, 'f', 2));
  // ui_.getPoseOz->setText(QString::number(msg.pose.orientation.z, 'f', 2));
  // ui_.getPoseOw->setText(QString::number(msg.pose.orientation.w, 'f', 2));
  q_x = msg.pose.orientation.x;
  q_y = msg.pose.orientation.y;
  q_z = msg.pose.orientation.z;
  q_w = msg.pose.orientation.w;
  siny_cosp = +2.0 * (q_w * q_z + q_x * q_y);
  cosy_cosp = +1.0 - 2.0 * (q_y * q_y + q_z * q_z);  
  yaw = atan2(siny_cosp, cosy_cosp) * 180 / 3.14159265;
  ui_.getPoseYaw->setText(QString::number(yaw, 'd', 2));
}

// --------------------------------------------------------------------------

} // namespace inspector_gcs

//PLUGINLIB_DECLARE_CLASS(inspector_gcs, GcsPlugin, inspector_gcs::GcsPlugin, rqt_gui_cpp::Plugin)
PLUGINLIB_EXPORT_CLASS(inspector_gcs::GcsPlugin, rqt_gui_cpp::Plugin)
