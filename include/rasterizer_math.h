#pragma once

struct Vector2
{
  float x;
  float y;

};


struct Vector3
{
  float x;
  float y;
  float z;

};


struct Camera
{
  Vector3 position; // Position Vector
  Vector3 rotation; // Euler Angles in Degrees

  float fovAngle;   // Angle opening of the camera (field of view)
};

