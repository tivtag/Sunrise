#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp> // translate, rotate, scale, perspective
#include <glm/gtc/type_ptr.hpp>			// value_ptr

#include <iostream>

namespace sr 
{   
	namespace CameraBehaviour
	{
		enum Enum 
		{
			FirstPerson,
			Spectator,
			Flight,
			Orbit
		};
	}

	class Camera
	{
	public:
		const glm::mat4x4& Proj() const { return proj; }
		const glm::mat4x4& View() const { return view; }
		const glm::mat4x4& ViewProj() const { return viewProj; }
		float AspectRatio() const { return aspectRatio; }

		glm::uvec2 WindowSize() const { return windowSize; }
		void WindowSize( const glm::uvec2& value ) 
		{
			windowSize = value; 
			aspectRatio = (float)windowSize.x / (float)windowSize.y;

			SetupProj();	
			SetupViewProj();
		}

		CameraBehaviour::Enum Behaviour() const { return behaviour; }
		void Behaviour(CameraBehaviour::Enum);

		void Acceleration(const glm::vec3& value) { acceleration = value; }

		glm::vec3 CurrentVelocity() const { return currentVelocity; }
		void CurrentVelocity(const glm::vec3& value) { currentVelocity = value; }

		bool PreferTargetYAxisOrbiting() const { return preferTargetYAxisOrbiting; }

	public:
		Camera();

		void Position(const glm::vec3&);		
		void Move(const glm::vec3&);
		
		void RotateSmoothly(const glm::vec3& angles) { RotateSmoothly(angles.x, angles.y, angles.z); }
		void RotateSmoothly(float headingDegrees, float pitchDegrees, float rollDegrees);

		void Rotate(const glm::vec3& angles) { Rotate(angles.x, angles.y, angles.z); }
		void Rotate(float headingDegrees, float pitchDegrees, float rollDegrees);
		
		void UpdatePosition(const glm::vec3& direction, float frameTime);
		
		void WriteState(std::ostream& stream) const;
		void ReadState(std::istream& stream);

	private:
		void SetupProj();
		void SetupView();
		void SetupViewProj();
		Camera(Camera&) {}
		
		void RotateFirstPerson(float headingDegrees, float pitchDegrees);		
		void RotateFlight(float headingDegrees, float pitchDegrees, float rollDegrees);
		void RotateOrbit(float headingDegrees, float pitchDegrees, float rollDegrees);
	    void UpdateVelocity(const glm::vec3& direction, float frameTime);

	private:
		// Projection
		glm::uvec2 windowSize;		
		float fovx;
		float aspectRatio;
		float znear;
		float zfar;

		// Settings
		CameraBehaviour::Enum behaviour;
		float accumPitchDegrees;
		float savedAccumPitchDegrees;
		float rotationSpeed;

		// Orbit
		bool preferTargetYAxisOrbiting;
		float orbitMinZoom;
		float orbitMaxZoom;
		float orbitOffsetDistance;
		
		// First person
		float firstPersonYOffset;

		// Orientation
		glm::vec3 eye, savedEye;
		glm::vec3 target, targetYAxis;
		glm::vec3 xAxis, yAxis, zAxis;
		glm::vec3 viewDir;
		glm::quat orientation, savedOrientation;

		// Movement
		glm::vec3 acceleration, currentVelocity, velocity;

		// Matrices
		glm::mat4x4 proj;
		glm::mat4x4 view;
		glm::mat4x4 viewProj;
	};
}
