#pragma once

#include "glm\glm.hpp"

namespace sr
{
	class Input;
	class Camera;

	/*
	   Responsible for mapping keyboard/mouse input onto the control of the camera.
	*/
	class CameraController
	{
	public:
		CameraController(Input&, Camera&);
		void Update();

	private:
		glm::vec3 MovementDirection();

	private:
		Input& input;
		Camera& camera;
	};
}
