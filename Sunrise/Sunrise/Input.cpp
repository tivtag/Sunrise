#include "Input.hpp"

#include "OIS\OIS.h"

#include <iostream>
sr::Input::Input(void const * const windowHandle)
{
	inputManager = OIS::InputManager::createInputSystem((std::size_t)windowHandle);	
	keyboard = (OIS::Keyboard*)inputManager->createInputObject( OIS::OISKeyboard, true );
	mouse = (OIS::Mouse*)inputManager->createInputObject( OIS::OISMouse, true );

	//g_kb->setEventCallback( &handler );
	//g_m->setEventCallback( &handler );
}

sr::Input::~Input()
{
	OIS::InputManager::destroyInputSystem(inputManager);
}

bool sr::Input::ShiftDown()
{
	return keyboard->isModifierDown(OIS::Keyboard::Shift);
}

void sr::Input::SetMousePos(const glm::uvec2& position)
{
	 OIS::MouseState &mutableMouseState = const_cast<OIS::MouseState &>(mouse->getMouseState());
	 mutableMouseState.X.abs = position.x;
	 mutableMouseState.Y.abs = position.y;
}

void sr::Input::Capture()
{
	keyboard->capture();
	mouse->capture();
}

void sr::Input::EndFrame()
{
	keyboard->copyKeyStates(oldKeyStates);
}

glm::vec2 sr::Input::MousePosition() const
{
	return glm::vec2( mouse->getMouseState().X.abs, mouse->getMouseState().Y.abs );
}

glm::vec2 sr::Input::RelativeMousePosition() const
{
	return glm::vec2( mouse->getMouseState().X.rel, mouse->getMouseState().Y.rel );
}

bool sr::Input::RightMouseDown() const
{
	return mouse->getMouseState().buttonDown( OIS::MB_Right );
}

bool sr::Input::KeyDown(const sr::Key::Enum key) const
{
	return keyboard->isKeyDown(static_cast<OIS::KeyCode>(key));
}

bool sr::Input::KeyUp(const sr::Key::Enum key) const
{
	return !KeyDown(key);
}

bool sr::Input::WasKeyDown(const sr::Key::Enum key) const
{
	const std::size_t index = static_cast<std::size_t>(key);
	return oldKeyStates[index] == 1;
}

bool sr::Input::WasKeyUp(const sr::Key::Enum key) const
{
	return !WasKeyDown(key);
}

bool sr::Input::KeyPressed(const sr::Key::Enum key) const
{
	return KeyDown(key) && WasKeyUp(key);
}