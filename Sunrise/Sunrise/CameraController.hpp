#pragma once

#include "glm\glm.hpp"

namespace sr
{
	class Input;
	class Camera;

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
