#ifndef H_MESHRENDERERENGINE
#define H_MESHRENDERERENGINE

#include "ogl_inc.h"
#include <engine.h>

#include <vector>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Shader.hpp>
#include "vertex.h"
#include "rendertarget.h"


class MeshRendererEngine : public Engine
{
public:
	MeshRendererEngine();
	virtual ~MeshRendererEngine();
	virtual bool Init();
	virtual void Update(float deltaTime);
	virtual void Draw();
private:

	void LoadMesh();
	void LoadMeshTexture();
	void LoadShader();
	void LoadOutlineShader();
	void LoadBasicShader();
	void LoadNormalsShader();

	void LoadPostProcShader(sf::Shader &shader);
	void LoadSobelShader();
	void LoadPostProcShaders();

	void InitShaderDraw();
	void DrawMeshImmediate();
	void DrawMeshVA();
	void DrawNormalsVA();

	void DoDraw2d();
	void Draw2d();

	void DoPostProc();
	void MakeTargetCurrent(RenderTarget *rt, RenderTarget &prev);
	void ApplyPostProc(sf::Shader &shader);
	void PostProc();

	void SetupOrthoMatrix();
	void UndoSetupOrthoMatrix();

	float m_rot;

	sf::Texture m_texture;
	std::vector<Vertex> m_vertices;
	std::vector<Vector3f> m_vertexNormals;

	sf::Shader m_wireframeShader;
	std::vector<Vector3f> m_vertexAttributes;
	GLint m_attribIndex;
	
	sf::Shader m_outlineShader;

	Vector3f m_eyePos;
	Vector3f m_eyeDir;
	Vector3f m_lookAt;

	sf::Shader m_sobelShader;
	sf::Shader m_dilateShader;
	sf::Shader m_erodeShader;
	sf::Shader m_boxBlurShader;
	RenderTarget m_ppRT;
	RenderTarget m_pp1RT;
	RenderTarget m_pp2RT;

	sf::Shader m_basicShader;

	sf::Shader m_normalsShader;
};


#endif
