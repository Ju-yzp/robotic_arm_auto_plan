# realtime_urdf_filter

## Dependencies

### Core Libraries

| Library | Version | Installation |
|---------|---------|--------------|
| [![GLFW](https://img.shields.io/badge/GLFW-3.3+-89BFFF?logo=glfw)](https://www.glfw.org) | ≥ 3.3 | `sudo apt install libglfw3-dev` |
| [![GLM](https://img.shields.io/badge/GLM-0.9.9+-7DCAAD)](https://github.com/g-truc/glm) | ≥ 0.9.9 | Header-only (copy to `/usr/local/include`) |


### ROS2 Enviroment

- **OS**: Ubuntu 22.04
- **ROS Distro**: [Humble Hawksbill](https://docs.ros.org/en/humble/)
- **Build Tool**: Colcon


## ROS2 Node

There are two ROS2 nodes that can be used out of the box:

- realtime_urdf_filter

- urdf_filtered_tracker

## Quick Start

Open a terminal in the source directory of workspace and input the following commands one by one:

```bash
colcon build --packages-select realtime_urdf_filter
source install/setup.bash
ros2 launch realtime_urdf_filter test.launch.py
```

## Core Maintainers 

This project is maintained by the following individuals. We welcome you to contact us at any time for support or feedback.

| Name | Contact |
|------|---------|
| **Jup** | [![GitHub](https://img.shields.io/badge/GitHub-Ju--yzp-181717?logo=github)](https://github.com/Ju-yzp) <br> 📧 Ju230551@outlook.com |





