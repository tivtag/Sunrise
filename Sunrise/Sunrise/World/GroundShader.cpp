#include "GroundShader.hpp"

#include "..\FileSystem.hpp"
#include <glm\gtc\type_ptr.hpp>

const int VERTEX_ARRAY  = 0;
const int ATTRIB_COLOUR = 1;
const int ATTRIB_UV		= 2;
const int ATTRIB_NORMAL = 3;

sr::GroundShader::GroundShader(sr::FileSystem& _fileSystem)
    : fileSystem(_fileSystem), vertexShader(0), fragShader(0), programObject(0)
{
}

sr::GroundShader::~GroundShader()
{	
    if(vertexShader) glDeleteShader(vertexShader);
	if(fragShader) glDeleteShader(fragShader);    
    if(programObject) glDeleteProgram(programObject);
}

bool sr::GroundShader::Load()
{
    //	Fragment and vertex shaders code
	const std::string fragShaderSource = fileSystem.ReadAllText( "media/shaders/PointLight.fsh" );
	const std::string vertShaderSource = fileSystem.ReadAllText( "media/shaders/PointLight.vsh" );

	// Create and compile shaders
 	fragShader   = CompileFromSource(fragShaderSource, GL_FRAGMENT_SHADER);
 	vertexShader = CompileFromSource(vertShaderSource, GL_VERTEX_SHADER);

    if(!vertexShader || !fragShader)
    {
        return false;
    }

	// Create the shader program
    programObject = glCreateProgram();

	// Attach the fragment and vertex shaders to it
    glAttachShader(programObject, fragShader);
    glAttachShader(programObject, vertexShader);

	// Bind the custom vertex attributes
    glBindAttribLocation(programObject, VERTEX_ARRAY,  "a_vertex");
   glBindAttribLocation(programObject, ATTRIB_COLOUR, "a_color");
   glBindAttribLocation(programObject, ATTRIB_UV,     "a_uv");
   glBindAttribLocation(programObject, ATTRIB_NORMAL, "a_normal");
    			
	// Link the program
    glLinkProgram(programObject);

	// Check if linking succeeded in the same way we checked for compilation success
    GLint bLinked;
    glGetProgramiv(programObject, GL_LINK_STATUS, &bLinked);
	if (!bLinked)
	{
		return false;
	}
    
    // Cache shader parameter locations
	location_textureID     = glGetUniformLocation(programObject, "sampler2d");
    location_lightPosition = glGetUniformLocation(programObject, "LightPosition");

    location_model         = glGetUniformLocation(programObject, "Model");
    location_view          = glGetUniformLocation(programObject, "View");
    location_mvp           = glGetUniformLocation(programObject, "ModelViewProj");
    return true;
}

void sr::GroundShader::SetTexture(sr::TexturePtr texture)
{	
    glActiveTexture(GL_TEXTURE0);
    texture->Bind();
    glUniform1i(location_textureID, 0); // Set our "sampler2d" sampler to use Texture Unit 0
}

void sr::GroundShader::SetModel(const glm::mat4& model)
{
    const glm::mat4 mvp = viewProj * model;

    glUniformMatrix4fv( location_model, 1, GL_FALSE, glm::value_ptr(model) );
    glUniformMatrix4fv( location_mvp, 1, GL_FALSE, glm::value_ptr(mvp) );
}

void sr::GroundShader::SetViewProj(const glm::mat4& view, const glm::mat4& viewProj)
{
    this->viewProj = viewProj;
    glUniformMatrix4fv( location_view, 1, GL_FALSE, glm::value_ptr(view) );
}

void sr::GroundShader::SetSunLightPosition(const glm::vec3& position)
{
    glUniform3fv( location_lightPosition, 1, glm::value_ptr(position) );
}

void sr::GroundShader::Begin()
{    
	glEnable(GL_DEPTH_TEST);     // Enable depth test
	glDepthFunc(GL_LESS);	     // Accept fragment if it closer to the camera than the former one		
    glUseProgram(programObject); // Actually use the created program

    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
}

void sr::GroundShader::End()
{
}
