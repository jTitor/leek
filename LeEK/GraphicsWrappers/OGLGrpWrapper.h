#ifndef OGLGRPWRAPPER_H
#define OGLGRPWRAPPER_H
#pragma once
#include "GraphicsWrappers/IGraphicsWrapper.h"
#include "Strings/String.h"
#include "Math/Matrix4x4.h"
#include "FileManagement/Filesystem.h"
#include "Hashing/HashMap.h"
#include "Rendering/Shader.h"
#include <vector>
#ifdef WIN32
#include <Libraries/GL_Loaders/GL/gl_core_4_3.h>
#else
#include <Libraries/GL_Loaders/GLX/glx_core_4_3.h>
#endif

namespace LeEK
{
	/**
	* OpenGL (and currently only) implementation of the IGraphicsWrapper class.
	*/
	class OGLGrpWrapper : public IGraphicsWrapper
	{
	private:
		Matrix4x4 worldMat, viewMat, projectionMat;
		String vendorString, rendererString;
		HashMap<TypedHandle<Shader>> shaderProgramList;
		Vector<GLuint> currentShaderList;
		TypedHandle<Shader> currentProgram;
		//debug drawing members
		Shader* debugProgram;
		U32 debugVAOHnd;
		U32 debugVertArrayHnd;
		U32 debugIndexHnd;
		bool contextSet;
		//screen properties
		Vector2 screenRes;
		//F32 fieldOfView, screenAspect;
		F32 screenAspect;
		bool enableVSync;
		void* context;

		static const int debugVertArraySize = 256;
		//mesh data
		static const F32 cubePos[24];
		static const U32 cubeIndex[14];
		static char* dbgShdrName;

		void _assertNoErr(const char* file, int line);
		bool buildDbgSphere();
		bool buildDebugMeshes();
		bool loadDebugShader();
		bool loadDebugFunctions();
		
		bool isContextSet();
		/**
		Determines if the given GLenum is an enum for
		geometry type, such as GL_TRIANGLE or GL_LINE_LOOP.
		*/
		inline bool isGeomEnum(GLenum enumVal) const;
		/**
		Determines if the given GLenum is an enum for
		geometry types using line primitives,
		such as GL_LINE_LOOP.
		*/
		inline bool isLineGeomEnum(GLenum enumVal) const;
		/**
		Draws the given raw geometry to the screen in wireframe.
		@param meshType the GLenum type that the vertex data represents.
		Can be GL_LINES_* or GL_TRIANGLES_*.
		*/
		void debugDraw(	size_t numVerts,
						size_t numInds,
						const F32* vertData,
						const U32* indData,
						const Matrix4x4& pWorldMat,
						const Color& color,
						GLenum meshType);
	public:
		const static int MIN_MAJOR_VER = 3;
		const static int MIN_MINOR_VER = 0;

		OGLGrpWrapper(void);
		~OGLGrpWrapper(void);

		#pragma region Properties
		void SetScreenResolution(const Vector2& val);

		inline bool VSyncEnabled() const { return enableVSync; }
		inline void SetVSyncEnabled(bool val) { enableVSync = val; }
		inline const HashMap<TypedHandle<Shader>>& GetShaderList() const { return shaderProgramList; }
		const void* Context() const;
		void SetContext(void* newCtx);
		#pragma endregion

		#pragma region Interface Implementation
		inline const RendererType Type() const { return OPEN_GL; }
		F32 ScreenAspect()  const { return screenAspect; }
		const Vector2& GetScreenResolution() const { return screenRes; }

		void PrintConsole() {}
		bool Startup();
		bool InitGeometry(Geometry& mesh);
		bool InitText(Text& text);
		bool InitTexture(Texture2D& tex, TextureMeta::MapType type);
		void Shutdown();
		void ShutdownMesh(const Geometry& mesh);
		void ShutdownText(const Text& text);
		void ShutdownTexture(Texture2D& tex);
		void Clear(Color c);
		inline void Clear() { Clear(Colors::Black); }
		void Draw(const Geometry& mesh);
		void Draw(Text& text);
		#pragma region Debug Drawing Commands
		void DebugDrawLine(const Vector3& start, const Vector3& end, const Color& color);
		void DebugDrawPlane(const Vector3& origin, const Vector3& normal, const Color& color);
		void DebugDrawBox(const Vector3& center, Quaternion rotation, F32 halfHeight, F32 halfWidth, F32 halfDepth, const Color& color);
		void DebugDrawSphere(const Vector3& center, F32 radius, const Color& color);
		void DebugDrawFrustum(const Matrix4x4& invProjMat, const Matrix4x4& viewMat, const Color& color);
		void DebugDrawFrustum(const CameraBase* cam, const Color& color);
		void DebugDrawFrustum(const Frustum& frust, const Color& color);
		void DebugDrawMesh(const Geometry& meshGeom, const Vector3& center, Quaternion rotation, const Color& color);
		void DebugDrawModel(const Model& model, const Vector3& center, Quaternion rotation, const Color& color);
		#pragma endregion
		TypedHandle<Shader> MakeShader(String shaderName, U32 shaderFileCount, Vector< std::pair< ShaderType,Path > > FilePaths);
		bool SetShader(String shaderName);
		bool SetShader(TypedHandle<Shader> shader);
		#pragma endregion

		bool LoadFunctions();
		bool LoadShader(GLenum type, Path filePath);
		//varargs should consist first of the number of attributes, followed pairs of attribute positions and attribute names in char* form
		TypedHandle<Shader> CreateShaderProgram(String progName, U32 attribCount, ...);
		TypedHandle<Shader> CreateShaderProgram(String progName, U32 attribCount, va_list attribs);
		void ShutdownShaderProgram();
		//helper function
		//varargs should consist first of the number of attributes, followed pairs of attribute positions and attribute names in char* form
		bool ProgramFromShaderPair(String progName, Path vertexShaderSource, Path fragShaderSource, U32 attribCount, ...);
		void PrintShaderStatus();

		bool SetMatrixUniform(String name, const Matrix4x4& value);
		bool SetVec3Uniform(String name, const Vector3& value);
		bool SetVec4Uniform(String name, const Vector4& value);
		bool SetIntUniform(String name, const U32& value);
		bool SetFloatUniform(String name, const F32& value);
		bool SetTexture(const Texture2D& tex, TextureMeta::MapType type);
		bool SetTexture(U32 texHandle, TextureMeta::MapType type);

		bool SetWorld(const Matrix4x4& world);
		bool SetView(const Matrix4x4& view);
		bool SetProjection(const Matrix4x4& projection);

		Texture2D GenerateBlankTexture(	U32 width, U32 height, 
												Texture2D::PixelType pixType,
												Texture2D::CompressionType compType);
		bool FillTextureSection(const Texture2D& tex, U32 x, U32 y, U32 width, U32 height, char* data);
	};
}
#endif //OGLGRPWRAPPER_H
