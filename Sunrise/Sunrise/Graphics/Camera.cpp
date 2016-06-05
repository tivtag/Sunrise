#include "Camera.hpp"
#include "../Math.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, perspective
#include <glm/gtx/quaternion.hpp>

// Constants
#define DEFAULT_FOVX 45.0f
#define DEFAULT_ZNEAR 0.1f
#define DEFAULT_ZFAR 1000.0f
#define DEFAULT_ORBIT_MIN_ZOOM (DEFAULT_ZNEAR + 1.0f)
#define DEFAULT_ORBIT_MAX_ZOOM (DEFAULT_ZFAR * 0.5f)
#define DEFAULT_ORBIT_OFFSET_DISTANCE (DEFAULT_ORBIT_MIN_ZOOM + (DEFAULT_ORBIT_MAX_ZOOM - DEFAULT_ORBIT_MIN_ZOOM) * 0.25f)
#define WORLD_XAXIS glm::vec3(1.0f, 0.0f, 0.0f)
#define WORLD_YAXIS glm::vec3(0.0f, 1.0f, 0.0f)
#define WORLD_ZAXIS glm::vec3(0.0f, 0.0f, 1.0f)

sr::Camera::Camera()
	: behaviour(CameraBehaviour::Spectator),
	preferTargetYAxisOrbiting(true),
	accumPitchDegrees(0.0f), savedAccumPitchDegrees(0.0f),
	rotationSpeed(0.3f),	
	fovx(DEFAULT_FOVX), znear(DEFAULT_ZNEAR), zfar(DEFAULT_ZFAR), aspectRatio(0.0f),
	orbitMinZoom(DEFAULT_ORBIT_MIN_ZOOM), orbitMaxZoom(DEFAULT_ORBIT_MAX_ZOOM), orbitOffsetDistance(DEFAULT_ORBIT_OFFSET_DISTANCE),

	eye(0.0f), savedEye(0.0f), target(0.0f), 
	xAxis(1.0f, 0.0f, 0.0f), yAxis(0.0f, 1.0f, 0.0f), targetYAxis(0.0f, 1.0f, 0.0f), zAxis(0.0f, 0.0f, 1.0f), viewDir(0.0f, 0.0f, -1.0f),
	acceleration(3.0f), currentVelocity(0.0f), velocity(4.0f),

	orientation(), savedOrientation(),
	view(1.0f), proj(1.0f), viewProj(1.0f)
{
}

void sr::Camera::SetupProj()
{
	const float PI = 3.141592653589793238462643383279502884197169399375f;
	
	// proj = glm::frustum(-aspect, aspect, -aspect, aspect, 1.0f, 100.0f); 
	proj = glm::perspective(fovx, AspectRatio(), 0.1f, 10000.0f);
}

void sr::Camera::SetupView()
{
	 // Reconstruct the view matrix.
	view = glm::mat4_cast(orientation);

	xAxis = glm::vec3( view[0][0], view[1][0], view[2][0] );
	yAxis = glm::vec3( view[0][1], view[1][1], view[2][1] );
	zAxis = glm::vec3( view[0][2], view[1][2], view[2][2] );
	viewDir = -zAxis;

	if(behaviour == CameraBehaviour::Orbit)
	{
		// Calculate the new camera position based on the current
        // orientation. The camera must always maintain the same
        // distance from the target. Use the current offset vector
        // to determine the correct distance from the target.
		eye = target + zAxis * orbitOffsetDistance;
	}
	
    view[3][0] = -glm::dot(xAxis, eye);
    view[3][1] = -glm::dot(yAxis, eye);
    view[3][2] = -glm::dot(zAxis, eye);

	SetupViewProj();
}

void sr::Camera::SetupViewProj()
{
	viewProj = proj * view;
}

void sr::Camera::Move(const glm::vec3& offset)
{
	// Moves the camera by dx world units to the left or right; dy
	// world units upwards or downwards; and dz world units forwards
	// or backwards.

	if(behaviour == CameraBehaviour::Orbit)
	{
		// Orbiting camera is always positioned relative to the
		// target position. See updateViewMatrix().
		return;
	}

	glm::vec3 newEye = eye;
	glm::vec3 forwards;

	if(behaviour == CameraBehaviour::FirstPerson)
	{
		// Calculate the forwards direction. Can't just use the camera's local
		// z axis as doing so will cause the camera to move more slowly as the
		// camera's view approaches 90 degrees straight up and down.
		forwards = glm::cross(WORLD_YAXIS, xAxis);
		forwards = glm::normalize(forwards);
	}
	else
	{
		forwards = viewDir;
	}

	newEye += xAxis * offset.x;
	newEye += WORLD_YAXIS * offset.y;
	newEye += forwards * offset.z;

	Position(newEye);
}


void sr::Camera::Behaviour(CameraBehaviour::Enum newBehaviour)
{
}

void sr::Camera::Position(const glm::vec3& newEye)
{
	eye = newEye;
	SetupView();
}   

void sr::Camera::RotateSmoothly(float headingDegrees, float pitchDegrees, float rollDegrees)
{
    // This method applies a scaling factor to the rotation angles prior to
    // using these rotation angles to rotate the camera. This method is usually
    // called when the camera is being rotated using an input device (such as a
    // mouse or a joystick). 

    headingDegrees *= rotationSpeed;
    pitchDegrees *= rotationSpeed;
    rollDegrees *= rotationSpeed;

    Rotate(headingDegrees, pitchDegrees, rollDegrees);
}

void sr::Camera::Rotate(float headingDegrees, float pitchDegrees, float rollDegrees)
{
    // Rotates the camera based on its current behavior.
    // Note that not all behaviors support rolling.
    pitchDegrees = -pitchDegrees;
    headingDegrees = -headingDegrees;
    rollDegrees = -rollDegrees;

    switch(behaviour)
    {
		default:
			break;

		case CameraBehaviour::FirstPerson:
		case CameraBehaviour::Spectator:
			RotateFirstPerson(headingDegrees, pitchDegrees);
			break;

		case CameraBehaviour::Flight:
			RotateFlight(headingDegrees, pitchDegrees, rollDegrees);
			break;

		case CameraBehaviour::Orbit:
			RotateOrbit(headingDegrees, pitchDegrees, rollDegrees);
			break;
    }

    SetupView();
}


void sr::Camera::RotateFirstPerson(float headingDegrees, float pitchDegrees)
{
    // Implements the rotation logic for the first person style and
    // spectator style camera behaviors. Roll is ignored.
    //accumPitchDegrees += pitchDegrees;

    //if(accumPitchDegrees > 90.0f)
    //{
    //    pitchDegrees = 90.0f - (accumPitchDegrees - pitchDegrees);
    //    accumPitchDegrees = 90.0f;
    //}

    //if(accumPitchDegrees < -90.0f)
    //{
    //    pitchDegrees = -90.0f - (accumPitchDegrees - pitchDegrees);
    //    accumPitchDegrees = -90.0f;
    //}
   
    // Rotate camera about the world y axis.
    // Note the order the quaternions are multiplied. That is important!
    if( headingDegrees != 0.0f )
    {
        const glm::quat rot = glm::angleAxis(headingDegrees, WORLD_YAXIS);
        orientation = rot * orientation;
    }

    // Rotate camera about its local x axis.
    // Note the order the quaternions are multiplied. That is important!
    if( pitchDegrees != 0.0f )
    {
        const glm::quat rot = glm::angleAxis(pitchDegrees, WORLD_XAXIS);
        orientation = rot * orientation;
    }
}

void sr::Camera::RotateFlight(float headingDegrees, float pitchDegrees, float rollDegrees)
{
    // Implements the rotation logic for the flight style camera behavior.
    accumPitchDegrees += pitchDegrees;

    if(accumPitchDegrees > 360.0f)
        accumPitchDegrees -= 360.0f;

    if(accumPitchDegrees < -360.0f)
       accumPitchDegrees += 360.0f;
   
    const glm::quat rot(glm::radians(glm::vec3(pitchDegrees, headingDegrees, rollDegrees)));
    orientation = rot * orientation;
}

void sr::Camera::RotateOrbit(float headingDegrees, float pitchDegrees, float rollDegrees)
{
    // Implements the rotation logic for the orbit style camera behavior.
    // Roll is ignored for target Y axis orbiting.
    //
    // Briefly here's how this orbit camera implementation works. Switching to
    // the orbit camera behavior via the setBehavior() method will set the
    // camera's orientation to match the orbit target's orientation. Calls to
    // rotateOrbit() will rotate this orientation. To turn this into a third
    // person style view the updateViewMatrix() method will move the camera
    // position back 'm_orbitOffsetDistance' world units along the camera's
    // local z axis from the orbit target's world position.
    
    if(preferTargetYAxisOrbiting)
    {
        if(headingDegrees != 0.0f)
        {
            const glm::quat rot = glm::angleAxis(headingDegrees, targetYAxis);
            orientation = rot * orientation;
        }

        if(pitchDegrees != 0.0f)
        {
            const glm::quat rot = glm::angleAxis(pitchDegrees, WORLD_XAXIS);
            orientation = orientation * rot;
        }
    }
    else
    {
		const glm::quat rot(glm::vec3(pitchDegrees, headingDegrees, rollDegrees));
		orientation = orientation * rot;
    }
}

void sr::Camera::UpdatePosition(const glm::vec3& direction, float frameTime)
{
    // Moves the camera using Newton's second law of motion. Unit mass is
    // assumed here to somewhat simplify the calculations. The direction vector
    // is in the range [-1,1].

	const float lengthSq = currentVelocity.x*currentVelocity.x + currentVelocity.y*currentVelocity.y + currentVelocity.z*currentVelocity.z; 

    if(lengthSq != 0.0f)
    {
        // Only move the camera if the velocity vector is not of zero length.
        // Doing this guards against the camera slowly creeping around due to
        // floating point rounding errors.
        glm::vec3 displacement = (currentVelocity * frameTime) + (0.5f * acceleration * frameTime * frameTime);

        // Floating point rounding errors will slowly accumulate and cause the
        // camera to move along each axis. To prevent any unintended movement
        // the displacement vector is clamped to zero for each direction that
        // the camera isn't moving in. Note that the updateVelocity() method
        // will slowly decelerate the camera's velocity back to a stationary
        // state when the camera is no longer moving along that direction. To
        // account for this the camera's current velocity is also checked.

        if(direction.x == 0.0f && sr::closeEnough(currentVelocity.x, 0.0f))
            displacement.x = 0.0f;

        if(direction.y == 0.0f && sr::closeEnough(currentVelocity.y, 0.0f))
            displacement.y = 0.0f;

        if(direction.z == 0.0f && sr::closeEnough(currentVelocity.z, 0.0f))
            displacement.z = 0.0f;

        Move(displacement);
    }

    // Continuously update the camera's velocity vector even if the camera
    // hasn't moved during this call. When the camera is no longer being moved
    // the camera is decelerating back to its stationary state.
    UpdateVelocity(direction, frameTime);
}


void sr::Camera::UpdateVelocity(const glm::vec3& direction, float frameTime)
{
    // Updates the camera's velocity based on the supplied movement direction
    // and the elapsed time (since this method was last called). The movement
    // direction is in the range [-1,1].
    if(direction.x != 0.0f)
    {
        // Camera is moving along the x axis.
        // Linearly accelerate up to the camera's max speed.
        currentVelocity.x += direction.x * acceleration.x * frameTime;

        if(currentVelocity.x > velocity.x)
            currentVelocity.x = velocity.x;
        else if(currentVelocity.x < -velocity.x)
            currentVelocity.x = -velocity.x;
    }
    else
    {
        // Camera is no longer moving along the x axis.
        // Linearly decelerate back to stationary state.

        if(currentVelocity.x > 0.0f)
        {
            if((currentVelocity.x -= acceleration.x * frameTime) < 0.0f)
			    currentVelocity.x = 0.0f;
        }
        else
        {
            if((currentVelocity.x += acceleration.x * frameTime) > 0.0f)
                currentVelocity.x = 0.0f;
        }
    }

    if(direction.y != 0.0f)
    {
        // Camera is moving along the y axis.
        // Linearly accelerate up to the camera's max speed.

        currentVelocity.y += direction.y * acceleration.y * frameTime;
        
        if(currentVelocity.y > velocity.y)
            currentVelocity.y = velocity.y;
        else if(currentVelocity.y < -velocity.y)
            currentVelocity.y = -velocity.y;
    }
    else
    {
        // Camera is no longer moving along the y axis.
        // Linearly decelerate back to stationary state.

        if(currentVelocity.y > 0.0f)
        {
            if((currentVelocity.y -= acceleration.y * frameTime) < 0.0f)
                currentVelocity.y = 0.0f;
        }
        else
        {
            if((currentVelocity.y += acceleration.y * frameTime) > 0.0f)
                currentVelocity.y = 0.0f;
        }
    }

    if(direction.z != 0.0f)
    {
        // Camera is moving along the z axis.
        // Linearly accelerate up to the camera's max speed.

        currentVelocity.z += direction.z * acceleration.z * frameTime;

        if(currentVelocity.z > velocity.z)
            currentVelocity.z = velocity.z;
        else if (currentVelocity.z < -velocity.z)
            currentVelocity.z = -velocity.z;
    }
    else
    {
        // Camera is no longer moving along the z axis.
        // Linearly decelerate back to stationary state.

        if(currentVelocity.z > 0.0f)
        {
            if((currentVelocity.z -= acceleration.z * frameTime) < 0.0f)
                currentVelocity.z = 0.0f;
        }
        else
        {
            if((currentVelocity.z += acceleration.z * frameTime) > 0.0f)
                currentVelocity.z = 0.0f;
        }
    }
}

struct CameraState
{		
	glm::vec3 eye;
	glm::vec3 target, targetYAxis;
	glm::vec3 viewDir;
	glm::quat orientation;
};

void sr::Camera::WriteState(std::ostream& stream) const
{
	const int Version = 1;
	stream << Version;

	CameraState state;
	state.eye = eye;
	state.orientation = orientation;
	state.target = target;
	state.targetYAxis = targetYAxis;
	state.viewDir = viewDir;

	stream.write( (const char*)&state, sizeof( CameraState ) );
	stream.flush();
}

void sr::Camera::ReadState(std::istream& stream)
{
	int version;
	stream >> version;
		
	CameraState state;
	stream.read( (char*)&state, sizeof( CameraState ) );
	
	savedEye = eye = state.eye;
	savedOrientation = orientation = state.orientation;
	target = state.target;
	targetYAxis = state.targetYAxis;
	viewDir = state.viewDir;

	SetupView();
}
