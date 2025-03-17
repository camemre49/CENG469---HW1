#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "BezierCurveGenerator.h"
#include "stb_image.h"

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

GLuint vao[3];
GLuint gProgram[3];
int gWidth, gHeight;

GLint modelingMatrixLoc[3];
GLint viewingMatrixLoc[3];
GLint projectionMatrixLoc[3];
GLint eyePosLoc[3];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 0);
glm::vec3 eyeGaze(0, 0, -1);
glm::vec3 eyeUp(0, 1, 0);


struct Vertex
{
	Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) {}
	GLfloat x, y, z;
};

struct Texture
{
	Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) {}
	GLfloat u, v;
};

struct Normal
{
	Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) {}
	GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
	GLuint vIndex[3], tIndex[3], nIndex[3];
};

vector<Vertex> gVertices[3];
vector<Texture> gTextures[3];
vector<Normal> gNormals[3];
vector<Face> gFaces[3];

GLuint gVertexAttribBuffer[3], gIndexBuffer[3];
GLint gInVertexLoc[3], gInNormalLoc[3];
int gVertexDataSizeInBytes[3], gNormalDataSizeInBytes[3], gTextureDataSizeInBytes[3];

// Defined variables by me
BezierCurveGenerator bezierCurveGenerator(-1.2f, 1.2f);
glm::vec3 objectCenter = glm::vec3(0.0f, 0.0f, 0.0f);

bool ParseObj(const string& fileName, int objId)
{
	fstream myfile;

	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			stringstream str(curLine);
			GLfloat c1, c2, c3;
			GLuint index[9];
			string tmp;

			if (curLine.length() >= 2)
			{
				if (curLine[0] == 'v')
				{
					if (curLine[1] == 't') // texture
					{
						str >> tmp; // consume "vt"
						str >> c1 >> c2;
						gTextures[objId].push_back(Texture(c1, c2));
					}
					else if (curLine[1] == 'n') // normal
					{
						str >> tmp; // consume "vn"
						str >> c1 >> c2 >> c3;
						gNormals[objId].push_back(Normal(c1, c2, c3));
					}
					else // vertex
					{
						str >> tmp; // consume "v"
						str >> c1 >> c2 >> c3;
						gVertices[objId].push_back(Vertex(c1, c2, c3));
					}
				}
				else if (curLine[0] == 'f') // face
				{
					str >> tmp; // consume "f"
					char c;
					int vIndex[3], nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0];
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1];
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2];

					assert(vIndex[0] == nIndex[0] &&
						vIndex[1] == nIndex[1] &&
						vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

					gFaces[objId].push_back(Face(vIndex, tIndex, nIndex));
				}
				else
				{
					cout << "Ignoring unidentified line in obj file: " << curLine << endl;
				}
			}

			//data += curLine;
			if (!myfile.eof())
			{
				//data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	assert(gVertices[objId].size() == gNormals[objId].size());

	return true;
}

bool ReadDataFromFile(
	const string& fileName, ///< [in]  Name of the shader file
	string& data)     ///< [out] The contents of the file
{
	fstream myfile;

	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			data += curLine;
			if (!myfile.eof())
			{
				data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	return true;
}

GLuint createVS(const char* shaderName)
{
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &shader, &length);
	glCompileShader(vs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(vs, 1024, &length, output);
	printf("VS compile log: %s\n", output);

	return vs;
}

GLuint createFS(const char* shaderName)
{
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &shader, &length);
	glCompileShader(fs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(fs, 1024, &length, output);
	printf("FS compile log: %s\n", output);

	return fs;
}

void initTexture()
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load("haunted_library.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

void initShaders()
{
	// Create the programs

	gProgram[0] = glCreateProgram(); //for armadillo
	gProgram[1] = glCreateProgram(); //for background quad
	gProgram[2] = glCreateProgram(); //for curve

	// Create the shaders for both programs

	//for armadillo
	GLuint vs1 = createVS("vert.glsl"); //or vert2.glsl
	GLuint fs1 = createFS("frag.glsl"); //or frag2.glsl

	//for background quad
	GLuint vs2 = createVS("vert_quad.glsl");
	GLuint fs2 = createFS("frag_quad.glsl");

	//for background quad
	GLuint vs3 = createVS("vert2.glsl");
	GLuint fs3 = createFS("frag2.glsl");

	// Attach the shaders to the programs

	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);

	glAttachShader(gProgram[1], vs2);
	glAttachShader(gProgram[1], fs2);

	glAttachShader(gProgram[2], vs3);
	glAttachShader(gProgram[2], fs3);

	// Link the programs

	glLinkProgram(gProgram[0]);
	GLint status;
	glGetProgramiv(gProgram[0], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	glLinkProgram(gProgram[1]);
	glGetProgramiv(gProgram[1], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	glLinkProgram(gProgram[2]);
	glGetProgramiv(gProgram[2], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	// Get the locations of the uniform variables from both programs

	for (int i = 0; i < 3; ++i)
	{
		glUseProgram(gProgram[i]);

		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}
}

void initVBO()
{
	for (size_t t = 0; t < 2; t++) // 2 objects. t=0 is armadillo, t=1 is background quad.
	{
		glGenVertexArrays(1, &vao[t]);
		assert(vao[t] > 0);

		glBindVertexArray(vao[t]);
		cout << "vao = " << vao[t] << endl;

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		assert(glGetError() == GL_NONE);

		glGenBuffers(1, &gVertexAttribBuffer[t]);
		glGenBuffers(1, &gIndexBuffer[t]);

		assert(gVertexAttribBuffer[t] > 0 && gIndexBuffer[t] > 0);

		glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer[t]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer[t]);

		gVertexDataSizeInBytes[t] = gVertices[t].size() * 3 * sizeof(GLfloat);
		gNormalDataSizeInBytes[t] = gNormals[t].size() * 3 * sizeof(GLfloat);
		gTextureDataSizeInBytes[t] = gTextures[t].size() * 2 * sizeof(GLfloat);
		int indexDataSizeInBytes = gFaces[t].size() * 3 * sizeof(GLuint);

		GLfloat* vertexData = new GLfloat[gVertices[t].size() * 3];
		GLfloat* normalData = new GLfloat[gNormals[t].size() * 3];
		GLfloat* textureData = new GLfloat[gTextures[t].size() * 2];
		GLuint* indexData = new GLuint[gFaces[t].size() * 3];

		float minX = 1e6, maxX = -1e6;
		float minY = 1e6, maxY = -1e6;
		float minZ = 1e6, maxZ = -1e6;

		for (int i = 0; i < gVertices[t].size(); ++i)
		{
			vertexData[3 * i] = gVertices[t][i].x;
			vertexData[3 * i + 1] = gVertices[t][i].y;
			vertexData[3 * i + 2] = gVertices[t][i].z;

			minX = std::min(minX, gVertices[t][i].x);
			maxX = std::max(maxX, gVertices[t][i].x);
			minY = std::min(minY, gVertices[t][i].y);
			maxY = std::max(maxY, gVertices[t][i].y);
			minZ = std::min(minZ, gVertices[t][i].z);
			maxZ = std::max(maxZ, gVertices[t][i].z);

			if (t == 0) {
				objectCenter.x = (minX + maxX) / 2;
				objectCenter.y = (minY + maxY) / 2;
				objectCenter.z = (minZ + maxZ) / 2;
			}
		}

		std::cout << "minX = " << minX << std::endl;
		std::cout << "maxX = " << maxX << std::endl;
		std::cout << "minY = " << minY << std::endl;
		std::cout << "maxY = " << maxY << std::endl;
		std::cout << "minZ = " << minZ << std::endl;
		std::cout << "maxZ = " << maxZ << std::endl;

		for (int i = 0; i < gNormals[t].size(); ++i)
		{
			normalData[3 * i] = gNormals[t][i].x;
			normalData[3 * i + 1] = gNormals[t][i].y;
			normalData[3 * i + 2] = gNormals[t][i].z;
		}

		for (int i = 0; i < gTextures[t].size(); ++i)
		{
			textureData[2 * i] = gTextures[t][i].u;
			textureData[2 * i + 1] = gTextures[t][i].v;
		}

		for (int i = 0; i < gFaces[t].size(); ++i)
		{
			indexData[3 * i] = gFaces[t][i].vIndex[0];
			indexData[3 * i + 1] = gFaces[t][i].vIndex[1];
			indexData[3 * i + 2] = gFaces[t][i].vIndex[2];
		}


		glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes[t] + gNormalDataSizeInBytes[t] + gTextureDataSizeInBytes[t], 0, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes[t], vertexData);
		glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes[t], gNormalDataSizeInBytes[t], normalData);
		glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes[t] + gNormalDataSizeInBytes[t], gTextureDataSizeInBytes[t], textureData);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

		// done copying; can free now
		delete[] vertexData;
		delete[] normalData;
		delete[] textureData;
		delete[] indexData;

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes[t]));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes[t] + gNormalDataSizeInBytes[t]));
	}


	// VBO for curves
	glGenVertexArrays(1, &vao[2]);
	assert(vao[2] > 0);

	glBindVertexArray(vao[2]);
	cout << "vao = " << vao[2] << endl;

	glEnableVertexAttribArray(0);
	assert(glGetError() == GL_NONE);

	glGenBuffers(1, &gVertexAttribBuffer[2]);
	glGenBuffers(1, &gIndexBuffer[2]);

	assert(gVertexAttribBuffer[2] > 0 && gIndexBuffer[2] > 0);

	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer[2]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer[2]);

	// +4 stands for control points
	gVertexDataSizeInBytes[2] = (gVertices[2].size() + 5) * 3 * sizeof(GLfloat);
	int indexDataSizeInBytes = (BezierCurveGenerator::sampleCount + 5) * sizeof(GLuint);

	GLfloat* vertexData = new GLfloat[(gVertices[2].size() + 5) * 3];
	GLuint* indexData = new GLuint[BezierCurveGenerator::sampleCount + 5];

	float minX = 1e6, maxX = -1e6;
	float minY = 1e6, maxY = -1e6;
	float minZ = 1e6, maxZ = -1e6;

	for (int i = 0; i < gVertices[2].size(); ++i)
	{
		vertexData[3 * i] = gVertices[2][i].x;
		vertexData[3 * i + 1] = gVertices[2][i].y;
		vertexData[3 * i + 2] = gVertices[2][i].z;

		minX = std::min(minX, gVertices[2][i].x);
		maxX = std::max(maxX, gVertices[2][i].x);
		minY = std::min(minY, gVertices[2][i].y);
		maxY = std::max(maxY, gVertices[2][i].y);
		minZ = std::min(minZ, gVertices[2][i].z);
		maxZ = std::max(maxZ, gVertices[2][i].z);
	}

	std::cout << "minX = " << minX << std::endl;
	std::cout << "maxX = " << maxX << std::endl;
	std::cout << "minY = " << minY << std::endl;
	std::cout << "maxY = " << maxY << std::endl;
	std::cout << "minZ = " << minZ << std::endl;
	std::cout << "maxZ = " << maxZ << std::endl;

	for (int i = 0; i < BezierCurveGenerator::sampleCount + 5; ++i)
	{
		indexData[i] = i;
	}


	glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes[2], 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes[2], vertexData);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

	// done copying; can free now
	delete[] vertexData;
	delete[] indexData;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void init(BezierCurveGenerator &bezierCurveGenerator)
{
	ParseObj("armadillo.obj", 0);
	ParseObj("quad.obj", 1);

	glEnable(GL_DEPTH_TEST);
	initTexture();
	initShaders();


	bezierCurveGenerator.generateRandomCurveCPs();
	for (auto & currentCurveCoordinate : bezierCurveGenerator.currentCurveCoordinates) {
		gVertices[2].emplace_back(
		currentCurveCoordinate.x,
		currentCurveCoordinate.y,
		currentCurveCoordinate.z
		);
	}
	for (int i = 0; i < 4; i++) {
		gVertices[2].emplace_back(
			bezierCurveGenerator.currentCurveCPsMatrix[i].x,
			bezierCurveGenerator.currentCurveCPsMatrix[i].y,
			bezierCurveGenerator.currentCurveCPsMatrix[i].z);
	}

	gVertices[2].emplace_back(
			bezierCurveGenerator.currentCurveCPsMatrix[0].x,
			bezierCurveGenerator.currentCurveCPsMatrix[0].y,
			bezierCurveGenerator.currentCurveCPsMatrix[0].z);

	initVBO();
}

void drawScene()
{
	for (size_t t = 0; t < 2; t++)
	{
		// Set the active program and the values of its uniform variables
		glUseProgram(gProgram[t]);
		glUniformMatrix4fv(projectionMatrixLoc[t], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glUniformMatrix4fv(viewingMatrixLoc[t], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
		glUniformMatrix4fv(modelingMatrixLoc[t], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
		glUniform3fv(eyePosLoc[t], 1, glm::value_ptr(eyePos));

		glBindVertexArray(vao[t]);

		if (t == 1)
			glDepthMask(GL_FALSE);

		glDrawElements(GL_TRIANGLES, gFaces[t].size() * 3, GL_UNSIGNED_INT, 0);

		if (t == 1)
			glDepthMask(GL_TRUE);
	}

	// Draw the curve
	glUseProgram(gProgram[2]);
	glUniformMatrix4fv(projectionMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -5.0f));
	glUniformMatrix4fv(modelingMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(matT));

	glBindVertexArray(vao[2]);

	glm::vec4 lineColor(1.0f, 0.0f, 0.0f, 1.0f); // Red color for lines
	glUniform4fv(glGetUniformLocation(gProgram[2], "color"), 1, &lineColor[0]);
	glLineWidth(3.0f);
	glDrawElements(GL_LINE_STRIP, BezierCurveGenerator::sampleCount, GL_UNSIGNED_INT, 0);
	glLineWidth(1.0f);

	glm::vec4 pointColor(0.0f, 0.0f, 1.0f, 1.0f); // Blue color for points
	glUniform4fv(glGetUniformLocation(gProgram[2], "color"), 1, &pointColor[0]);
	glPointSize(5.0f);
	glDrawElements(GL_POINTS, 4, GL_UNSIGNED_INT, (void*)(100 * sizeof(GLuint)));
	glPointSize(1.0f);

	glm::vec4 positionColor(0.0f, 1.0f, 0.0f, 1.0f); // Blue color for points
	glUniform4fv(glGetUniformLocation(gProgram[2], "color"), 1, &positionColor[0]);
	glPointSize(15.0f);
	glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, (void*)(104 * sizeof(GLuint)));
	glPointSize(1.0f);
}

void display()
{
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	static float angle = 0;

	float angleRad = (float)(angle / 180.0) * M_PI;

	// Compute the modeling matrix

	//modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, -0.4f, -5.0f));
	//modelingMatrix = glm::rotate(modelingMatrix, angleRad, glm::vec3(0.0, 1.0, 0.0));
	glm::vec3 translationVector = bezierCurveGenerator.currentCurveCoordinates[bezierCurveGenerator.currentTIndex] - objectCenter;
	translationVector += glm::vec3(0.0f, 0.3f, -5.0f);
	glm::mat4 matT = glm::translate(glm::mat4(1.0),translationVector);
	//glm::mat4 matR = glm::rotate(glm::mat4(1.0), angleRad, glm::vec3(0.0, 1.0, 0.0));
	//glm::mat4 matRx = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	//glm::mat4 matRy = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
	//glm::mat4 matRz = glm::rotate<float>(glm::mat4(1.0), angleRad, glm::vec3(0.0, 0.0, 1.0));
	//modelingMatrix = matRy * matRx;
	modelingMatrix = glm::mat4(1.0f);

	// Scale the object.
	glm::vec3 scaleFactors(0.15f, 0.15f, 0.15f);;
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0), scaleFactors);
	modelingMatrix = scaleMatrix * modelingMatrix;

	// Let's make some alternating roll rotation
	static float rollDeg = 0;
	static float changeRoll = 2.5;
	float rollRad = (float)(rollDeg / 180.f) * M_PI;
	rollDeg += changeRoll;
	if (rollDeg >= 10.f || rollDeg <= -10.f)
	{
		changeRoll *= -1.f;
	}
	glm::mat4 matRoll = glm::rotate<float>(glm::mat4(1.0), rollRad, glm::vec3(1.0, 0.0, 0.0));

	// Let's make some pitch rotation
	static float pitchDeg = 0;
	static float changePitch = 0.1;
	float startPitch = 0;
	float endPitch = 90;
	float pitchRad = (float)(pitchDeg / 180.f) * M_PI;
	pitchDeg += changePitch;
	if (pitchDeg >= endPitch)
	{
		changePitch = 0;
	}
	//glm::mat4 matPitch = glm::rotate<float>(glm::mat4(1.0), pitchRad, glm::vec3(0.0, 0.0, 1.0));
	//modelingMatrix = matRoll * matPitch * modelingMatrix; // gimbal lock
	//modelingMatrix = matPitch * matRoll * modelingMatrix;   // no gimbal lock

	glm::quat q0(0, 1, 0, 0); // along x
	glm::quat q1(0, 0, 1, 0); // along y
	glm::quat q = glm::mix(q0, q1, (pitchDeg - startPitch) / (endPitch - startPitch));

	float sint = sin(rollRad / 2);
	glm::quat rollQuat(cos(rollRad / 2), sint * q.x, sint * q.y, sint * q.z);
	glm::quat pitchQuat(cos(pitchRad / 2), 0, 0, 1 * sin(pitchRad / 2));
	//modelingMatrix = matT * glm::toMat4(pitchQuat) * glm::toMat4(rollQuat) * modelingMatrix;
	// modelingMatrix = matT * glm::toMat4(rollQuat) * glm::toMat4(pitchQuat) * modelingMatrix; // roll is based on pitch
	modelingMatrix = matT * modelingMatrix;

	//cout << rollQuat.w << " " << rollQuat.x << " " << rollQuat.y << " " << rollQuat.z << endl;

	// Draw the scene
	drawScene();

	angle += 0.5;
}

void reshape(GLFWwindow* window, int w, int h)
{
	w = w < 1 ? 1 : w;
	h = h < 1 ? 1 : h;

	gWidth = w;
	gHeight = h;

	glViewport(0, 0, w, h);

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glOrtho(-10, 10, -10, 10, -10, 10);
	//gluPerspective(45, 1, 1, 100);

	// Use perspective projection

	float fovyRad = (float)(45.0 / 180.0) * M_PI;
	projectionMatrix = glm::perspective(fovyRad, 1.0f, 1.0f, 100.0f);

	// Assume a camera position and orientation (camera is at
	// (0, 0, 0) with looking at -z direction and its up vector pointing
	// at +y direction)

	viewingMatrix = glm::lookAt(eyePos, eyePos + eyeGaze, eyeUp);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		//glShadeModel(GL_FLAT);
	}
}

void updateObjectPosition() {
	static int frameCounter = 0;
	int updateRate = 4;

	if (frameCounter++ % updateRate != 0) {
		return; // Skip updates to slow down motion
	}

	glm::vec3 newPosition(
		bezierCurveGenerator.currentCurveCoordinates[bezierCurveGenerator.currentTIndex].x,
		bezierCurveGenerator.currentCurveCoordinates[bezierCurveGenerator.currentTIndex].y,
		bezierCurveGenerator.currentCurveCoordinates[bezierCurveGenerator.currentTIndex].z);

	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer[2]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer[2]);
	glBufferSubData(GL_ARRAY_BUFFER, 104 * 3 * sizeof(GL_FLOAT), 3 * sizeof(GL_FLOAT), glm::value_ptr(newPosition));

	// If the current curve is traveled fully, generate and store new curve information
	if (bezierCurveGenerator.increaseTIndex()) {
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			100 * 3 * sizeof(GL_FLOAT),
			bezierCurveGenerator.currentCurveCoordinates.data());

		glBufferSubData(
			GL_ARRAY_BUFFER,
			100 * 3 * sizeof(GL_FLOAT),
			4 * 3 * sizeof(GL_FLOAT),
			glm::value_ptr(bezierCurveGenerator.currentCurveCPsMatrix));
	}
}

void mainLoop(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();

		updateObjectPosition();
	}
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
	GLFWwindow* window;
	if (!glfwInit())
	{
		exit(-1);
	}

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	int width = 640, height = 480;
	window = glfwCreateWindow(width, height, "Simple Example", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	char rendererInfo[512] = { 0 };
	strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER));
	strcat(rendererInfo, " - ");
	strcat(rendererInfo, (const char*)glGetString(GL_VERSION));
	glfwSetWindowTitle(window, rendererInfo);

	init(bezierCurveGenerator);

	glfwSetKeyCallback(window, keyboard);
	glfwSetWindowSizeCallback(window, reshape);

	reshape(window, width, height); // need to call this once ourselves

	mainLoop(window); // this does not return unless the window is closed

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
