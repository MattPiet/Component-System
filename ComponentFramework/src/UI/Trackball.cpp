#include <glew.h>
#include <SDL.h>
#include <UI/Trackball.h>
#include <VMath.h>
#include <QMath.h>

#define M_PI 3.14159265358979323846f

///https://www.khronos.org/opengl/wiki/Object_Mouse_Trackball
using namespace MATH;

Trackball::Trackball() {
	setWindowDimensions();
	mouseDown = false;
}

Trackball::~Trackball() {}

void Trackball::setWindowDimensions() {
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	invNDC = MMath::inverse(MMath::NDCtoViewport(viewport[2], viewport[3]));
}

void Trackball::HandleEvents(const SDL_Event& sdlEvent) {
	if (sdlEvent.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
		onLeftMouseDown(static_cast<int>(sdlEvent.button.x), static_cast<int>(sdlEvent.button.y));
	}
	else if (sdlEvent.type == SDL_EVENT_MOUSE_BUTTON_UP) {
		onLeftMouseUp(static_cast<int>(sdlEvent.button.x), static_cast<int>(sdlEvent.button.y));
	}
	else if (sdlEvent.type == SDL_EVENT_MOUSE_MOTION &&
		(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK)) {
		onMouseMove(static_cast<int>(sdlEvent.button.x), static_cast<int>(sdlEvent.button.y));
	}
	else if (sdlEvent.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
		setWindowDimensions();
	}
}

void Trackball::onLeftMouseDown(int x, int y) {
	mouseDown = true;
	lastMouseX = x;
	lastMouseY = y;

}

void Trackball::onLeftMouseUp(int x, int y) {
	mouseDown = false;
}

void Trackball::onMouseMove(int x, int y) {
	if (!mouseDown) return;
	float deltaX = static_cast<float>(x - lastMouseX);
	float deltaY = static_cast<float>(y - lastMouseY);
	m_Yaw += -deltaX * m_Sensitivity;
	m_Pitch += -deltaY * m_Sensitivity;
	if (m_Pitch > 89.0f)  m_Pitch = 89.0f;
	if (m_Pitch < -89.0f) m_Pitch = -89.0f;

	Quaternion qYaw = QMath::angleAxisRotation(m_Yaw, Vec3(0.0f, 1.0f, 0.0f));
	Quaternion qPitch = QMath::angleAxisRotation(m_Pitch, Vec3(1.0f, 0.0f, 0.0f));
	mouseRotationQuat = qYaw * qPitch;
	lastMouseX = x;
	lastMouseY = y;
}
#undef M_PI