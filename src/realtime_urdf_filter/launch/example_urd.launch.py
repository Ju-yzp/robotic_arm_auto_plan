import os
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # 获取机器人描述文件的路径
    robot_description_path = os.path.join(
        get_package_share_directory('realtime_urdf_filter'),
        'urdf',
        'a0912.urdf'
    )

    # 定义 ROS2 启动节点
    return LaunchDescription([
        # 发布机器人状态节点
        Node(
            package='robot_state_publisher',  # 使用 robot_state_publisher 包
            executable='robot_state_publisher',
            arguments=[robot_description_path]
        ),


        Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        name='joint_state_publisher_gui',
        arguments=[robot_description_path]
        ),
        # 启动 RViz2
        Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        )
    ])
