#include "meshrendererengine.h"
#include <oglutils.h>
#include <cmath>
#include <fstream>
#include <string>
#include <iostream>

#define ENGINE_ROOT "C:\\Users\\Michael\\Documents\\Visual Studio 2012\\Projects\\OGLDesktop\\MeshRendererEngine\\"
#define MAKE_ENGINE_PATH(file) ENGINE_ROOT file


MeshRendererEngine::MeshRendererEngine()
	: m_rot(0)
{
}

MeshRendererEngine::~MeshRendererEngine()
{
}

bool MeshRendererEngine::Init()
{
	glewExperimental = GL_TRUE;
	GLenum glewErr = glewInit();
	bool glewOk = glewErr == GLEW_OK;

	oglutil::InitOpenGLBase();
	//glClearColor(0.0f, 0.25f, 0.5f, 1.0f);
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glViewport(0, 0, GetWidth(), GetHeight());
	glMatrixMode(GL_PROJECTION);
	gluPerspective(90, (double)GetWidth() / (double)GetHeight(), 1, 100);

	glMatrixMode(GL_MODELVIEW);

	LoadMesh();
	LoadMeshTexture();
	LoadShader();
	InitShaderDraw();

	m_eyePos = Vector3f(2.0f, 0.0f, 2.0f);
	m_lookAt = Vector3f(0.0f, 0.0f, 0.0f);
	m_eyeDir = (m_lookAt - m_eyePos).Normalized();

	LoadOutlineShader();

	m_ppRT.Init(GetWidth(), GetHeight());
	m_pp1RT.Init(GetWidth(), GetHeight());
	m_pp2RT.Init(GetWidth(), GetHeight());
	LoadSobelShader();
	LoadPostProcShaders();

	LoadBasicShader();
	LoadNormalsShader();

	return true;
}

void MeshRendererEngine::Update(float deltaTime)
{
	m_rot += 45.0f * deltaTime;
	m_rot = std::fmod(m_rot, 360.0f);
}

void MeshRendererEngine::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	//gluLookAt(2.0, 2.0, 0.75, 0, 0, 0, 0, 0, 1);
	gluLookAt(m_eyePos.GetX(), m_eyePos.GetY(), m_eyePos.GetZ(),
		m_lookAt.GetX(), m_lookAt.GetY(), m_lookAt.GetZ(),
		0, 1, 0);

	glRotated(m_rot, 0, 1, 0);
	//glScaled(1.5f, 1.5f, 1.5f);
	//oglutil::DrawColorfulCube();
	//glRotated(90.0f, 1, 0, 0);
	//InitShaderDraw();
	//sf::Shader::bind(&m_wireframeShader);
	//sf::Shader::bind(&m_outlineShader);

	//sf::Shader::bind(&m_basicShader);
	//sf::Shader::bind(&m_wireframeShader);
	sf::Shader::bind(&m_normalsShader);
	DrawMeshVA();
	sf::Shader::bind(NULL);
	DrawNormalsVA();

#if 0
	//RenderTarget::MakeCurrent(&m_edgeRT);
	RenderTarget::MakeCurrent(&m_ppRT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawMeshVA();
	RenderTarget::MakeCurrent(NULL);
	
	//DoDraw2d();
	DoPostProc();
#endif
}


enum MeshLoaderState
{
	VX, VY, VZ,
	NX, NY, NZ,
	TX, TY, T0,
	LAST
};

void RewriteVector3f(Vector3f &vec, int index, float value)
{
	switch(index)
	{
		case 0:
		{
			vec = Vector3f(value, vec.GetY(), vec.GetZ());
			break;
		}
		case 1:
		{
			vec = Vector3f(vec.GetX(), value, vec.GetZ());
			break;
		}
		case 2:
		{
			vec = Vector3f(vec.GetX(), vec.GetY(), value);
			break;
		}
	}
}

void RewriteVector2f(Vector2f &vec, int index, float value)
{
	switch(index)
	{
		case 0:
		{
			vec = Vector2f(value, vec.GetY());
			break;
		}
		case 1:
		{
			vec = Vector2f(vec.GetX(), value);
			break;
		}
	}
}

void WriteToVertex(MeshLoaderState state, Vertex &vertex, float value)
{
	switch (state)
	{
		case VX:
		{
			RewriteVector3f(vertex.m_Position, 0, value);
			break;
		}
		case VY:
		{
			RewriteVector3f(vertex.m_Position, 1, value);
			break;
		}
		case VZ:
		{
			RewriteVector3f(vertex.m_Position, 2, value);
			break;
		}
		case NX:
		{
			RewriteVector3f(vertex.m_Normal, 0, value);
			break;
		}
		case NY:
		{
			RewriteVector3f(vertex.m_Normal, 1, value);
			break;
		}
		case NZ:
		{
			RewriteVector3f(vertex.m_Normal, 2, value);
			break;
		}
		case TX:
		{
			RewriteVector2f(vertex.m_texCoord, 0, value);
			break;
		}
		case TY:
		{
			RewriteVector2f(vertex.m_texCoord, 1, value);
			break;
		}
		case T0:
		case LAST:
		default:
		{
			break;
		}
	}
}

#define MESH_DIR "C:\\Users\\Michael\\prog\\projects\\meshRenderer\\"
#define MAKE_MESH_PATH(file) MESH_DIR file

#define MESH_PATH MAKE_MESH_PATH("flowerPot.mesh")
#define MESH_TEXTURE_PATH MAKE_MESH_PATH("flowerPotTex.png")
//#define MESH_PATH MAKE_MESH_PATH("teddyBear.mesh")
//#define MESH_TEXTURE_PATH MAKE_MESH_PATH("teddyBearTex.png")
//#define MESH_PATH MAKE_MESH_PATH("rubberDuckie.mesh")
//#define MESH_TEXTURE_PATH MAKE_MESH_PATH("rubberDuckieTex.png")

void MeshRendererEngine::LoadMesh()
{
	std::fstream file(MESH_PATH, std::ios::in);

	std::string header;
	std::getline(file, header);

	std::string faceCountStr;
	std::getline(file, faceCountStr);
	int faceCount = std::stoi(faceCountStr);

	int structSize = sizeof(Vertex);
	int v3Size = sizeof(Vector3f);
	int v2Size = sizeof(Vector2f);
	m_vertices.reserve(faceCount * sizeof(Vertex) * 3);

	MeshLoaderState state = MeshLoaderState::VX;
	int tupleCount = 0;
	std::string buffer;
	Vertex vertex;
	char c = file.get();
	while(file.good())
	{
		if(c != '[' && c != ']' && c != ',')
		{
			buffer.push_back(c);
		}

		if(c == ']' || c == ',')
		{
			//std::cout << buffer << std::endl;
			WriteToVertex(state, vertex, std::stof(buffer));
			buffer.clear();
			state = (MeshLoaderState)((state + 1) % MeshLoaderState::LAST);
		}

		if(c == ']')
		{
			tupleCount++;
			if(tupleCount == 3)
			{
				tupleCount = 0;
				m_vertices.push_back(vertex);
			}
		}

		c = file.get();
	}

	//auto diff = ((unsigned char *)&m_vertices[1].m_Position) - ((unsigned char *)&m_vertices[0].m_Position);

	m_vertexAttributes.resize(m_vertices.size());
	for (int i = 0; i < m_vertices.size() / 3; i++)
	{
		m_vertexAttributes[i * 3 + 0] = Vector3f(1, 0, 0);
		m_vertexAttributes[i * 3 + 1] = Vector3f(0, 1, 0);
		m_vertexAttributes[i * 3 + 2] = Vector3f(0, 0, 1);
	}

	m_vertexNormals.resize(m_vertices.size() * 2);
	for (int i = 0; i < m_vertices.size(); i++)
	{
		m_vertexNormals[i * 2 + 0] = m_vertices[i].m_Position;
		m_vertexNormals[i * 2 + 1] = m_vertices[i].m_Position + m_vertices[i].m_Normal * 0.5f;
	}
}

void MeshRendererEngine::LoadMeshTexture()
{
	sf::Image textureImage;
	textureImage.loadFromFile(MESH_TEXTURE_PATH);
	textureImage.flipVertically();
	m_texture.loadFromImage(textureImage);
	m_texture.setSmooth(true);
}

#undef MESH_PATH
#undef MESH_TEXTURE_PATH
#undef MAKE_PATH
#undef MESH_DIR

void MeshRendererEngine::LoadShader()
{
	m_wireframeShader.loadFromFile(MAKE_ENGINE_PATH("wireframe.vert"), MAKE_ENGINE_PATH("wireframe.frag"));

	sf::Shader::bind(&m_wireframeShader);
	GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
	m_attribIndex = glGetAttribLocationARB(program, "barycentric");
	glUniform1iARB(glGetUniformLocationARB(program, "currentTexture"), 0);
	glUniform1fARB(glGetUniformLocationARB(program, "lineWidth"), 0.5f);
	glUniform3fARB(glGetUniformLocationARB(program, "lineColor"), 0.0f, 1.0f, 0.0f);
	sf::Shader::bind(NULL);
}

void MeshRendererEngine::LoadOutlineShader()
{
	m_outlineShader.loadFromFile(MAKE_ENGINE_PATH("outline.vert"), MAKE_ENGINE_PATH("outline.frag"));
	sf::Shader::bind(&m_outlineShader);
	GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
	glUniform1iARB(glGetUniformLocationARB(program, "maskTex"), 0);
	glUniform1iARB(glGetUniformLocationARB(program, "outlineTex"), 1);
	sf::Shader::bind(NULL);
}

void MeshRendererEngine::LoadBasicShader()
{
	m_basicShader.loadFromFile(MAKE_ENGINE_PATH("basic.vert"), MAKE_ENGINE_PATH("basic.frag"));

	sf::Shader::bind(&m_basicShader);
	GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
	glUniform1iARB(glGetUniformLocationARB(program, "currentTexture"), 0);
	sf::Shader::bind(NULL);
}

void MeshRendererEngine::LoadNormalsShader()
{
	m_normalsShader.loadFromFile(MAKE_ENGINE_PATH("normals.vert"), MAKE_ENGINE_PATH("normals.frag"));
	sf::Shader::bind(&m_normalsShader);
	GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
	//glUniform3fARB(glGetUniformLocationARB(program, "eyeDir"), VECTOR3_UNPACK(m_eyeDir));
	glUniform3fARB(glGetUniformLocationARB(program, "eyePos"), VECTOR3_UNPACK(m_eyePos));
	//glUniform3fARB(glGetUniformLocationARB(program, "eyePos"), 0, -8, 0);
	sf::Shader::bind(NULL);
}

void MeshRendererEngine::LoadPostProcShader(sf::Shader &shader)
{
	sf::Shader::bind(&shader);
	GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
	glUniform1iARB(glGetUniformLocationARB(program, "screenTexture"), 1);
	glUniform1iARB(glGetUniformLocationARB(program, "texWidth"), GetWidth());
	glUniform1iARB(glGetUniformLocationARB(program, "texHeight"), GetHeight());
	sf::Shader::bind(NULL);
}

void MeshRendererEngine::LoadSobelShader()
{
	m_sobelShader.loadFromFile(MAKE_ENGINE_PATH("postproc/base.vert"), MAKE_ENGINE_PATH("postproc/sobel.frag"));
	LoadPostProcShader(m_sobelShader);
}

void MeshRendererEngine::LoadPostProcShaders()
{
	m_dilateShader.loadFromFile(MAKE_ENGINE_PATH("postproc/base.vert"), MAKE_ENGINE_PATH("postproc/dilate.frag"));
	m_erodeShader.loadFromFile(MAKE_ENGINE_PATH("postproc/base.vert"), MAKE_ENGINE_PATH("postproc/erode.frag"));
	m_boxBlurShader.loadFromFile(MAKE_ENGINE_PATH("postproc/base.vert"), MAKE_ENGINE_PATH("postproc/boxblur.frag"));

	LoadPostProcShader(m_dilateShader);
	LoadPostProcShader(m_erodeShader);
	LoadPostProcShader(m_boxBlurShader);
}

void MeshRendererEngine::InitShaderDraw()
{
	glEnableVertexAttribArrayARB(m_attribIndex);
	glVertexAttribPointerARB(m_attribIndex, 3, GL_FLOAT, GL_FALSE, 12, &m_vertexAttributes[0]);
}

void MeshRendererEngine::DrawMeshImmediate()
{
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < m_vertices.size(); i++)
	{
		auto &vertex = m_vertices[i];

		glColor3f(std::abs(vertex.m_Normal.GetX()),
			std::abs(vertex.m_Normal.GetY()),
			std::abs(vertex.m_Normal.GetZ())
		);

		glNormal3f(vertex.m_Normal.GetX(),
			vertex.m_Normal.GetY(),
			vertex.m_Normal.GetZ()
		);

		glTexCoord2f(vertex.m_texCoord.GetX(),
			vertex.m_texCoord.GetY()
		);

		glVertex3f(vertex.m_Position.GetX(),
			vertex.m_Position.GetY(),
			vertex.m_Position.GetZ()
		);
	}
	glEnd();
}

void MeshRendererEngine::DrawMeshVA()
{
	//glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	//glColorPointer(3, GL_FLOAT, 32, &m_vertices[0].m_Normal);
	glVertexPointer(3, GL_FLOAT, 32, &m_vertices[0].m_Position);
	glTexCoordPointer(2, GL_FLOAT, 32, &m_vertices[0].m_texCoord);
	glNormalPointer(GL_FLOAT, 32, &m_vertices[0].m_Normal);

	//glEnable(GL_TEXTURE_2D);
	sf::Texture::bind(&m_texture);
	glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

void MeshRendererEngine::DrawNormalsVA()
{
	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, 12, &m_vertexNormals[0]);

	glColor3ub(0, 255, 255);
	glDrawArrays(GL_LINES, 0, m_vertices.size());
	glColor3ub(255, 255, 255);

	glDisableClientState(GL_VERTEX_ARRAY);
}

void MeshRendererEngine::DoDraw2d()
{
	SetupOrthoMatrix();
	Draw2d();
	UndoSetupOrthoMatrix();
}

void MeshRendererEngine::Draw2d()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	//oglutil::DrawColorfulQuad();
}

void MeshRendererEngine::DoPostProc()
{
	SetupOrthoMatrix();
	PostProc();
	UndoSetupOrthoMatrix();
}

void MeshRendererEngine::MakeTargetCurrent(RenderTarget *rt, RenderTarget &prev)
{
	RenderTarget::MakeCurrent(rt);
	//glClear(GL_DEPTH_BUFFER_BIT);
	glActiveTextureARB(GL_TEXTURE0_ARB + 1);
	glBindTexture(GL_TEXTURE_2D, prev.GetTexture());
}

void MeshRendererEngine::ApplyPostProc(sf::Shader &shader)
{
	sf::Shader::bind(&shader);
	oglutil::DrawTexturedQuad();
	sf::Shader::bind(NULL);
}

void MeshRendererEngine::PostProc()
{
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glLoadIdentity();
	glScalef(GetWidth(), GetHeight(), 0.0f);

	glBindTexture(GL_TEXTURE_2D, 0);

#if 0
	RenderTarget::MakeCurrent(&m_pp1RT);
	glClear(GL_DEPTH_BUFFER_BIT);
	glActiveTextureARB(GL_TEXTURE0_ARB + 1);
	glBindTexture(GL_TEXTURE_2D, m_ppRT.GetTexture());

	sf::Shader::bind(&m_sobelShader);
	oglutil::DrawTexturedQuad();
	sf::Shader::bind(NULL);

	RenderTarget::MakeCurrent(NULL);
	glClear(GL_DEPTH_BUFFER_BIT);
	glActiveTextureARB(GL_TEXTURE0_ARB + 1);
	glBindTexture(GL_TEXTURE_2D, m_pp1RT.GetTexture());

	sf::Shader::bind(&m_boxBlurShader);
	oglutil::DrawTexturedQuad();
	sf::Shader::bind(NULL);
#endif

	MakeTargetCurrent(&m_pp1RT, m_ppRT);
	ApplyPostProc(m_sobelShader);
	MakeTargetCurrent(&m_pp2RT, m_pp1RT);
	ApplyPostProc(m_dilateShader);
	MakeTargetCurrent(&m_pp1RT, m_pp2RT);
	ApplyPostProc(m_dilateShader);
	//MakeTargetCurrent(&m_pp2RT, m_pp1RT);
	//ApplyPostProc(m_dilateShader);
#if 1
	MakeTargetCurrent(&m_pp2RT, m_pp1RT);
	ApplyPostProc(m_boxBlurShader);
	MakeTargetCurrent(&m_pp1RT, m_pp2RT);
	ApplyPostProc(m_boxBlurShader);
	//MakeTargetCurrent(&m_pp2RT, m_pp1RT);
	//ApplyPostProc(m_boxBlurShader);
	//MakeTargetCurrent(&m_pp1RT, m_pp2RT);
	//ApplyPostProc(m_boxBlurShader);
#endif

	MakeTargetCurrent(NULL, m_pp1RT);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, m_ppRT.GetTexture());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	ApplyPostProc(m_outlineShader);
	glDisable(GL_BLEND);

	//oglutil::DrawColorfulQuad();
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_DEPTH_TEST);
}

void MeshRendererEngine::SetupOrthoMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, GetWidth(), 0, GetHeight(), -1, 1);
	glMatrixMode(GL_MODELVIEW);
}

void MeshRendererEngine::UndoSetupOrthoMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}
