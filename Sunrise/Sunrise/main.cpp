#include "Shell/PVRShell.h"
#include "Shell/PVRTgles2Ext.h"
#include "Graphics/API.hpp"

#include <glm/glm.hpp>

#include <fstream>
#include <string>
#include <vector>
#include <boost/algorithm/string/replace.hpp>

#include "Graphics/ModelManager.hpp"
#include "Graphics/TextureManager.hpp"
#include "Graphics/Camera.hpp"
#include "Graphics/Image.hpp"
#include "Graphics/Model.hpp"

#include "World/GroundInfo.hpp"
#include "World/Ground.hpp"
#include "World/GroundShader.hpp"
#include "World/WorldInfo.hpp"
#include "World/ModelInstance.hpp"

#include "GravityResourceFile.hpp"
#include "Input.hpp"
#include "CameraController.hpp"
#include "FileSystem.hpp"

/*
	The main class of the platform-independent Sunrise renderer 
*/
class SunriseShell : public PVRShell
{	
	bool drawWireframe;
	int width, height;

    sr::GroundInfo groundInfo;
    sr::Ground ground;
    sr::GroundShader* shader;

    sr::WorldInfo worldInfo;

	sr::Input* input;
	sr::Camera camera;
	sr::CameraController* cameraController;
	sr::FileSystem* fileSystem;
    sr::TextureManager* textureManager;
    sr::ModelManager* modelManager;

    glm::vec3 sunPosition;
    
    std::vector<std::shared_ptr<sr::ModelInstance>> models;
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
	void LoadExtensions();
};

SunriseShell::SunriseShell()
	: input(nullptr), cameraController(nullptr), fileSystem(nullptr), textureManager(nullptr), shader(nullptr), drawWireframe(false), sunPosition(0), modelManager(nullptr)
{
}

float factor = 0.0f;

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
    delete shader;
    delete textureManager;
    delete modelManager;
    delete cameraController;
	delete input;
	delete fileSystem;
    return true;
}

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
    shader = new sr::GroundShader(*fileSystem);
    modelManager = new sr::ModelManager(*textureManager, *fileSystem);

	if( !shader->Load() )
    {
        std::cerr << "Could not load shader." << std::endl;
        return false;
    }
    
	//LoadCube();
	LoadWorld();
	LoadExtensions();
	
	// Init view
	width = PVRShellGet(prefWidth);
	height = PVRShellGet(prefHeight);
	camera.WindowSize( glm::uvec2(PVRShellGet(prefWidth), PVRShellGet(prefHeight)) );

	glViewport(0, 0, width, height);
	return true;
}

void SunriseShell::LoadExtensions()
{	
	if( CPVRTgles2Ext::IsGLExtensionSupported( "GL_EXT_texture_filter_anisotropic" ) )
	{
		std::cout << "GL_EXT_texture_filter_anisotropic supported";
	}
	else
	{
		std::cout << "GL_EXT_texture_filter_anisotropic not supported!! :(";
	}
}

void SunriseShell::LoadWorld()
{
	std::string gndPath = fileSystem->GetFullPath( "media\\maps\\ra\\ra.gnd" );//ra\\ra.gnd glast_01 juperos_02 moc_prydb1";
	std::string rswPath = fileSystem->GetFullPath( "media\\maps\\ra\\ra.rsw" );

	std::ifstream gndStream, rswStream;
	gndStream.open(gndPath, std::ios::in | std::ios::binary);
	rswStream.open(rswPath, std::ios::in | std::ios::binary);

	if( gndStream.is_open() )
	{
		if(!groundInfo.ReadStream(gndStream))
        {
            std::cerr << "Error reading GroundInfo " << gndPath << std::endl;
        }

        if(!worldInfo.ReadStream(rswStream))
        {
            std::cerr << "Error reading WorldInfo " << rswPath << std::endl;
        }

        //sr::GravityResourceFile grf;        
        //grf.Open("E:\\Spiele\\RebirthRO\\gf\\bdata.grf");
        ////worldInfo.ExtractFiles(grf, *fileSystem);
        
        //worldInfo.ExtractFiles(grf, *fileSystem);
        //groundInfo.ExtractFiles(grf, *fileSystem);
        
        auto worldModels = worldInfo.Models();

        int count = 0;
        float tileSize = 10.0f;
        
	    float xOffset = tileSize * groundInfo.Width() / 2;
	    float yOffset = tileSize * groundInfo.Height() / 2;

        for(auto itr = std::begin(worldModels); itr != std::end(worldModels); ++itr)
        {
            const sr::WorldInfo::ModelObject& worldModel = **itr;

            //if(!modelManager->HasInfo(worldModel.ModelName()))
            //{
            //    auto info = modelManager->LoadInfo(worldModel.ModelName());
            //    info->ExtractFiles(grf, *fileSystem);
            //}

            const sr::ModelPtr model = modelManager->Load(worldModel.ModelName());
                        
            if(model)
            {
                std::shared_ptr<sr::ModelInstance> instance = std::make_shared<sr::ModelInstance>(model, worldModel);
                models.push_back(instance);

                auto position = instance->Position() + glm::vec3(xOffset, 0.0f, yOffset);
                instance->Position(position);
            }
            else
            {
                std::cerr << "Missing model" << std::endl;               
            }

            ++count;

			if(count > 500)
			{
				break;
			}
        }

        ground.Load(groundInfo, *textureManager);
	}
    else
    {
        std::cerr << "Error reading file " << gndPath << std::endl;
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
	return true;
}

int frame = 0;
bool first = true;

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
	if( input->KeyPressed(sr::Key::KC_F3) )
	{
		first = !first;
	}
    	
    if( input->KeyPressed(sr::Key::KC_C) )
	{
		factor -= 0.1f;

        if(factor < 0.0f)
            factor = 0.0f;
	}
    	
    if( input->KeyPressed(sr::Key::KC_V) )
	{
		factor += 0.1f;

        if(factor > 1.0f)
            factor = 1.0f;
	}

    if( input->KeyPressed(sr::Key::KC_SPACE) )
    {
        sunPosition = camera.Eye();
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
	glDisable(GL_BLEND);

	++frame;

    sr::GroundShader& shader = *this->shader;
    shader.Begin();
    {		
        shader.SetSunLightPosition(sunPosition);
        shader.SetViewProj(camera.View(), camera.ViewProj() );
        
        //Rotate -90 degrees about the z axis
        
	    float sizex = groundInfo.Zoom() * groundInfo.Width();
	    float sizey = groundInfo.Zoom() * groundInfo.Height();

        glm::mat4 rotM(1.0f);
        //auto rot = glm::value_ptr(rotM);


        //rot[0] = 1.0f;
        //rot[1] = 0.0f;
        //rot[2] = 0.0f;
        //rot[3] = 0.0f;

        //rot[4] = 0.0f;
        //rot[5] = 1.0f;
        //rot[6] = 0.0f;
        //rot[7] = 0.0f;

        //rot[8] = 0.0f;
        //rot[9] = 0.0f;
        //rot[10] = -1.0f;
        //rot[11] = 0.0f;

        //rot[12] = factor * -sizex/2;
        //rot[13] = 0.0f;
        //rot[14] = factor * sizey/2;
        //rot[15] = 1.0f;

        shader.SetModel(rotM);
        ground.Render(shader);

        for(auto itr = std::begin(models); itr != std::end(models); ++itr)
        {
            (*itr)->Render(shader);
        }
    }
    shader.End();

	return true;
}

PVRShell* NewDemo()
{
	return new SunriseShell();
}
