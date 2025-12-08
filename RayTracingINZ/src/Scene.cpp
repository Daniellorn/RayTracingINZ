#include "Scene.h"

#include "assimp/Importer.hpp"     // C++ importer interface
#include "assimp/scene.h"          // Output data structure
#include "assimp/postprocess.h"     // Post processing flags

namespace App {

	void Scene::AddObject(const Sphere& sphere)
	{
		m_Objects.push_back(sphere);
	}

	void Scene::AddMaterial(const Material& material)
	{
		m_Materials.push_back(material);
	}

	bool Scene::AddObject(const std::filesystem::path& filepath, int materialIndex)
	{

		if (!std::filesystem::exists(filepath))
		{
			return false;
		}

		Assimp::Importer importer;

		const aiScene* pScene = importer.ReadFile(filepath.string(), aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenNormals);

		if (pScene == nullptr)
		{
			return false;
		}

		for (int i = 0; i < pScene->mNumMeshes; i++)
		{
			aiMesh* mesh = pScene->mMeshes[i];

			int startIndex = (int)m_Triangles.size();

			for (int j = 0; j < mesh->mNumFaces; j++)
			{
				const aiFace& face = mesh->mFaces[j];

				Triangle tri;

				for (int k = 0; k < 3; k++)
				{
					int idx = face.mIndices[k];
					aiVector3D pos = mesh->mVertices[idx];
					aiVector3D normal = mesh->mNormals[idx];

					if (k == 0)
					{
						tri.v1 = DirectX::XMFLOAT4(pos.x, pos.y, pos.z, 1.0f);
						tri.n1 = DirectX::XMFLOAT4(normal.x, normal.y, normal.z, 1.0f);
					}
					if (k == 1)
					{
						tri.v2 = DirectX::XMFLOAT4(pos.x, pos.y, pos.z, 1.0f);
						tri.n2 = DirectX::XMFLOAT4(normal.x, normal.y, normal.z, 1.0f);
					}
					if (k == 2)
					{
						tri.v3 = DirectX::XMFLOAT4(pos.x, pos.y, pos.z, 1.0f);
						tri.n3 = DirectX::XMFLOAT4(normal.x, normal.y, normal.z, 1.0f);
					}
				}

				m_Triangles.push_back(tri);
			}

			Model model;
			model.startTriangle = startIndex;
			model.triangleCount = (int)mesh->mNumFaces;
			model.materialIndex = materialIndex;

			m_Models.push_back(model);
			startIndex += (int)mesh->mNumFaces;
		}

		return true;

	}

}