#include "CameraController.hpp"
#include "Graphics/Camera.hpp"
#include "Input.hpp"



sr::CameraController::CameraController(sr::Input& _input, sr::Camera& _camera)
	: input(_input), camera(_camera)
{
}

void sr::CameraController::Update()
{
	const float CAMERA_SPEED_FLIGHT_YAW = 100.0f;
	const float CAMERA_SPEED_ORBIT_ROLL = 100.0f;
	float frameTime = 0.333f;

	glm::vec2 mouseDelta = input.RelativeMousePosition();
    glm::vec3 direction = MovementDirection();
	
    switch( camera.Behaviour() )
    {
    default:
        break;

	case CameraBehaviour::FirstPerson:
	case CameraBehaviour::Spectator:
        camera.RotateSmoothly(-mouseDelta.x, -mouseDelta.y, 0.0f);
        camera.UpdatePosition(direction, frameTime);
        break;

	case CameraBehaviour::Flight:		
        camera.RotateSmoothly(0.0f, mouseDelta.y, -mouseDelta.x);

        if(float deltaX = -direction.x * CAMERA_SPEED_FLIGHT_YAW * frameTime!= 0.0f)
            camera.Rotate(glm::radians(deltaX), 0.0f, 0.0f);

        direction.x = 0.0f; // ignore yaw motion when updating camera's velocity
        camera.UpdatePosition(direction, frameTime);
        break;

	case CameraBehaviour::Orbit:
        camera.RotateSmoothly(mouseDelta.x, mouseDelta.y, 0.0f);

        if( !camera.PreferTargetYAxisOrbiting() )
        {
            if( float deltaZ = direction.x * CAMERA_SPEED_ORBIT_ROLL * frameTime != 0.0f)
                camera.Rotate(0.0f, 0.0f, deltaZ);
        }

        //if ((dz = -mouse.wheelPos()) != 0.0f)
        //    g_camera.zoom(dz, g_camera.getOrbitMinZoom(), g_camera.getOrbitMaxZoom());
        break;
    }

	input.SetMousePos( glm::uvec2(camera.WindowSize().x/2, camera.WindowSize().y/2) );
}



glm::vec3 sr::CameraController::MovementDirection()
{
    static bool moveForwardsPressed = false;
    static bool moveBackwardsPressed = false;
    static bool moveRightPressed = false;
    static bool moveLeftPressed = false;
    static bool moveUpPressed = false;
    static bool moveDownPressed = false;

    glm::vec3 velocity = camera.CurrentVelocity();	
    glm::vec3 direction(0.0f);

    if( input.KeyDown( sr::Key::KC_W ))
    {
        if(!moveForwardsPressed)
        {
            moveForwardsPressed = true;
            camera.CurrentVelocity(glm::vec3(velocity.x, velocity.y, 0.0f));
        }

        direction.z += 1.0f;
    }
    else
    {
        moveForwardsPressed = false;
    }

    if(input.KeyDown( sr::Key::KC_S ))
    {
        if (!moveBackwardsPressed)
        {
            moveBackwardsPressed = true;
            camera.CurrentVelocity(glm::vec3(velocity.x, velocity.y, 0.0f));
        }

        direction.z -= 1.0f;
    }
    else
    {
        moveBackwardsPressed = false;
    }

    if(input.KeyDown( sr::Key::KC_D))
    {
        if (!moveRightPressed)
        {
            moveRightPressed = true;
            camera.CurrentVelocity(glm::vec3(0.0f, velocity.y, velocity.z));
        }

        direction.x += 1.0f;
    }
    else
    {
        moveRightPressed = false;
    }

    if(input.KeyDown( sr::Key::KC_A))
    {
        if (!moveLeftPressed)
        {
            moveLeftPressed = true;
            camera.CurrentVelocity(glm::vec3(0.0f, velocity.y, velocity.z));
        }

        direction.x -= 1.0f;
    }
    else
    {
        moveLeftPressed = false;
    }

    if(input.KeyDown( sr::Key::KC_E))
    {
        if (!moveUpPressed)
        {
            moveUpPressed = true;
            camera.CurrentVelocity(glm::vec3(velocity.x, 0.0f, velocity.z));
        }

        direction.y += 1.0f;
    }
    else
    {
        moveUpPressed = false;
    }

    if(input.KeyDown( sr::Key::KC_Q))
    {
        if (!moveDownPressed)
        {
            moveDownPressed = true;
            camera.CurrentVelocity(glm::vec3(velocity.x, 0.0f, velocity.z));
        }

        direction.y -= 1.0f;
    }
    else
    {
        moveDownPressed = false;
    }

	return direction;
}