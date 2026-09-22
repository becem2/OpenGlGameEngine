#pragma once


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include "Shader.h"
#include "Mesh.h"


using namespace std;

class Model
{

public:

	Model(char* path);

	void Draw(Shader& shader);

private:

	vector<Mesh>meshes;
	string directory;


	void loadModel(string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
		string typeName);
};

