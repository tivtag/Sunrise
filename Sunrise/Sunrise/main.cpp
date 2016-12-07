#include "Shell/PVRShell.h"
#include "Shell/PVRTgles2Ext.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <glm/glm.hpp>

#include <fstream>
#include <string>
#include <vector>

#include "Graphics/TextureManager.hpp"
#include "Graphics/Camera.hpp"
#include "World/GroundInfo.hpp"
#include "GravityResourceFile.hpp"
#include "Input.hpp"
#include "CameraController.hpp"
#include "FileSystem.hpp"

class SunriseShell : public PVRShell
{	
	GLuint uiFragShader, uiVertShader;		/* Used to hold the fragment and vertex shader handles */
	GLuint uiProgramObject;					/* Used to hold the program handle (made out of the two previous shaders */
	
	GLuint ui32Vbo;							// Vertex buffer object handle
	GLuint colorBuffer;
	GLuint uvBuffer;
	GLuint location_textureID;
	GLuint m_ui32VertexStride;

	sr::TexturePtr texture;
	GLuint triangleCount;

	bool drawWireframe;
	int width, height;

	sr::Input* input;
	sr::Camera camera;
	sr::CameraController* cameraController;
	sr::FileSystem* fileSystem;
    sr::TextureManager* textureManager;

public:
	SunriseShell();

	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

private:
	void HandleInput();
	void LoadCube();
	void LoadWorld();
	void LoadTexture();
};

SunriseShell::SunriseShell()
	: input(0), cameraController(0), fileSystem(0), textureManager(0), drawWireframe(false), texture(0)
{
}

/*!****************************************************************************
 @Function		InitApplication
 @Return		bool		true if no error occured
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool SunriseShell::InitApplication()
{
	fileSystem = new sr::FileSystem(static_cast<const char*>(PVRShellGet(prefReadPath)));

	PVRShellSet(prefWidth, 1024);
	PVRShellSet(prefHeight, 800);
	PVRShellSet(prefFSAAMode, 2);
	// PVRShellSet(prefSwapInterval, 0); // VSync

	return true;
}

/*!****************************************************************************
 @Function		QuitApplication
 @Return		bool		true if no error  occured
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.
******************************************************************************/
bool SunriseShell::QuitApplication()
{
    delete textureManager;
	delete cameraController;
	delete input;
	delete fileSystem;
    return true;
}

const int VERTEX_ARRAY  = 0;
const int ATTRIB_COLOUR = 1;
const int ATTRIB_UV		= 2;

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no  error occured
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependant on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool SunriseShell::InitView()
{
	void* windowHandle = PVRShellGet(prefNativeWindowType);
	input = new sr::Input(windowHandle);
	cameraController = new sr::CameraController(*input, camera);
    textureManager = new sr::TextureManager(*fileSystem);

	//	Fragment and vertex shaders code
	char* pszFragShader = "\
		uniform sampler2D sampler2d;		\
		varying mediump vec2 v_uv; \
		varying lowp vec4 v_color;			\
											\
		void main(void)\
		{\
			gl_FragColor = texture2D(sampler2d, v_uv); \
		}";

	char* pszVertShader = "\
		attribute highp vec4 a_vertex;		\
		attribute lowp vec4 a_color;		\
		attribute mediump vec4 a_uv;		\
											\
		uniform mediump mat4	PMVMatrix;	\
		varying lowp vec4 v_color;			\
		varying mediump vec2 v_uv;			\
											\
		void main(void)\
		{\
			gl_Position = PMVMatrix * a_vertex;	\
			v_color = a_color;					\
			v_uv = a_uv.st;						\
		}";


	// Create the fragment shader object
	uiFragShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load the source code into it
	glShaderSource(uiFragShader, 1, (const char**)&pszFragShader, NULL);

	// Compile the source code
	glCompileShader(uiFragShader);

	// Check if compilation succeeded
	GLint bShaderCompiled;
    glGetShaderiv(uiFragShader, GL_COMPILE_STATUS, &bShaderCompiled);

	if( !bShaderCompiled )
	{
		return false;
	}

	// Loads the vertex shader in the same way
	uiVertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(uiVertShader, 1, (const char**)&pszVertShader, NULL);
	glCompileShader(uiVertShader);
    glGetShaderiv(uiVertShader, GL_COMPILE_STATUS, &bShaderCompiled);
	
	if( !bShaderCompiled )
	{
		return false;
	}

	// Create the shader program
    uiProgramObject = glCreateProgram();

	// Attach the fragment and vertex shaders to it
    glAttachShader(uiProgramObject, uiFragShader);
    glAttachShader(uiProgramObject, uiVertShader);

	// Bind the custom vertex attribute "myVertex" to location VERTEX_ARRAY
    glBindAttribLocation(uiProgramObject, VERTEX_ARRAY, "a_vertex");
	glBindAttribLocation(uiProgramObject, ATTRIB_COLOUR, "a_color");
	glBindAttribLocation(uiProgramObject, ATTRIB_UV, "a_uv");

	// Link the program
    glLinkProgram(uiProgramObject);

	// Check if linking succeeded in the same way we checked for compilation success
    GLint bLinked;
    glGetProgramiv(uiProgramObject, GL_LINK_STATUS, &bLinked);
	if (!bLinked)
	{
		return false;
	}
    
	//LoadCube();
	LoadWorld();
	LoadTexture();
	
	// Init view
	width = PVRShellGet(prefWidth);
	height = PVRShellGet(prefHeight);
	camera.WindowSize( glm::uvec2(PVRShellGet(prefWidth), PVRShellGet(prefHeight)) );

	glViewport(0, 0, width, height);
	return true;
}

void SunriseShell::LoadCube()
{
	// Interleaved vertex data
	GLfloat afVertices[] = {
		-1.0f,-1.0f,-1.0f, // triangle 1 : begin
		0.000059f, 1.0f-0.000004f,
		-1.0f,-1.0f, 1.0f,
		0.000103f, 1.0f-0.336048f,
		-1.0f, 1.0f, 1.0f, // triangle 1 : end
			0.335973f, 1.0f-0.335903f,
		1.0f, 1.0f,-1.0f, // triangle 2 : begin
		1.000023f, 1.0f-0.000013f,
		-1.0f,-1.0f,-1.0f,
		0.667979f, 1.0f-0.335851f,
		-1.0f, 1.0f,-1.0f, // triangle 2 : end
		0.999958f, 1.0f-0.336064f,
		1.0f,-1.0f, 1.0f,
		0.667979f, 1.0f-0.335851f,
		-1.0f,-1.0f,-1.0f,
		0.336024f, 1.0f-0.671877f,
		1.0f,-1.0f,-1.0f,
		0.667969f, 1.0f-0.671889f,
		1.0f, 1.0f,-1.0f,
		1.000023f, 1.0f-0.000013f,
		1.0f,-1.0f,-1.0f,
		0.668104f, 1.0f-0.000013f,
		-1.0f,-1.0f,-1.0f,
		0.667979f, 1.0f-0.335851f,
		-1.0f,-1.0f,-1.0f,
		0.000059f, 1.0f-0.000004f,
		-1.0f, 1.0f, 1.0f,
		0.335973f, 1.0f-0.335903f,
		-1.0f, 1.0f,-1.0f,
		0.336098f, 1.0f-0.000071f,
		1.0f,-1.0f, 1.0f,
		0.667979f, 1.0f-0.335851f,
		-1.0f,-1.0f, 1.0f,
		0.335973f, 1.0f-0.335903f,
		-1.0f,-1.0f,-1.0f,
		0.336024f, 1.0f-0.671877f,
		-1.0f, 1.0f, 1.0f,
		1.000004f, 1.0f-0.671847f,
		-1.0f,-1.0f, 1.0f,
		0.999958f, 1.0f-0.336064f,
		1.0f,-1.0f, 1.0f,0.667979f, 1.0f-0.335851f,
		1.0f, 1.0f, 1.0f,0.668104f, 1.0f-0.000013f,
		1.0f,-1.0f,-1.0f, 	 0.335973f, 1.0f-0.335903f,
		1.0f, 1.0f,-1.0f,	 0.667979f, 1.0f-0.335851f,
		1.0f,-1.0f,-1.0f,	 0.335973f, 1.0f-0.335903f,
		1.0f, 1.0f, 1.0f,	 0.668104f, 1.0f-0.000013f,
		1.0f,-1.0f, 1.0f,	 0.336098f, 1.0f-0.000071f,
		1.0f, 1.0f, 1.0f,	 0.000103f, 1.0f-0.336048f,
		1.0f, 1.0f,-1.0f,	 0.000004f, 1.0f-0.671870f,
		-1.0f, 1.0f,-1.0f,	 0.336024f, 1.0f-0.671877f,
		1.0f, 1.0f, 1.0f,	 0.000103f, 1.0f-0.336048f,
		-1.0f, 1.0f,-1.0f,	 0.336024f, 1.0f-0.671877f,
		-1.0f, 1.0f, 1.0f,	 0.335973f, 1.0f-0.335903f,
		1.0f, 1.0f, 1.0f,	 0.667969f, 1.0f-0.671889f,
		-1.0f, 1.0f, 1.0f,	 1.000004f, 1.0f-0.671847f,
		1.0f,-1.0f, 1.0f, 	 0.667979f, 1.0f-0.335851f
	};

	glGenBuffers(1, &ui32Vbo);
	triangleCount = 6*2*3;
	m_ui32VertexStride = 5 * sizeof(GLfloat); // 3 floats for the pos, 2 for the UVs

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, ui32Vbo);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, sizeof(afVertices), afVertices, GL_STATIC_DRAW);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SunriseShell::LoadTexture()
{	
	const char* readPath = static_cast<const char*>(PVRShellGet(prefReadPath));
	std::string path = "media\\textures\\desktop.png";

	texture = textureManager->LoadTexture(path);		
	location_textureID = glGetUniformLocation(uiProgramObject, "sampler2d");


	if( CPVRTgles2Ext::IsGLExtensionSupported( "GL_EXT_texture_filter_anisotropic" ) )
	{
		std::cout << "GL_EXT_texture_filter_anisotropic supported";
	}
	else
	{
		std::cout << "GL_EXT_texture_filter_anisotropic not supported!! :(";
	}
}

struct Vertex
{	
#pragma pack(push,1)
	GLfloat x;
	GLfloat y;
	GLfloat z;

	GLfloat u;
	GLfloat v;
#pragma pack(pop)

	Vertex(GLfloat _x, GLfloat _y,  GLfloat _z, GLfloat _u, GLfloat _v )
		:x((GLfloat)_x), y((GLfloat)_y), z((GLfloat)_z), u(_u), v(_v)
	{
	}
};


void SunriseShell::LoadWorld()
{
	std::string path = fileSystem->GetFullPath( "media\\maps\\juperos_02\\juperos_02.gnd" );//ra\\ra.gnd glast_01";

	std::ifstream stream;
	stream.open(path, std::ios::in | std::ios::binary);

	if( stream.is_open() )
	{
		GroundInfo ground;
		ground.ReadStream(stream);
                
        //const std::string path = "E:\\Spiele\\RebirthRO\\gf\\bdata.grf";
        //sr::GravityResourceFile grf;

        //grf.Open(path);

        //for(int i = 0; i < ground.TextureCount(); ++i)
        //{
        //    std::string texture = "data\\texture\\" + std::string(ground.GetTexture(i));

        //    if( grf.HasFile(texture))
        //    {
        //        std::cout << "found " << texture << std::endl;

        //        std::string targetFile = fileSystem->GetFullPath( texture );
        //        std::string targetDir = fileSystem->GetFolder( targetFile );
        //        fileSystem->CreateFolder(targetDir);

        //        grf.SaveFile( texture, targetFile );
        //    }
        //    else
        //    {
        //        std::cout << "missing " << texture << std::endl;
        //    }
        //}
        

        // Create Vertices
		std::vector<Vertex> vertices;
		const unsigned int w = ground.Width();
		const unsigned int h = ground.Height();

		for( unsigned int x = 0; x < w; ++x )
		{
			for( unsigned int y = 0; y < h; ++y )
			{
				const GroundInfo::Cell& cell = ground.GetCell(x, y);

#if 1 /* TILE_UP */
				if( cell.topSurfaceId != -1 )
				{
					const GroundInfo::Surface& surface = ground.GetSurface(cell.topSurfaceId);			
					vertices.push_back( Vertex( x + 0, y + 0, -cell.height[0], surface.u[0], surface.v[0] ) );
					vertices.push_back( Vertex( x + 1, y + 0, -cell.height[1], surface.u[1], surface.v[1] ) );
					vertices.push_back( Vertex( x + 1, y + 1, -cell.height[3], surface.u[3], surface.v[3] ) );
										
					vertices.push_back( Vertex( x + 0, y + 0, -cell.height[0], surface.u[0], surface.v[0] ) );
					vertices.push_back( Vertex( x + 1, y + 1, -cell.height[3], surface.u[3], surface.v[3] ) );
					vertices.push_back( Vertex( x + 0, y + 1, -cell.height[2], surface.u[2], surface.v[2] ) );
	/*				
					vertsv[0][0] = (float)x;
					vertsv[0][1] = (float)y;
					vertsv[0][2] = -cell.height[0];
					texv[0][0] = surface.u[0];
					texv[0][1] = surface.v[0];

					vertsv[1][0] = (float)x + 1.00f;
					vertsv[1][1] = (float)y;
					vertsv[1][2] = -cell.height[1];
					texv[1][0] = surface.u[1];
					texv[1][1] = surface.v[1];
					
					vertsv[2][0] = (float)x + 1.00f;
					vertsv[2][1] = (float)y + 1.00f;
					vertsv[2][2] = -cell.height[3];
					texv[2][0] = surface.u[3];
					texv[2][1] = surface.v[3];
					
					vertsv[3][0] = (float)x;
					vertsv[3][1] = (float)y + 1.00f;
					vertsv[3][2] = -cell.height[2];
					texv[3][0] = surface.u[2];
					texv[3][1] = surface.v[2];*/
				}
#endif

#if 1 /* TILE_SIDE */
				if (cell.frontSurfaceId != -1) 
				{
					const GroundInfo::Cell& cell2 = ground.GetCell(x, y + 1);
					const GroundInfo::Surface& surface = ground.GetSurface(cell.frontSurfaceId);
				
					vertices.push_back( Vertex( x + 0, y + 1,  -cell.height[2], surface.u[0], surface.v[0] ) );
					vertices.push_back( Vertex( x + 1, y + 1,  -cell.height[3], surface.u[1], surface.v[1] ) );
					vertices.push_back( Vertex( x + 1, y + 1, -cell2.height[1], surface.u[3], surface.v[3] ) );
												
					vertices.push_back( Vertex( x + 0, y + 1,  -cell2.height[0], surface.u[2], surface.v[2] ) );
					vertices.push_back( Vertex( x + 1, y + 1, -cell2.height[1], surface.u[3], surface.v[3] ) );
					vertices.push_back( Vertex( x + 0, y + 1,  -cell.height[2], surface.u[0], surface.v[0] ) );


					/// top = T0->T1->T2->T3
					/// front = T2->T3->F0->F1
					/// right = T3->T1->R2->R0
					/// 2---3
					/// | F | (cell y+1)
					/// 0---1
					/// 2---3 2---3
					/// | T | | R | (cell x+1)
					/// 0---1 0---1

					//vertsv[0][0] = (float)x;
					//vertsv[0][2] = (float)y + 1.00f;
					//vertsv[0][1] = -cell.height[3];
					//texv[0][0] = surface.u[0];
					//texv[0][1] = surface.v[0];

					//vertsv[1][0] = (float)x + 1.00f;
					//vertsv[1][2] = (float)y + 1.00f;
					//vertsv[1][1] = -cell.height[1];
					//texv[1][0] = surface.u[1];
					//texv[1][1] = surface.v[1];

					//vertsv[2][0] = (float)x + 1.00f;
					//vertsv[2][2] = (float)y + 1.00f;
					//vertsv[2][1] = -cell2.height[0];
					//texv[2][0] = surface.u[3];
					//texv[2][1] = surface.v[3];

					//vertsv[3][0] = (float)x;
					//vertsv[3][2] = (float)y + 1.00f;
					//vertsv[3][1] = -cell.height[2];
					//texv[3][0] = surface.u[2];
					//texv[3][1] = surface.v[2];
				}
#endif

#if 1 /* TILE_ASIDE */
				if (cell.rightSurfaceId != -1) 
				{
					const GroundInfo::Cell& cell2 = ground.GetCell(x + 1, y);
					const GroundInfo::Surface& surface = ground.GetSurface(cell.rightSurfaceId);
				
					vertices.push_back( Vertex( x + 1, y + 1, -cell.height[3], surface.u[0], surface.v[0] ) );  // Lower left
					vertices.push_back( Vertex( x + 1, y + 0, -cell.height[1], surface.u[1], surface.v[1] ) );	// Lower right
					vertices.push_back( Vertex( x + 1, y + 1, -cell2.height[2], surface.u[2], surface.v[2] ) ); // Upper right
					
					vertices.push_back( Vertex( x + 1, y + 0, -cell.height[1], surface.u[1], surface.v[1] ) );	// Lower Left
					vertices.push_back( Vertex( x + 1, y + 0, -cell2.height[0], surface.u[3], surface.v[3] ) ); // Upper right
					vertices.push_back( Vertex( x + 1, y + 1, -cell2.height[2], surface.u[2], surface.v[2] ) );	// Upper left

					//vertsv[0][0] = (float)x + 1.00f;
					//vertsv[0][2] = (float)y + 1.00f;
					//vertsv[0][1] = -cell.height[2];
					//texv[0][0] = surface.u[0];
					//texv[0][1] = surface.v[0];

					//vertsv[1][0] = (float)x + 1.00f;
					//vertsv[1][2] = (float)y;
					//vertsv[1][1] = -cell.height[3];
					//texv[1][0] = surface.u[1];
					//texv[1][1] = surface.v[1];

					//vertsv[2][0] = (float)x + 1.00f;
					//vertsv[2][2] = (float)y;
					//vertsv[2][1] = -cell2.height[1];
					//texv[2][0] = surface.u[3];
					//texv[2][1] = surface.v[3];


					//vertsv[3][0] = (float)x + 1.00f;
					//vertsv[3][2] = (float)y + 1.00f;
					//vertsv[3][1] = -cell2.height[0];
					//texv[3][0] = surface.u[2];
					//texv[3][1] = surface.v[2];
				}
#endif
			}
		}
		
		glGenBuffers(1, &ui32Vbo);

		triangleCount = vertices.size();
		m_ui32VertexStride = sizeof(Vertex); // 3 floats for the pos, 2 for the UVs

		std::cout << triangleCount << " vertices";

		// Bind the VBO
		glBindBuffer(GL_ARRAY_BUFFER, ui32Vbo);

		// Set the buffer's data
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		// Unbind the VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool SunriseShell::ReleaseView()
{
	// Delete program and shader objects
	glDeleteProgram(uiProgramObject);
	glDeleteShader(uiVertShader);
	glDeleteShader(uiFragShader);
    
	// Delete buffer objects
	glDeleteBuffers(1, &ui32Vbo);
	glDeleteBuffers(1, &colorBuffer);
	glDeleteBuffers(1, &uvBuffer);
	return true;
}


glm::vec2 oldMouse;
glm::vec3 rotate;
int frame = 0;

void SunriseShell::HandleInput()
{
	input->Capture();
	cameraController->Update();

	if( input->KeyPressed(sr::Key::KC_F5) )
	{
		const std::string path = fileSystem->GetFullPath("camera.cam"); 
		std::fstream str(path, std::ios::out);

		if( str.is_open() )
		{
			camera.WriteState(str);
		}
	}
	
	if( input->KeyPressed(sr::Key::KC_F7) )
	{
		const std::string path = fileSystem->GetFullPath("camera.cam"); 
		std::fstream str(path, std::ios::in);

		if( str.is_open() )
		{
			camera.ReadState(str);
		}
	}
	
	if( input->KeyPressed(sr::Key::KC_F2) )
	{
		drawWireframe = !drawWireframe;
	}

	input->EndFrame();
}


/*!****************************************************************************
 @Function		RenderScene
 @Return		bool		true if no error occured
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevent OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool SunriseShell::RenderScene()
{
	HandleInput();

	// Clear frame state:
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //0.6f, 0.8f, 1.0f, 1.0f);
	

	++frame;


    glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST); // Enable depth test
	glDepthFunc(GL_LESS);	 // Accept fragment if it closer to the camera than the former one		
    glUseProgram(uiProgramObject); // Actually use the created program
			
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	texture->Bind();
	glUniform1i(location_textureID, 0); // Set our "myTextureSampler" sampler to user Texture Unit 0

	{
		// Model
		// ((float)frame)*0.5f
		glm::mat4 model = glm::rotate( glm::mat4(1.0f), (float)40.0f, glm::vec3(0.0f, 0.7f, 0.7f));	
		glm::mat4 MVP = camera.ViewProj() * model;
	
		// First gets the location of that variable in the shader using its name
		// Then passes the matrix to that variable
		glUniformMatrix4fv( 
			glGetUniformLocation(uiProgramObject, "PMVMatrix"),
			1, GL_FALSE, glm::value_ptr(MVP)
		);
		
		// Bind the VBO
		glBindBuffer(GL_ARRAY_BUFFER, ui32Vbo);

		// Pass the vertex data
		glEnableVertexAttribArray(VERTEX_ARRAY);
		glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, m_ui32VertexStride, 0);

		// Pass the texture coordinates data
		glEnableVertexAttribArray(ATTRIB_UV);
		glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, m_ui32VertexStride, (void*) (3 * sizeof(GLfloat)));

		// Draws a non-indexed triangle array
		glDrawArrays(drawWireframe ? GL_LINES : GL_TRIANGLES, 0, triangleCount);

		// Unbind the VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
		
	//{
	//	// Model
	//	glm::mat4 model = glm::rotate( glm::translate( glm::mat4(1.0f), glm::vec3(-5.0f, 0.0f, 0.0f ) ), (float)frame, glm::vec3(0.0f, 1.0f, 0.0f));	
	//	glm::mat4 MVP = projection * view * model;
	//
	//	// First gets the location of that variable in the shader using its name
	//	// Then passes the matrix to that variable
	//	glUniformMatrix4fv( 
	//		glGetUniformLocation(uiProgramObject, "PMVMatrix"),
	//		1, GL_FALSE, glm::value_ptr(MVP)
	//	);

	//	/*
	//		Draw a triangle.
	//		Please refer to HelloTriangle or IntroducingPVRShell for a detailed explanation.
	//	*/

	//	// Bind the VBO
	//	glBindBuffer(GL_ARRAY_BUFFER, ui32Vbo);

	//	// Pass the vertex data
	//	glEnableVertexAttribArray(VERTEX_ARRAY);
	//	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, m_ui32VertexStride, 0);

	//	// Pass the texture coordinates data
	//	glEnableVertexAttribArray(ATTRIB_UV);
	//	glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, m_ui32VertexStride, (void*) (3 * sizeof(GLfloat)));

	//	// Draws a non-indexed triangle array
	//	glDrawArrays(GL_TRIANGLES, 0, triangleCount);

	//	// Unbind the VBO
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//}

	//{
	//	// Model
	//	glm::mat4 model = glm::rotate( glm::translate( glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f ) ), (float)frame, glm::vec3(0.0f, 0.0f, 1.0f));	
	//	glm::mat4 MVP = projection * view * model;
	//
	//	// First gets the location of that variable in the shader using its name
	//	// Then passes the matrix to that variable
	//	glUniformMatrix4fv( 
	//		glGetUniformLocation(uiProgramObject, "PMVMatrix"),
	//		1, GL_FALSE, glm::value_ptr(MVP)
	//	);

	//	/*
	//		Draw a triangle.
	//		Please refer to HelloTriangle or IntroducingPVRShell for a detailed explanation.
	//	*/

	//	// Bind the VBO
	//	glBindBuffer(GL_ARRAY_BUFFER, ui32Vbo);

	//	// Pass the vertex data
	//	glEnableVertexAttribArray(VERTEX_ARRAY);
	//	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, m_ui32VertexStride, 0);

	//	// Pass the texture coordinates data
	//	glEnableVertexAttribArray(ATTRIB_UV);
	//	glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, m_ui32VertexStride, (void*) (3 * sizeof(GLfloat)));

	//	// Draws a non-indexed triangle array
	//	glDrawArrays(GL_TRIANGLES, 0, triangleCount);

	//	// Unbind the VBO
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//}
	return true;
}

PVRShell* NewDemo()
{
	return new SunriseShell();
}