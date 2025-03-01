import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    config = os.path.join(
        get_package_share_directory('realtime_urdf_filter'), 'config', 'urdf_filter_params.yaml')

    rm_serial_driver_node = Node(
        package='realtime_urdf_filter',
        executable='urdf_filter',
        namespace='',
        output='screen',
        emulate_tty=True
    )

    return LaunchDescription([rm_serial_driver_node])
