#include <Libraries/GL_Loaders/GL/gl_core_4_3.h>
#include "StdAfx.h"
#include "OGLGrpWrapper.h"
#include "FileManagement/DataStream.h"
#include "Constants/GraphicsConstants.h"
#include "Constants/AllocTypes.h"
#include "Stats/Profiling.h"
#include "DebugUtils/Assertions.h"
#include "Rendering/Camera/Camera.h"

#ifndef RENDERER_HARD_ASSERT
//#define RENDERER_HARD_ASSERT
#endif

using namespace LeEK;

const U32 NUM_CUBE_POINTS = 8;

const F32 OGLGrpWrapper::cubePos[NUM_CUBE_POINTS * 3] = 
	{
		-0.5f, -0.5f, 0.5f,		//left lower back (LLB)
		0.5f, -0.5f, 0.5f,		//RLB
		-0.5f, 0.5f, 0.5f,		//LUB
		0.5f, 0.5f, 0.5f,		//RUB
		-0.5f, -0.5f, -0.5f,	//LLF
		0.5f, -0.5f, -0.5f,		//RLF
		-0.5f, 0.5f, -0.5f,		//LUF
		0.5f, 0.5f, -0.5f,		//RUF
	};

const U32 OGLGrpWrapper::cubeIndex[(2*NUM_CUBE_POINTS)-2] = {
		0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1
	};

char* OGLGrpWrapper::dbgShdrName = "_debugDraw";

F32* spherePos;
U32* sphereIndex;

U32 sphereRings = 20;
U32 sphereSectors = 20;
U32 vertsPerRow = sphereRings+1;
U32 vertsPerCol = sphereSectors+1;
U32 numSphrPoints = (vertsPerRow)*(vertsPerCol);
U32 numSphrIndices = sphereRings * sphereSectors * 6;

GLint texInputTypes[Texture2D::PIXTYPE_LEN];
GLenum texOutputTypes[Texture2D::PIXTYPE_LEN];
GLenum texDataWidths[Texture2D::PIXTYPE_LEN];
GLint comprFormatTypes[Texture2D::COMPTYPE_LEN];

//functions for setting up format lookup tables.
void initTexFormatTable()
{
	texInputTypes[Texture2D::RGB8] = GL_RGB;
	texOutputTypes[Texture2D::RGB8] = GL_RGBA;

	texInputTypes[Texture2D::RGB565] = GL_RGB;
	texOutputTypes[Texture2D::RGB565] = GL_RGBA;

	texInputTypes[Texture2D::RGBA8] = GL_RGBA;
	texOutputTypes[Texture2D::RGBA8] = GL_RGBA;

	texInputTypes[Texture2D::BGRA8] = GL_BGRA;
	texOutputTypes[Texture2D::BGRA8] = GL_RGBA;

	texInputTypes[Texture2D::BYTE] = GL_RED;
	texOutputTypes[Texture2D::BYTE] = GL_RED;

	texDataWidths[Texture2D::RGB8] = GL_UNSIGNED_BYTE;
	texDataWidths[Texture2D::RGB565] = GL_UNSIGNED_SHORT_5_6_5;
	texDataWidths[Texture2D::RGBA8] = GL_UNSIGNED_BYTE;
	texDataWidths[Texture2D::BGRA8] = GL_UNSIGNED_BYTE;
	texDataWidths[Texture2D::BYTE] = GL_UNSIGNED_BYTE;
}

void initCompFormatTable()
{
	comprFormatTypes[Texture2D::NONE] = 0;
	comprFormatTypes[Texture2D::DXT1] = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	//TODO: figure out how the hell to do crunch
}
GLenum lastGlobalErr = GL_NO_ERROR;
Map<GLenum, String> errStrings;
void OGLGrpWrapper::_assertNoErr(const char* file, int line) 
{
	if(!isContextSet())
	{
		LogE(String("Attempted OpenGL call at ") + file + ", line " + line + " when no context was set!");
		return;
	}

	lastGlobalErr = GL_NO_ERROR;
	lastGlobalErr = glGetError();
	int numErrs = 0;
	const int MAX_ERRS = 3;
	while(lastGlobalErr != GL_NO_ERROR && numErrs < MAX_ERRS)
	{
		LogE(String("GL error tripped: at ") + file + ", line " + line + ": " + errStrings[lastGlobalErr]);
#ifdef RENDERER_HARD_ASSERT
		L_ASSERT(false);
#endif
		lastGlobalErr = glGetError();
		++numErrs;
	}
	lastGlobalErr = GL_NO_ERROR;
}

#if defined(_DEBUG) || defined(RELDEBUG)
#define assertNoErr() _assertNoErr(__FILE__, __LINE__)
#else
#define assertNoErr()
#endif

bool OGLGrpWrapper::buildDbgSphere()
{
	/*
	 *From StackOverflow:
	 *If there are M lines of latitude (horizontal) and N lines of longitude (vertical), then put dots at
	 *(x, y, z) = (sin(Pi * m/M) cos(2Pi * n/N), sin(Pi * m/M) sin(2Pi * n/N), cos(Pi * m/M))
	 *for each m in { 0, ..., M } and n in { 0, ..., N-1 } and draw the line segments between the dots, accordingly.
	 *edit: maybe adjust M by 1 or 2 as required, because you should decide whether or not to count "latitude lines" at the poles
	 */
	//we'll use 10 U, 10 V
	//there's 10 sphereRings, each ring has (sphereSectors) = 10 points
	//add 2 points for the top and bottom
	//must render first and last (sphereSectors + 1) points as a TRIANGLE_FAN
	//alternate plan:
	//each U and V is a circle
	//so a ring consists of (2*sphereSectors) points
	//rest as TRIANGLE_STRIPS
	
	//we need to account for the extra points on the poles
	//these won't be quads, they'll basically be a triangle fan
	//even though we won't be addressing them as such
	
	//we'll operate by small angles
	F32 theta = 0.0f;
	F32 phi = 0.0f;
	//sphere vertically spans from -.5Pi to .5Pi
	//horizontally from 0 to 2Pi
	//so split those arcs by our number of rows/cols
	F32 vertArcStride = Math::PI / sphereSectors;
	F32 horizArcStride = Math::TWO_PI / sphereRings;
	
	//loop over each ring
	//going from top to bottom, counterclockwise
	U32 startIndex = 0;
	for(U32 u = 0; u < vertsPerCol; u++)
	{
		//we'll start on the top of the sphere
		theta = Math::PI_OVER_2 - (vertArcStride * u);
		//then over each point in the ring
		for(U32 v = 0; v < vertsPerRow; v++)
		{
			phi = horizArcStride * v;
			//each vert is * 3 floats long,
			//but each ring is vertsPerRow long.
			//to index a specific row, move by (u * vertsPerCol)
			//then index into the particular point in the ring, that's (3*v) floats
			//U32 startIndex = 3*v + u*vertsPerCol;
			//scale by our vert position along the sphere
			/*spherePos[startIndex] = Math::Cos(theta) * Math::Cos(phi);		//x
			spherePos[startIndex + 1] = Math::Cos(theta) * Math::Sin(phi);	//y
			spherePos[startIndex + 2] = Math::Sin(theta);					//z*/
			spherePos[startIndex] =		Math::Cos(theta) * Math::Cos(phi);	//x
			spherePos[startIndex + 1] = Math::Sin(theta);					//y
			spherePos[startIndex + 2] = Math::Cos(theta) * Math::Sin(phi);	//z

			startIndex += 3;
			L_ASSERT(startIndex <= (numSphrPoints*3));
		}
	}
	//now build indices
	//each triangle is 3 verts
	//there's a quad at each 4 points, so that's 6 indices per quad
	
	//again, iterate from top to bottom, counterclockwise
	startIndex = 0;
	for(U32 u = 0; u < sphereSectors; u++)
	{
		//then over each point in the ring
		for(U32 v = 0; v < sphereRings; v++)
		{
			U32 lt = v + u * (vertsPerRow);
			U32 rt = (v+1) + u * (vertsPerRow);
			U32 lb = v + (u+1) * (vertsPerRow);
			U32 rb = (v+1) + (u+1) * (vertsPerRow);
			//now our start is a little different
			sphereIndex[startIndex] = rt;
			sphereIndex[startIndex+1] = lt;
			sphereIndex[startIndex+2] = lb;

			sphereIndex[startIndex+3] = lb;
			sphereIndex[startIndex+4] = rb;
			sphereIndex[startIndex+5] = rt;
			startIndex += 6;
			L_ASSERT(startIndex <= numSphrIndices);
		}
	}
	return true;
}

bool OGLGrpWrapper::buildDebugMeshes()
{
	//try to gen sphere data
	if(!buildDbgSphere())
	{
		return false;
	}
	return true;
}

bool OGLGrpWrapper::loadDebugFunctions()
{
	//load error to string conversion map
	errStrings = Map<GLenum, String>();
	errStrings[GL_NO_ERROR] = "No error.";
	errStrings[GL_INVALID_ENUM] = "Invalid enum value!";
	errStrings[GL_INVALID_VALUE] = "Invalid numeric value!";
	errStrings[GL_INVALID_OPERATION] = "Attempted illegal operation!";
	errStrings[GL_INVALID_FRAMEBUFFER_OPERATION] = "Attempted operation on incomplete framebuffer!";
	errStrings[GL_OUT_OF_MEMORY] = "Out of memory!";
	errStrings[GL_STACK_UNDERFLOW] = "Attempted stack underflow!";
	errStrings[GL_STACK_OVERFLOW] = "Attempted stack overflow!";

	//TODO
	//load drawing functions
	Vector< ShaderFilePair > paths;
	paths.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "/Shaders/Debug/DebugWireframe.vp").c_str())) );
	paths.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "/Shaders/Debug/DebugWireframe.fp").c_str())) );
	auto result = MakeShader(dbgShdrName, 2, paths );
	L_ASSERT(result && result->ProgramHandle() != 0);
	if(!result)
	{
		LogE("Could not load debug shaders!");
		return false;
	}

	//next, generate array
	glGenBuffers(1, &debugVertArrayHnd);
	assertNoErr();
	glGenBuffers(1, &debugIndexHnd);
	assertNoErr();
	glGenVertexArrays(1, &debugVAOHnd);
	assertNoErr();

	//build any mesh data
	if(!buildDebugMeshes())
	{
		LogE("Could not build debug meshes!");
		return false;
	}

	return true;
}

bool OGLGrpWrapper::isContextSet()
{
	return context != NULL;//((Win32Platform*)IPlatform::GetIPlatform())->GetOGLCtx();
}

bool OGLGrpWrapper::isGeomEnum(GLenum enumVal) const
{
	return enumVal == GL_LINES || enumVal == GL_LINE_LOOP || enumVal == GL_LINE_STRIP ||
		enumVal == GL_TRIANGLES || enumVal == GL_TRIANGLE_FAN || enumVal == GL_TRIANGLE_STRIP ||
		enumVal == GL_POINTS;
}

bool OGLGrpWrapper::isLineGeomEnum(GLenum enumVal) const
{
	return enumVal == GL_LINES || enumVal == GL_LINE_LOOP || enumVal == GL_LINE_STRIP;
}

void OGLGrpWrapper::debugDraw(size_t numVerts, size_t numInds, const F32* vertData, const U32* indData, const Matrix4x4& pWorldMat, const Color& color, GLenum meshType)
{
	if(!isGeomEnum(meshType))
	{
		return;
	}
	bool isLineType = isLineGeomEnum(meshType);
	//switch over to debug drawing, and prep buffers
	SetShader(dbgShdrName);
	glUseProgram(currentProgram->ProgramHandle());
	assertNoErr();
	//additional check -
	//make sure the shader program's
	//not invalid here
	GLint shaderProg = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProg);
	L_ASSERT(shaderProg != 0);
	glBindVertexArray(debugVAOHnd);
	//the vert attrib array needs to be active
	glBindBuffer(GL_ARRAY_BUFFER, debugVertArrayHnd);
	assertNoErr();
	//glClearBufferData(GL_ARRAY_BUFFER, GL_RGB32F, GL_RGB, GL_FLOAT, NULL);
	glBufferData(GL_ARRAY_BUFFER, numVerts * 3 * sizeof(F32), vertData, GL_STATIC_DRAW);
	assertNoErr();
	glEnableVertexAttribArray(POSITION);
	assertNoErr();
	//inform the shader of where the position data is in the buffer
	glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
	assertNoErr();
	//pass the color uniform
	glUniform3fv(currentProgram->GetUniformHandle("colorVec"), 1, color.GetRGB().ToFloatArray());
	assertNoErr();
	//now pass index data, if necessary
	if(!isLineType)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debugIndexHnd);
		assertNoErr();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, numInds * sizeof(U32), indData, GL_STATIC_DRAW);
		assertNoErr();
		//temporarily disable back culling so we get the hidden lines
		glDisable(GL_CULL_FACE);
		assertNoErr();
	}
	//temporarily set an altered world matrix
	Matrix4x4 origWorld = worldMat;
	SetWorldViewProjection(pWorldMat, viewMat, projectionMat);
	//and draw in wireframe
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	assertNoErr();
	//now draw
	if(isLineType)
	{
		glDrawArrays(meshType, 0, numVerts);
	}
	else
	{
		glDrawElements(meshType, numInds, GL_UNSIGNED_INT, 0);
	}
	assertNoErr();
	//restore drawing state
	glEnable(GL_CULL_FACE);
	assertNoErr();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	assertNoErr();
	//now unbind data
	if(!isLineType)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		assertNoErr();
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	assertNoErr();
	glBindVertexArray(0);
	//glDisableVertexAttribArray(POSITION);
	glUseProgram(0);
	assertNoErr();
	//restore world matrix
	worldMat = origWorld;
}

//replace with typeid?
#pragma region Class Functions
OGLGrpWrapper::OGLGrpWrapper(void) : currentProgram()
{
	
	worldMat = Matrix4x4::Identity;
	viewMat = Matrix4x4::Identity;
	projectionMat = Matrix4x4::Identity;
	screenAspect = -1.0f;
	screenRes = Vector2::Zero;
	enableVSync = false;
	context = NULL;

	vendorString, rendererString;
	shaderProgramList = HashMap<TypedHandle<Shader>>();
	currentShaderList = Vector<GLuint>();
	currentProgram = TypedHandle<Shader>();
	//debug drawing members
	debugProgram = NULL;
	debugVAOHnd = 0;
	debugVertArrayHnd = 0;
	debugIndexHnd = 0;
	contextSet = false;

	initTexFormatTable();
	initCompFormatTable();
}

OGLGrpWrapper::~OGLGrpWrapper(void)
{
}

#pragma region Properties
const void* OGLGrpWrapper::Context() const
{
	return context;
}

void OGLGrpWrapper::SetContext(void* newCtx)
{
	context = newCtx;
}
#pragma endregion

#pragma region Interface Implementation
void OGLGrpWrapper::SetScreenResolution(const Vector2& val)
{
	screenRes = val;
	screenAspect = screenRes.Y() == 0 ? -1.0f : screenRes.X() / screenRes.Y();
}

bool OGLGrpWrapper::Startup()
{
	//init dynamic vals
	//remember that sphere points consist of 3 floats!
	spherePos = CustomArrayNew<F32>(numSphrPoints*3, RENDERER_ALLOC, "DebugMeshInit");
	sphereIndex = CustomArrayNew<U32>(numSphrIndices, RENDERER_ALLOC, "DebugMeshInit");
	shaderProgramList = HashMap<TypedHandle<Shader>>();
	//clear depth buffer
	glClearDepth(1.0f);
	assertNoErr();
	//enable depth testing
	glEnable(GL_DEPTH_TEST);
	assertNoErr();
	//set polygon winding to counterclockwise
	glFrontFace(GL_CCW);
	assertNoErr();
	//enable back face culling
	glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	assertNoErr();
	//also enable transparency
	glEnable(GL_BLEND);
	assertNoErr();
	//set a default blending function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	assertNoErr();

	//setup world matrix
	worldMat = Matrix4x4::Identity;

	//setup field of view and the screen's aspect ratio
	//fieldOfView = Math::PI_OVER_4; //for now, default is 90 degrees

	//make a perspective projection matrix (doesn't exist in Matrix4x4 at the moment...
	//load debug functions and shaders
	if(!loadDebugFunctions())
	{
		LogE("Failed to initialize API's debug functions!");
	}

	//pull video card name
	vendorString = (char*)glGetString(GL_VENDOR);
	assertNoErr();
	rendererString = (char*)glGetString(GL_RENDERER);
	assertNoErr();
	LogD(String("OpenGL API initialized: ") + vendorString + " - " + rendererString + ", OpenGL version " + (char*)glGetString(GL_VERSION));
	assertNoErr();

	//intialize lookup tables if needed

	return true;
}

bool OGLGrpWrapper::InitGeometry(Geometry& mesh)
{
	if(!isContextSet())
	{
		LogE("OpenGL context not set, can't initialize geometry!");
		return false;
	}
	//PROFILE("InitGeometry");
	//Follows a simple order:
	//generate VAOs and buffers if the mesh is missing them
	//set mesh's handles as active handles so shaders have proper attrib data
	//enable attribute arrays
	//make call to glDrawElements
	//if(mesh.VertexArrayHandle() == 0)
	//{
		U32 VAOHandle;
		glGenVertexArrays(1, &VAOHandle);
		mesh.SetVertexArrayHandle(VAOHandle);
	//}
	glBindVertexArray(mesh.VertexArrayHandle());
	assertNoErr();
	//load data into the vertex buffer
	//if(mesh.VertexBufferHandle() == 0)
	//{
		U32 VertHandle;
		glGenBuffers(1, &VertHandle);
		mesh.SetVertexBufferHandle(VertHandle);
	//}
	glBindBuffer(GL_ARRAY_BUFFER, mesh.VertexBufferHandle());
	assertNoErr();
	glBufferData(GL_ARRAY_BUFFER, mesh.VertexCount() * sizeof(Vertex), mesh.Vertices(), GL_STATIC_DRAW);
	assertNoErr();
	//about to recieve pointers to individual pieces of data; enable VAO attributes
	glEnableVertexAttribArray(POSITION);
	glEnableVertexAttribArray(COLOR);
	glEnableVertexAttribArray(NORMAL);
	glEnableVertexAttribArray(UV0);
	assertNoErr();
	//notify OpenGL where position data is in geometry
	//glBindBuffer(GL_ARRAY_BUFFER, mesh.VertexBufferHandle());
	//pass data to attribute 0, position
	//last param is an OFFSET to the pointer to the data
	glVertexAttribPointer(	POSITION,
							3,				//3 components in a Vector3
							GL_FLOAT,		//uses floats
							false,			//don't normalize values
							sizeof(Vertex),	//stride by vert size
							0);				//offset to position component within each vertex
	assertNoErr();
	//next, pass data to attribute 1, color
	//since color's the second element in Vertex, you have to offset by one vector
	//glBindBuffer(GL_ARRAY_BUFFER, mesh.VertexBufferHandle());
	glVertexAttribPointer(COLOR, 3, GL_FLOAT, false, sizeof(Vertex), (unsigned char*)0 + (sizeof(Vector3)));
	assertNoErr();
	//and finally, pass data to normals
	//glBindBuffer(GL_ARRAY_BUFFER, mesh.VertexBufferHandle());
	glVertexAttribPointer(NORMAL, 3, GL_FLOAT, false, sizeof(Vertex), (unsigned char*)0 + (2*sizeof(Vector3)));
	assertNoErr();
	//change to use Vector2s?
	glVertexAttribPointer(UV0, 2, GL_FLOAT, false, sizeof(Vertex), (unsigned char*)0 + (3*sizeof(Vector3)));
	assertNoErr();
	//now generate and pass vertex indices
	//if(mesh.IndexBufferHandle() == 0)
	//{
		U32 IndexHandle;
		glGenBuffers(1, &IndexHandle);
		mesh.SetIndexBufferHandle(IndexHandle);
	//}
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.IndexBufferHandle());
	assertNoErr();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.IndexCount() * sizeof(U32), mesh.Indices(), GL_STATIC_DRAW);
	assertNoErr();
	//TODO: handle normals

	//unbind buffers to prep another mesh
	//unclear, but may be dangerous/slow?
	//passing null signals that buffer can be cleared
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	assertNoErr();
	//glEnableVertexAttribArray(NULL); //and disable the attributes
	//and unbind the VAO
	
	return true;
}

bool OGLGrpWrapper::InitText(Text& text)
{
	//PROFILE("InitGeometry");
	//Follows a simple order:
	//generate VAOs and buffers if the mesh is missing them
	//set mesh's handles as active handles so shaders have proper attrib data
	//enable attribute arrays
	//make call to glDrawElements
	TextGeom& mesh = text.GetGeometry();
	//destroy any existing handles
	U32 VAOHandle = mesh.VertexArrayHandle();
	U32 VertHandle = mesh.VertexBufferHandle();
	U32 IndexHandle = mesh.IndexBufferHandle();
	glDeleteBuffers(1, &VertHandle);
	glDeleteBuffers(1, &IndexHandle);
	glDeleteVertexArrays(1, &VAOHandle);
	assertNoErr();

	glGenVertexArrays(1, &VAOHandle);
	mesh.SetVertexArrayHandle(VAOHandle);
	glBindVertexArray(mesh.VertexArrayHandle());
	assertNoErr();

	//load data into the vertex buffer
	//unlike with geometry, text only uses position data -
	//it's not affected by lighting, and color is a uniform on each character if not the entire string
	//U32 VertHandle;
	glGenBuffers(1, &VertHandle);
	mesh.SetVertexBufferHandle(VertHandle);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.VertexBufferHandle());
	glBufferData(GL_ARRAY_BUFFER, mesh.VertexCount() * sizeof(TextVertex), mesh.Vertices(), GL_STATIC_DRAW);
	assertNoErr();

	//about to recieve pointers to individual pieces of data; enable VAO attributes
	glEnableVertexAttribArray(POSITION);
	glEnableVertexAttribArray(UV0);
	assertNoErr();

	//notify OpenGL where position data is in geometry
	//glBindBuffer(GL_ARRAY_BUFFER, mesh.VertexBufferHandle());
	//pass data to attribute 0, position
	//last param is an OFFSET to the pointer to the data
	glVertexAttribPointer(	POSITION,
							3,				//only 1 Vec3!
							GL_FLOAT,		//uses floats
							false,			//don't normalize values
							sizeof(TextVertex),	//still stride by vertex size
							0);				//offset to position component within each vertex

	//texcoords are important!
	glVertexAttribPointer(UV0, 2, GL_FLOAT, false, sizeof(TextVertex), (unsigned char*)0 + (sizeof(Vector3)));
	assertNoErr();

	//now generate and pass vertex indices
	//U32 IndexHandle;
	glGenBuffers(1, &IndexHandle);
	mesh.SetIndexBufferHandle(IndexHandle);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.IndexBufferHandle());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.IndexCount() * sizeof(U32), mesh.Indices(), GL_STATIC_DRAW);
	assertNoErr();
	//TODO: handle normals

	//unbind buffers to prep another mesh
	//unclear, but may be dangerous/slow?
	//passing null signals that buffer can be cleared
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//glEnableVertexAttribArray(NULL); //and disable the attributes
	//and unbind the VAO
	assertNoErr();

	return true;
}

bool OGLGrpWrapper::InitTexture(Texture2D& tex, TextureMeta::MapType type)
{
	//generate texture handle
	L_ASSERT(tex.TextureBufferHandle == 0 && "Attempted to reinitialize a texture in OpenGL!");
	//have no idea what the texture's state was beforehand, and we're not touching it;
	//notify that no initialization happened
	if(tex.TextureBufferHandle != 0)
	{
		return false;
	}
	GLint activeTex = GL_TEXTURE0 + type;
	//glActiveTexture(activeTex);
	glActiveTexture(GL_TEXTURE0);
	assertNoErr();
	U32 texBufHnd = 0;
	glGenTextures(1, &texBufHnd);
	assertNoErr();
	tex.TextureBufferHandle = texBufHnd;
	//and setup the texture info
	glBindTexture(GL_TEXTURE_2D, tex.TextureBufferHandle);
	assertNoErr();

	//load the texture data
	glTexImage2D(	GL_TEXTURE_2D,	//2D texture
					0,				//base level of mipmap
					texOutputTypes[tex.PixType], //way the pixels should be stored in VRAM
					tex.Width,
					tex.Height,
					0,				//border never worked!
					texInputTypes[tex.PixType], //GL_RGBA,	//way the pixels are stored in the res manager
					texDataWidths[tex.PixType], //GL_UNSIGNED_BYTE,
					tex.Data);
	assertNoErr();
	GLint err = glGetError();
	if(err != GL_NO_ERROR)
	{
		LogE(StrFromVal(err));
	}

	//setup tex parameters.
	//TODO: make customizable?
	//Set edge behavior - by default, clamp.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	assertNoErr();
	if(err != GL_NO_ERROR)
	{
		LogE(StrFromVal(err));
	}
	//mipmap levels...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	assertNoErr();
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	//and filtering - basically, do trilinear.
	//Magnification is linear, minification is linear w/ linear mipmapping
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	assertNoErr();
	if(err != GL_NO_ERROR)
	{
		LogE(StrFromVal(err));
	}
	//generate mipmaps if needed1
	if(!tex.HasMipMap)
	{
		//apparently, ATI drivers as of 2011 have a bug
		//where you need to glEnable GL_TEXTURE_2D
		//for glGenerateMipmap to do anything.
		//Not clear if this does anything on other platforms.
		//glEnable(GL_TEXTURE_2D);
		assertNoErr();
		glGenerateMipmap(GL_TEXTURE_2D);
		assertNoErr();
	}

	//texture is now loaded - in theory, you can release the data now
	//unbind texture for next texture to come
	glBindTexture(GL_TEXTURE_2D, 0);
	assertNoErr();
	return true;
}

void OGLGrpWrapper::Shutdown()
{
	//delete all the shaders
	for(HashMap<TypedHandle<Shader>>::iterator it = shaderProgramList.begin(); it != shaderProgramList.end(); ++it)
	{
		glDeleteProgram(it->second->ProgramHandle());
		//now release the memory of the shader pointer
		HandleMgr::DeleteHandle(it->second);
	}
	//the shaders are now all invalidated; clear the map
	shaderProgramList.clear();

	//currentProgram = NULL;
	//disable any active attribute arrays
	glDisableVertexAttribArray(0); //usually position
	glDisableVertexAttribArray(1); //usually color. Move to consts?

	//unbind any vert & index buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//release any active vertex array.
	glBindVertexArray(0);

	//glDeleteBuffers(1, &vertexBufferHnd);
	//glDeleteBuffers(1, &indexBufferHnd);
	glDeleteBuffers(1, &debugVertArrayHnd);
	}

//Can only be called AFTER OGLGrpWrapper::Shutdown.
void OGLGrpWrapper::ShutdownMesh(const Geometry& mesh)
{
	if(!isContextSet())
	{
		LogW("OpenGL context not set, can't shutdown geometry!");
		return;
	}
	//TODO: call unbindings in a ShutdownModel() function?

	//delete any buffers/arrays the mesh might have a handle to
	if(mesh.VertexBufferHandle() != 0)
	{
		glDeleteBuffers(1, mesh.VertexBufferHandlePtr());
		assertNoErr();
	}
	if(mesh.IndexBufferHandle() != 0)
	{
		glDeleteBuffers(1, mesh.IndexBufferHandlePtr());
		assertNoErr();
	}
	if(mesh.VertexArrayHandle() != 0)
	{
		glDeleteVertexArrays(1, mesh.VertexArrayHandlePtr());
		assertNoErr();
	}
}

void OGLGrpWrapper::ShutdownText(const Text& text)
{
	const TextGeom& mesh = text.GetGeometry();
	//delete any buffers/arrays the mesh might have a handle to
	if(mesh.VertexBufferHandle() != 0)
	{
		glDeleteBuffers(1, mesh.VertexBufferHandlePtr());
	}
	if(mesh.IndexBufferHandle() != 0)
	{
		glDeleteBuffers(1, mesh.IndexBufferHandlePtr());
	}
	if(mesh.VertexArrayHandle() != 0)
	{
		glDeleteVertexArrays(1, mesh.VertexArrayHandlePtr());
	}
}

void OGLGrpWrapper::ShutdownTexture(Texture2D& tex)
{
	glDeleteTextures(1, &tex.TextureBufferHandle);
	tex.TextureBufferHandle = 0;
}

void OGLGrpWrapper::Clear(Color c)
{
	// Set the color to clear the screen to.
	glClearColor(c.R(), c.G(), c.B(), c.A()); 
	assertNoErr();

	// Clear the screen and depth buffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	assertNoErr();
}

void OGLGrpWrapper::Draw(const Geometry& mesh)
{
	PROFILE("DrawMesh");
	//simple - draw the mesh if it has VAO data
	if(mesh.VertexArrayHandle() != 0)
	{
		//glUseProgram(currentProgram->ProgramHandle());
		//glBindBuffer(GL_ARRAY_BUFFER, mesh.VertexBufferHandle());
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.IndexBufferHandle());
		glBindVertexArray(mesh.VertexArrayHandle());
		assertNoErr();
		//pass null as index pointer to indicate that a buffer should be used
		glDrawElements(	GL_TRIANGLES,		//doesn't matter as much as in glDrawArrays
						mesh.IndexCount(),	//number of indices
						GL_UNSIGNED_INT,	//index format
						0);					//indices not here, search an active buffer
		assertNoErr();
		//unbind to prep for next mesh?
		//or leave that up to DrawScene()?
		glBindVertexArray(0);
		assertNoErr();
		//glUseProgram(0);
	}
}

void OGLGrpWrapper::Draw(Text& text)
{
	PROFILE("DrawMesh");
	const TextGeom& mesh = text.GetGeometry();
	if(!text.GeometryReady())
	{
		text.RebuildGeometry();
		if(!InitText(text))
		{
			return;
		}
	}
	if(mesh.VertexArrayHandle() != 0)
	{
		//glUseProgram(currentProgram->ProgramHandle());
		//glBindBuffer(GL_ARRAY_BUFFER, mesh.VertexBufferHandle());
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.IndexBufferHandle());
		glBindVertexArray(mesh.VertexArrayHandle());
		assertNoErr();
		//pass null as index pointer to indicate that a buffer should be used
		glDrawElements(	GL_TRIANGLES,		//doesn't matter as much as in glDrawArrays
						mesh.IndexCount(),	//number of indices
						GL_UNSIGNED_INT,	//index format
						0);					//indices not here, search an active buffer
		assertNoErr();
		//unbind to prep for next mesh?
		glBindVertexArray(0);
		assertNoErr();
		//glUseProgram(0);
		//or leave that up to DrawScene()?
	}
}

//The debug rendering commands.
//Note that these all currently interrupt the API,
//loading the debug shaders and immediately drawing the requested primitives.
//This must be overhauled when we have a renderer system outlined!
#pragma region Debug Drawing Commands
void OGLGrpWrapper::DebugDrawLine(const Vector3& start, const Vector3& end, const Color& color)
{
	//build data
	F32 data[6] = { start.X(), start.Y(), start.Z(),
					end.X(), end.Y(), end.Z() };
	debugDraw(	2,		0, 
				data,	NULL,
				worldMat, color, GL_LINES);
}

void OGLGrpWrapper::DebugDrawPlane(const Vector3& origin, const Vector3& normal, const Color& color)
{
	//build data
	const U32 NUM_BASIS = 3;
	const I32 NUM_PLANE_PTS = 4;
	//Length of the normal line.
	F32 normalLineLen = 5;
	//Extent (half width/height) of the
	//square denoting the plane.
	F32 planeExtent = 1.5 * normalLineLen;
	//First, figure out the normal.
	Vector3 normalizedNorm = normal.GetNormalized();
	//draw the normal line first.
	DebugDrawLine(origin, origin + (normalLineLen * normalizedNorm), color);
	Vector3 basis[NUM_BASIS] = { Vector3(1,0,0), Vector3(0,1,0), Vector3(0,0,1) };
	Vector3 tangent = Vector3::Zero;
	Vector3 biTangent = Vector3::Zero;
	//Now figure out how to draw the plane. We'll use a line loop for it -
	//draw a square around the normal that is on the plane.
	//This means figuring out the plane's tangent, though.
	//We can use basis vectors; as long as the basis is not colinear with the normal,
	//this will yield a tangent.
	for(U32 i = 0; i < NUM_BASIS; ++i)
	{
		Vector3& basisVec = basis[i];
		if(Math::ApproxEqual(basisVec.Dot(normalizedNorm), 1.0f))
		{
			continue;
		}
		//Normalize the tangent, then calculate the bitangent.
		tangent = normalizedNorm.Cross(basisVec).GetNormalized();
		biTangent = normalizedNorm.Cross(tangent).GetNormalized();
		break;
	}
	//Skip this if we somehow couldn't get a good tangent.
	if(Math::ApproxEqual(tangent.LengthSquared(), 0.0f) || Math::ApproxEqual(biTangent.LengthSquared(), 0.0f))
	{
		return;
	}
	//otherwise, start building the vertex data.
	F32 planePts[NUM_PLANE_PTS * 3] = { 0.0f };
	for(I32 i = 0; i < NUM_PLANE_PTS; ++i)
	{
		//need to make this go in a circle, either CCW or CW
		I32 bothBitsSet =	((i & 1 << 0) >> 0) ^ 
							((i & 1 << 1) >> 1);
		Vector2 dispVec = Vector2(	bothBitsSet * 2 - 1,
									((i & 1 << 1) >> 1) * 2 - 1);
		Vector3 planePt =	((dispVec.X() * planeExtent) * tangent) +
							((dispVec.Y() * planeExtent) * biTangent) +
							origin;
		I32 ptIdx = 3*i;
		planePts[ptIdx] = planePt.X();
		planePts[ptIdx + 1] = planePt.Y();
		planePts[ptIdx + 2] = planePt.Z();
	}
	//now render the plane
	debugDraw(	NUM_PLANE_PTS, 0, planePts, NULL,
				worldMat, color, GL_LINE_LOOP);
}

void OGLGrpWrapper::DebugDrawBox(const Vector3& center, Quaternion rotation, F32 halfHeight, F32 halfWidth, F32 halfDepth, const Color& color)
{
	//Data's already built on initialization;
	//just pass it in.
	//World matrix is a simple SRT.
	Matrix4x4 newWorld =	Matrix4x4::BuildTranslation(center) *
							rotation.ToMatrix() *
							Matrix4x4::BuildScale(halfWidth * 2.0f, halfHeight * 2.0f, halfDepth * 2.0f);

	debugDraw(	sizeof(cubePos) / (3*sizeof(U32)),	sizeof(cubeIndex) / sizeof(U32), 
				cubePos,							cubeIndex,
				newWorld, color, GL_TRIANGLE_STRIP);
}

void OGLGrpWrapper::DebugDrawSphere(const Vector3& center, F32 radius, const Color& color)
{
	//Data's already built on initialization;
	//just pass it in.
	//World matrix is a ST - rotating a sphere doesn't really make sense.
	Matrix4x4 newWorld =	Matrix4x4::BuildTranslation(center) *
							Matrix4x4::BuildScale(radius, radius, radius) *
							worldMat;

	debugDraw(	numSphrPoints,	numSphrIndices, 
				spherePos,		sphereIndex,
				newWorld, color, GL_TRIANGLE_STRIP);
}

void OGLGrpWrapper::DebugDrawFrustum(const Matrix4x4& invProjMat, const Matrix4x4& viewMat, const Color& color)
{
	//The basic idea is to just take a projection matrix,
	//and invert it. That inverse matrix can then transform a cube into a
	//frustum, aspect ratio and all.
	//However, the inverted matrix is supposed to be used on a cube in NDC space -
	//a cube spanning [-1, 1] in all planes.
	//The current cube spans half that, so we need to scale by 2.

	//In DirectX, we'd also need to translate the cube and scale along Z,
	//as it expects a NDC cube to span [0, 1] along Z.

	//Since doing the inversion here would be bad (inverting on every draw call),
	//we'll rely on the caller to have inverted it for us.
	//Then the view matrix, if provided, can be used to transform the frustum into world coordinates.
	Matrix4x4 cubeTransformMat = (invProjMat).FindInverse();//.FindInverse();

	F32 frusPos[NUM_CUBE_POINTS*3];
	for(int i = 0; i < NUM_CUBE_POINTS; ++i)
	{
		int ptIdxStart = i*3;
		//make a vector from the current three points
		Vector3 pt = Vector3(cubePos[ptIdxStart], cubePos[ptIdxStart+1], cubePos[ptIdxStart+2]);
		//transform the point.
		//As the projection matrix distorts space,
		//it'll modify the W component of a point.
		//We thus need to divide by W to get points in non-perspective space.
		pt = Matrix4x4::BuildScale(2).MultiplyPoint(pt);
		pt = cubeTransformMat.MultiplyPerspPoint(pt);
		frusPos[ptIdxStart] = pt.X();
		frusPos[ptIdxStart+1] = pt.Y();
		frusPos[ptIdxStart+2] = pt.Z();
	}

	//Then multiply normally by the view matrix
	//to get the final world coordinates.
	Matrix4x4 newWorld = viewMat.FindInverse();

	//Rest is the same as drawing a normal box.
	debugDraw(	sizeof(cubePos) / (3*sizeof(U32)),	sizeof(cubeIndex) / sizeof(U32), 
				frusPos,							cubeIndex,
				newWorld, color, GL_TRIANGLE_STRIP);
}

void OGLGrpWrapper::DebugDrawFrustum(const CameraBase* cam, const Color& color)
{
	//Geometric implementation.
	//Generate the desired points
	//in the same order as the cube data,
	//then pass using the cube data's indices.
	const F32 halfNearHeight = Math::Tan(cam->FOV()/2) * cam->NearDist();
	const F32 halfNearWidth = halfNearHeight * cam->AspectRatio();
	const F32 halfFarHeight = Math::Tan(cam->FOV()/2) * cam->FarDist();
	const F32 halfFarWidth = halfFarHeight * cam->AspectRatio();
	const F32 nearD = cam->NearDist();
	const F32 farD = cam->FarDist();
	Vector3 origin = cam->Position();
	//remember the cube data's order - 
	//left lower back (LLB), RLB, LUB, RUB, LLF, RLF, LUF, RUF
	const I32 NUM_PTS = 8;
	I32 tmpI = 0;
	Vector3 pts[NUM_PTS];
	F32 vertData[NUM_PTS * 3];

	//back
	pts[tmpI++] =	origin + (-halfNearWidth*cam->Right()) +
					(-halfNearHeight*cam->Up()) +
					(nearD*cam->Forward());
	pts[tmpI++] =	origin + (halfNearWidth*cam->Right()) +
					(-halfNearHeight*cam->Up()) +
					(nearD*cam->Forward());
	pts[tmpI++] =	origin + (-halfNearWidth*cam->Right()) +
					(halfNearHeight*cam->Up()) +
					(nearD*cam->Forward());
	pts[tmpI++] =	origin + (halfNearWidth*cam->Right()) +
					(halfNearHeight*cam->Up()) +
					(nearD*cam->Forward());

	//front
	pts[tmpI++] =	origin + (-halfFarWidth*cam->Right()) +
					(-halfFarHeight*cam->Up()) +
					(farD*cam->Forward());
	pts[tmpI++] =	origin + (halfFarWidth*cam->Right()) +
					(-halfFarHeight*cam->Up()) +
					(farD*cam->Forward());
	pts[tmpI++] =	origin + (-halfFarWidth*cam->Right()) +
					(halfFarHeight*cam->Up()) +
					(farD*cam->Forward());
	pts[tmpI++] =	origin + (halfFarWidth*cam->Right()) +
					(halfFarHeight*cam->Up()) +
					(farD*cam->Forward());

	//now copy these over
	for(int i = 0; i < NUM_PTS; ++i)
	{
		int idx = 3*i;
		vertData[idx++] = pts[i].X();
		vertData[idx++] = pts[i].Y();
		vertData[idx++] = pts[i].Z();
	}

	//and render the data
	debugDraw(	NUM_PTS,	sizeof(cubeIndex) / sizeof(U32), 
				vertData,							cubeIndex,
				Matrix4x4::Identity, color, GL_TRIANGLE_STRIP);
}

void OGLGrpWrapper::DebugDrawFrustum(const Frustum& frust, const Color& color)
{
	F32 vertData[Frustum::NUM_POINTS * 3];

	for(int i = 0; i < Frustum::NUM_POINTS; ++i)
	{
		Vector3 pt = frust.GetBoundPoint(i);
		int idx = 3*i;
		vertData[idx++] = pt.X();
		vertData[idx++] = pt.Y();
		vertData[idx++] = pt.Z();
	}
	debugDraw(	Frustum::NUM_POINTS,	sizeof(cubeIndex) / sizeof(U32), 
				vertData,							cubeIndex,
				Matrix4x4::Identity, color, GL_TRIANGLE_STRIP);
}

void OGLGrpWrapper::DebugDrawMesh(const Geometry& meshGeom, const Vector3& center, Quaternion rotation, const Color& color)
{
}

void OGLGrpWrapper::DebugDrawModel(const Model& model, const Vector3& center, Quaternion rotation, const Color& color)
{
}

#pragma endregion

TypedHandle<Shader> OGLGrpWrapper::MakeShader(String shaderName, U32 shaderFileCount, Vector< std::pair< ShaderType,Path > > FilePaths)
{
	//doesn't bind attributes!
	if(shaderFileCount > 0)
	{
		//check that there isn't already a shader by the desired name
		if(shaderProgramList.find(shaderName) != shaderProgramList.end())
		{
			return 0;
		}
		//if there's any shaders in the to build list, temporarily swap them out
		Vector<GLuint> tempShaderStore(currentShaderList);
		currentShaderList.clear();
		//load shaders, then create program
		for(U32 i = 0; i < shaderFileCount; i++)
		{
			std::pair< ShaderType,Path > p = FilePaths[i];
			ShaderType shdType = p.first; //(ShaderType) va_arg(shaderList, int);
			LogV(String("ShaderType: ") + shdType);
			Path shdPath = p.second; //va_arg(shaderList, const char *); //XXX PROBABLY BROKEN CNL
			LogV(String("Shader Path: ") + shdPath.GetBaseName());
			GLenum shdGLType;
			switch(shdType)
			{
				case VERTEX:
				{
					shdGLType = GL_VERTEX_SHADER;
					break;
				}
				case FRAGMENT:
				{
					shdGLType = GL_FRAGMENT_SHADER;
					break;
				}
				case GEOMETRY:
				{
					shdGLType = GL_GEOMETRY_SHADER;
					break;
				}
				case TESS_CONTROL:
				{
					shdGLType = GL_TESS_CONTROL_SHADER;
					break;
				}
				case TESS_EVAL:
				{
					shdGLType = GL_TESS_EVALUATION_SHADER;
					break;
				}
				default:
				{
					return 0;
				}
			}
			LogV(String("Shader Type (for IGraphicsWrapper): ") + shdGLType);
			if(!LoadShader(shdGLType, shdPath))
			{
				return 0;
			}
		}
		return CreateShaderProgram(shaderName, 0);
	}
	return 0;
}

bool OGLGrpWrapper::SetShader(String shaderName)
{
	HashMap<TypedHandle<Shader>>::iterator it = shaderProgramList.find(shaderName);
	if(it != shaderProgramList.end())
	{
		//LogV(String("Assigning shader program ") + StrFromVal(it->second.ProgramHandle()));
		currentProgram = it->second;
		glUseProgram(currentProgram->ProgramHandle());
		assertNoErr();
		return true;
	}
	L_ASSERT(glIsProgram(currentProgram->ProgramHandle()) == GL_TRUE);
	L_ASSERT(false && "Tried to load unloaded shader!");
	return false;
}

bool OGLGrpWrapper::SetShader(TypedHandle<Shader> shader)
{
	//no need to enter a lookup table, just set the shader program.
	currentProgram = shader;
	L_ASSERT(glIsProgram(currentProgram->ProgramHandle()) == GL_TRUE);
	glUseProgram(currentProgram->ProgramHandle());
	assertNoErr();
	return true;
}
#pragma endregion

bool OGLGrpWrapper::LoadFunctions()
{
	int errVal;
	//first, try to pull core 4.3 functions
	errVal = ogl_LoadFunctions();
	//if the loader returns LOAD_FAILED, nothing was loaded
	if(errVal == ogl_LOAD_FAILED)
	{
		return false;
	}
	//otherwise, if it didn't return ogl_LOAD_SUCCEEDED, 
	//some functions weren't loaded
	else if(errVal != ogl_LOAD_SUCCEEDED)
	{
		//note how many core functions weren't loaded
		LogW(String("Failed to load ") + (errVal - ogl_LOAD_SUCCEEDED) + " OpenGL functions!");
		//should this be considered failure?
		//return false;
	}
	//otherwise, we're good
	LogD("Loaded OpenGL functions");
	return true;
}

#pragma region Shader Program Methods
bool OGLGrpWrapper::LoadShader(GLenum type, Path filePath)
{
	assertNoErr();
	//load code and attempt a compile; if it failed, log the failure
	//if it didn't, add it to the shader list with v.push_back()
	DataStream* shaderStream = Filesystem::OpenFile(filePath);
	if(!shaderStream)
	{
		//file not found
		LogE(String("Could not open shader file ") + filePath.GetBaseName() + "!");
		return false;
	}

	char* buffer = shaderStream->ReadAll();

	//make the shader handle the final program will need
	GLuint shaderHandle = glCreateShader(type);
	assertNoErr();
	if(!shaderHandle)
	{
		//somehow failed to get a handle
		LogE(String("Could not get an OpenGL handle for ") + filePath.GetBaseName() + "!");
		glDeleteShader(shaderHandle);
		assertNoErr();
		return false;
	}
	if(shaderHandle == GL_INVALID_ENUM)
	{

		LogE(String(filePath.GetBaseName()) + " was given an invalid type!");
		glDeleteShader(shaderHandle);
		assertNoErr();
		return false;
	}

	//load shader code and compile
	int length = strlen(buffer);
	glShaderSource(shaderHandle, 1, &buffer, &length);
	assertNoErr();
	glCompileShader(shaderHandle);
	assertNoErr();
	//check for errors
	GLint error;
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &error);
	assertNoErr();
	if(error == GL_FALSE)
	{
		//compile error, load log from OpenGL
		char errorLog[1024];
		glGetShaderInfoLog(shaderHandle, 1024, 0, errorLog);
		assertNoErr();
		LogE(String(filePath.GetBaseName()) + " failed to compile!\n");
		LogE("Error:");
		LogE(errorLog);
		glDeleteShader(shaderHandle);
		assertNoErr();
		return false;
	}

	//if it compiled, we're good, pass the shader handle
	LogV(String("Loaded shader ") + filePath.GetBaseName());
	currentShaderList.push_back(shaderHandle);
	LogV(String("Number of shaders: ") + currentShaderList.size());
	shaderStream->Close();
	return true;
}

TypedHandle<Shader> OGLGrpWrapper::CreateShaderProgram(String progName, U32 attribCount, va_list attribs)
{
	//will attempt to build a program, clearing currentShaderList in the process;
	//if build succeeds, assigns new progam as currentProgram,
	//otherwise does nothing.
	GLuint newProgramHnd = glCreateProgram();
	assertNoErr();

	//U32 count = 0;
	//attach shaders
	for(int i = currentShaderList.size() - 1; i >= 0; --i)//std::vector<GLuint>::reverse_iterator it = currentShaderList.rbegin(); it != currentShaderList.rend(); ++it)
	{
		glAttachShader(newProgramHnd, currentShaderList[i]);
		assertNoErr();
		//count++;
	}
	//std::cout << "Linking " << count << " shaders\n";

	//bind attributes
	int index;
	char* attribName;
	for(U32 i = 0; i < attribCount; i++)
	{
		index = va_arg(attribs, int);
		attribName = va_arg(attribs, char*);
		glBindAttribLocation(newProgramHnd, index, attribName);
		assertNoErr();
		LogV(String("Bound ") + attribName + " to location " + index);
	}

	//attempt to link
	glLinkProgram(newProgramHnd);
	assertNoErr();
	
	//remove shaders now
	for(int i = 0; i < currentShaderList.size(); ++i)//std::vector<GLuint>::iterator it = currentShaderList.begin(); it != currentShaderList.end(); ++it)
	{
		glDeleteShader(currentShaderList[i]);
		assertNoErr();
	}
	currentShaderList.clear();

	//if the link failed, quit
	GLint error;
	glGetProgramiv(newProgramHnd, GL_LINK_STATUS, &error);
	assertNoErr();
	if(error == GL_FALSE)
	{
		//load log from OpenGL
		char errorLog[1024];
		glGetProgramInfoLog(newProgramHnd, 1024, 0, errorLog);
		assertNoErr();
		LogE(String("Program ") + newProgramHnd + " failed to link!");
		LogE("Error:");
		LogE(errorLog);
		glDeleteProgram(newProgramHnd);
		assertNoErr();
		return 0;
	}
	//program must contain values for world, view, and projection
	//pull locations for them now
	GLuint tempWorldHnd, tempViewHnd, tempProjHnd;
	tempWorldHnd = glGetUniformLocation(newProgramHnd, SHADER_WORLD_MATRIX_NAME);
	assertNoErr();
	if(tempWorldHnd == -1)
	{
		LogE(String("Program ") + newProgramHnd + " is missing \"" + SHADER_WORLD_MATRIX_NAME + "\" variable!");
		return 0;
	}

	tempViewHnd = glGetUniformLocation(newProgramHnd, SHADER_VIEW_MATRIX_NAME);
	assertNoErr();
	if(tempViewHnd == -1)
	{
		LogE(String("Program ") + newProgramHnd + " is missing \"" + SHADER_VIEW_MATRIX_NAME + "\" variable!");
		return 0;
	}

	tempProjHnd = glGetUniformLocation(newProgramHnd, SHADER_PROJ_MATRIX_NAME);
	assertNoErr();
	if(tempProjHnd == -1)
	{
		LogE(String("Program ") + newProgramHnd + " is missing \"" + SHADER_PROJ_MATRIX_NAME + "\" variable!");
		return 0;
	}

	//parse uniforms
	HashMap<U32> uniforms;
	int uniformCount = 0;
	int varNameMaxLen = 0;
	glGetProgramiv(newProgramHnd, GL_ACTIVE_UNIFORMS, &uniformCount);
	assertNoErr();
	glGetProgramiv(newProgramHnd, GL_ACTIVE_UNIFORM_MAX_LENGTH, &varNameMaxLen);
	assertNoErr();
	//have to dynamically allocate this
	//since the max length isn't known at compile time
	char* uniformName = CustomArrayNew<char>(varNameMaxLen, 0, "UnfNameLoad");
	for(int i = 0; i < uniformCount; i++)
	{
		GLenum handleType;
		int uniSize;
		glGetActiveUniform(newProgramHnd, i, varNameMaxLen, 0, &uniSize, &handleType, uniformName);
		assertNoErr();
		int uniHandle = glGetUniformLocation(newProgramHnd, uniformName);
		assertNoErr();
		if(uniHandle != -1)
		{
			LogV(String("Found uniform ") + uniformName + ", handle " + uniHandle);
			uniforms[String(uniformName)] = uniHandle;
		}
	}
	CustomArrayDelete(uniformName);
	//if everything's valid so far, we can add the shader to the wrapper's shader list
	TypedHandle<Shader> result = HandleMgr::RegisterPtr(LNew(Shader, SHADER_ALLOC, "ShaderAlloc")
														(progName, newProgramHnd, uniforms));
	shaderProgramList[progName] = result;
	//if the current shader's invalid, set the one we made as current shader
	LogD(String("Created shader program ") + shaderProgramList[progName]->ProgramHandle());
	if(!currentProgram)// currentProgram->ProgramHandle() == NULL)
	{
		SetShader(progName);
	}
	return result;
}

TypedHandle<Shader> OGLGrpWrapper::CreateShaderProgram(String progName, U32 attribCount, ...)
{
	//handle the case that the varargs will be of type va_list
	va_list attribs;
	va_start(attribs, attribCount);
	TypedHandle<Shader> result = CreateShaderProgram(progName, attribCount, attribs);
	va_end(attribs);
	return result;
}

void OGLGrpWrapper::ShutdownShaderProgram()
{
	//presumably this won't be called during the draw
	//do any necessary pre-close work on the program, and then null our handle to it
	//currentProgram = NULL;
}

bool OGLGrpWrapper::ProgramFromShaderPair(String progName, Path vertexShaderSource, Path fragShaderSource, U32 attribCount, ...)
{
	if(!LoadShader(GL_VERTEX_SHADER, vertexShaderSource))
	{
		//log failure
		return false;
	}

	if(!LoadShader(GL_FRAGMENT_SHADER, fragShaderSource))
	{
		//log failure
		return false;
	}

	//now we need to pull the attrib parameters
	va_list attribs;
	va_start(attribs, attribCount);
	bool result = CreateShaderProgram(progName, attribCount, attribs);
	va_end(attribs);
	if(!result)
	{
		//log failure
		return false;
	}
	return true;
}

void OGLGrpWrapper::PrintShaderStatus()
{
	if(currentProgram)
	{
		int infoLogLen = 0;
		int charsWritten = 0;
		char* log;
		glGetShaderiv(currentProgram->ProgramHandle(), GL_INFO_LOG_LENGTH, &infoLogLen);
		assertNoErr();
		if(infoLogLen > 0)
		{
			log = CustomArrayNew<char>(infoLogLen, 0, "ShaderLogAlloc");//new char[infoLogLen];
			if(log == 0)
			{
				LogE("Out of memory! Couldn't print shader info log!");
				return;
			}
			glGetShaderInfoLog(currentProgram->ProgramHandle(), infoLogLen, &charsWritten, log);
			assertNoErr();
			LogD("From shader info log:");
			LogD(log);
			CustomDelete(log);
		}
	}
}
#pragma endregion

#pragma region Shader Parameter Setters
bool OGLGrpWrapper::SetMatrixUniform(String name, const Matrix4x4& value)
{
	PROFILE("SetMatrixVar");
	//only works if there's a shader assigned
	if(!currentProgram || currentProgram->ProgramHandle() == 0)
	{
		return false;
	}
	L_ASSERT(glIsProgram(currentProgram->ProgramHandle()) == GL_TRUE);
	assertNoErr();
	glUseProgram(currentProgram->ProgramHandle());
	assertNoErr();
	GLint shaderProg = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProg);
	L_ASSERT(shaderProg != 0);
	L_ASSERT(16*sizeof(F32) == 16*sizeof(GLfloat));
	GLint unfGLLoc = glGetUniformLocation(currentProgram->ProgramHandle(), name.c_str());
	assertNoErr();
	GLint unfLoc = currentProgram->GetUniformHandle(name);
	L_ASSERT(unfGLLoc == unfLoc);
	glUniformMatrix4fv(	unfLoc,	//copy to uniform's location
						1,										//passing 1 matrix
						false,									//don't transpose; can't be any other value anyway??
						value.ToFloatArray());					//matrix data
	//L_ASSERT(glActiveShaderProgram
	assertNoErr();
	return true;
}

bool OGLGrpWrapper::SetVec3Uniform(String name, const Vector3& value)
{
	PROFILE("SetVec3Var");
	//only works if there's a shader assigned
	if(!currentProgram || currentProgram->ProgramHandle() == 0)
	{
		return false;
	}

	glUniform3f(	currentProgram->GetUniformHandle(name),	//copy to uniform's location
					value.X(),
					value.Y(),
					value.Z());

	GLenum err = glGetError();
	L_ASSERT(err == GL_NO_ERROR);
	return true;
}

bool OGLGrpWrapper::SetVec4Uniform(String name, const Vector4& value)
{
	PROFILE("SetVec4Var");
	//only works if there's a shader assigned
	if(!currentProgram || currentProgram->ProgramHandle() == 0)
	{
		return false;
	}

	glUniform4f(	currentProgram->GetUniformHandle(name),	//copy to uniform's location
					value.X(),
					value.Y(),
					value.Z(),
					value.W());

	GLenum err = glGetError();
	L_ASSERT(err == GL_NO_ERROR);
	return true;
}

bool OGLGrpWrapper::SetIntUniform(String name, const U32& value)
{
	PROFILE("SetIntVar");
	//only works if there's a shader assigned
	if(!currentProgram || currentProgram->ProgramHandle() == 0)
	{
		return false;
	}
	GLboolean isProg = glIsProgram(currentProgram->ProgramHandle());
	L_ASSERT(isProg == GL_TRUE);
	glUseProgram(currentProgram->ProgramHandle());
	assertNoErr();
	glUniform1i(	currentProgram->GetUniformHandle(name),	//copy to uniform's location
					value);

	assertNoErr();
	return true;
}

bool OGLGrpWrapper::SetFloatUniform(String name, const F32& value)
{
	PROFILE("SetVec4Var");
	//only works if there's a shader assigned
	if(!currentProgram || currentProgram->ProgramHandle() == 0)
	{
		return false;
	}

	glUniform1f(	currentProgram->GetUniformHandle(name),	//copy to uniform's location
					value);

	GLenum err = glGetError();
	L_ASSERT(err == GL_NO_ERROR);
	return true;
}

bool OGLGrpWrapper::SetTexture(const Texture2D& tex, TextureMeta::MapType type)
{
	return SetTexture(tex.TextureBufferHandle, type);
}

bool OGLGrpWrapper::SetTexture(U32 texHandle, TextureMeta::MapType type)
{
	if(!texHandle)
	{
		return false;
	}
	glActiveTexture(GL_TEXTURE0 + type);
	glBindTexture(GL_TEXTURE_2D, texHandle);
	//should really test that this all worked
	GLenum err = glGetError();
	L_ASSERT(err == GL_NO_ERROR);
	return true;
}

bool OGLGrpWrapper::SetWorld(const Matrix4x4& world)
{
	PROFILE("SetWVPMatrix");
	worldMat = world;
	return SetMatrixUniform("worldMat", world);
}

bool OGLGrpWrapper::SetView(const Matrix4x4& view)
{
	PROFILE("SetWVPMatrix");
	viewMat = view;
	return SetMatrixUniform("viewMat", view);
}

bool OGLGrpWrapper::SetProjection(const Matrix4x4& projection)
{
	PROFILE("SetWVPMatrix");
	projectionMat = projection;
	return SetMatrixUniform("projectionMat", projection);
}
#pragma endregion

#pragma region Texture Methods
Texture2D OGLGrpWrapper::GenerateBlankTexture(	U32 width, U32 height, 
												Texture2D::PixelType pixType,
												Texture2D::CompressionType compType)
{
	Texture2D result;
	//raise to nearest power of two
	result.Width = Math::NearestPowOf2(width);
	result.Height = Math::NearestPowOf2(height);
	result.PixType = pixType;
	result.CompType = compType;

	//generate texture handle
	glActiveTexture(GL_TEXTURE0);
	GLenum err = glGetError();
	//L_ASSERT(err == GL_NO_ERROR);
	U32 texBufHnd = 0;
	glGenTextures(1, &texBufHnd);
	err = glGetError();
	if(err != GL_NO_ERROR)
	{
		return Texture2D();
	}
	result.TextureBufferHandle = texBufHnd;
	//and setup the texture info
	glBindTexture(GL_TEXTURE_2D, result.TextureBufferHandle);
	err = glGetError();
	if(err != GL_NO_ERROR)
	{
		return Texture2D();
	}

	//load the texture data
	//pass null data to indicate this will be filled later
	glTexImage2D(	GL_TEXTURE_2D,	//2D texture
					0,				//base level of mipmap
					texOutputTypes[result.PixType], //way the pixels should be stored in VRAM
					result.Width,
					result.Height,
					0,				//border never worked!
					texInputTypes[result.PixType],	//way the pixels are stored in the res manager
					texDataWidths[result.PixType],
					0);
	err = glGetError();
	if(err != GL_NO_ERROR)
	{
		return Texture2D();
	}

	//setup tex parameters.
	//TODO: make customizable?
	//Set edge behavior - by default, clamp.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if(err != GL_NO_ERROR)
	{
		LogE(StrFromVal(err));
	}
	//mipmap levels...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	//and filtering - unlike with normal textures,
	//we don't have data yet, so we can't generate mipmaps.
	//Set as bilinear.
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	if(err != GL_NO_ERROR)
	{
		LogE(StrFromVal(err));
	}

	//

	//texture is now loaded - in theory, you can release the data now
	//unbind texture for next texture to come
	glBindTexture(GL_TEXTURE_2D, 0);
	return result;
}

bool OGLGrpWrapper::FillTextureSection(const Texture2D& tex, U32 x, U32 y, U32 width, U32 height, char* data)
{
	L_ASSERT(tex.PixType != Texture2D::PIXTYPE_LEN && "Invalid texture object!");
	bool result = false;
	if(tex.TextureBufferHandle == 0 || tex.PixType == Texture2D::PIXTYPE_LEN)
	{
		return result;
	}

	//save the currently bound texture
	I32 currTex = 0;
	glGetIntegerv(GL_TEXTURE_2D, &currTex);
	
	//bind the texture to be written
	glBindTexture(GL_TEXTURE_2D, tex.TextureBufferHandle);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	//write the data
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, texInputTypes[tex.PixType], texDataWidths[tex.PixType], data);
	if(glGetError() == GL_NO_ERROR)
	{
		result = true;
	}
	else
	{
		result = false;
	}

	//rebind the previous texture
	glBindTexture(GL_TEXTURE_2D, currTex);
	return result;
}
#pragma endregion

#pragma endregion
