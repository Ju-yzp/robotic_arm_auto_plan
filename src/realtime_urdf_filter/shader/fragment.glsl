#version 330 core

layout (location = 0) out vec4 SensorDepth
layout (location = 1) out vec4 OpenglDepth
layout (location = 2) out vec4 Normal
layout (location = 3) out vec4 DiffDepth

uniform float z_far
uniform float z_near
uniform float max_diff
uniform float replace_value

uniform int width
uniform int height
uniform int depth_texture

float to_linear_depth(float d)
{
    return (z_near * z_far / (z_near - z_far))/(d-z_far/(z_far - z_near));
}

void main()
{  
  float sensor_depth = texelFetch (depth_texture, int(gl_FragCoord.y)*width + int(gl_FragCoord.x)).x;
  float virtual_depth = to_linear_depth (gl_FragCoord.z);
  float should_filter = float(sensor_depth > (virtual_depth - max_diff)); 

  // first color attachment: sensor depth image
  SensorDepth = vec4 (sensor_depth, sensor_depth, sensor_depth, 1.0);

  // second color attachment: opengl depth image
  OpenglDepth =  mix(vec4(sensor_depth, sensor_depth, sensor_depth, 1.0), vec4(replace_value, 0.0, 0.0, 1.0), should_filter);

  // third color attachment: normal visualization
  Normal = normal * 0.5 + 0.5;

  // fourth color attachment: difference image
  DiffDepth = vec4(should_filter, should_filter, should_filter, 0.0);
}
