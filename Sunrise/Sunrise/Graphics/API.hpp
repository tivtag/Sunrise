#pragma once


#define BUILD_OGLES2 1

/****************************************************************************
** 3D API header files
****************************************************************************/
#if defined(__BADA__)
#if defined(BUILD_OGLES)
#include <FGraphicsOpengl.h>
#else
#if defined(BUILD_OGLES2)
#include <FGraphicsOpengl2.h>
#endif
#endif

using namespace Osp::Graphics::Opengl;
#else
#ifdef BUILD_OGLES2
	#include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
	#include <EGL/egl.h>
#elif BUILD_OGLES3
	#include <GLES3/gl3.h>
    #include <GLES3/gl2ext.h>
	#include <EGL/egl.h>
#elif BUILD_OGL
#define SUPPORT_OPENGL
#if defined(WIN32) || defined(UNDER_CE)
	#include <windows.h>
#endif
	#include <GL/glew.h>
	#include <EGL/egl.h>
#elif BUILD_OVG
#include <VG/openvg.h>
#include <EGL/egl.h>
#else
	#include <EGL/egl.h>
	#include <GLES/gl.h>
#endif
#endif
