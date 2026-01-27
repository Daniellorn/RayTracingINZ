#include "Scene.h"

#include "assimp/Importer.hpp"     // C++ importer interface
#include "assimp/scene.h"          // Output data structure
#include "assimp/postprocess.h"     // Post processing flags

using namespace DirectX;

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

		for (uint32_t i = 0; i < pScene->mNumMeshes; i++)
		{
			aiMesh* mesh = pScene->mMeshes[i];

			int startIndex = (int)m_Triangles.size();

			for (uint32_t j = 0; j < mesh->mNumFaces; j++)
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

	void Scene::BuildBVH(int numOfTriangles)
	{
		m_BVHNodes.resize(2 * numOfTriangles - 1);
		m_TriIndexes.resize(numOfTriangles); 

		for (int i = 0; i < numOfTriangles; i++)
		{
			m_TriIndexes[i] = i;

			Triangle& tri = m_Triangles[i];

			XMVECTOR v1 = XMLoadFloat4(&tri.v1);
			XMVECTOR v2 = XMLoadFloat4(&tri.v2);
			XMVECTOR v3 = XMLoadFloat4(&tri.v3);

			XMVECTOR sum = XMVectorAdd(XMVectorAdd(v1, v2), v3);
			XMVECTOR centroid = XMVectorScale(sum, 0.3333f);

			XMStoreFloat4(&tri.centroid, centroid);
		}

		BVHNode& root = m_BVHNodes[rootNodeIndex];
		root.leftFirst = 0;
		root.triangleCount = numOfTriangles;

		UpdateNodeBounds(rootNodeIndex);
		SubDivide(rootNodeIndex);

		m_RenderConfiguration.numOfNodes = nodesUsed;
	}

	void Scene::UpdateNodeBounds(uint32_t nodeIndex)
	{
		BVHNode& node = m_BVHNodes[nodeIndex];
		XMVECTOR aabbMin = XMVectorSet(1e30f, 1e30f, 1e30f, 1e30f);
		XMVECTOR aabbMax = XMVectorSet(-1e30f, -1e30f, -1e30f, -1e30f);

		for (uint32_t first = node.leftFirst, i = 0; i < node.triangleCount; i++)
		{
			int leafTriIdx = m_TriIndexes[first + i];
			Triangle& leafTri = m_Triangles[leafTriIdx];

			XMVECTOR v1 = XMLoadFloat4(&leafTri.v1);
			XMVECTOR v2 = XMLoadFloat4(&leafTri.v2);
			XMVECTOR v3 = XMLoadFloat4(&leafTri.v3);

			aabbMin = XMVectorMin(aabbMin, v1);
			aabbMin = XMVectorMin(aabbMin, v2);
			aabbMin = XMVectorMin(aabbMin, v3);

			aabbMax = XMVectorMax(aabbMax, v1);
			aabbMax = XMVectorMax(aabbMax, v2);
			aabbMax = XMVectorMax(aabbMax, v3);

		}

		XMStoreFloat4(&node.aabbMin, aabbMin);
		XMStoreFloat4(&node.aabbMax, aabbMax);
	}

	void Scene::SubDivide(uint32_t nodeIndex)
	{
		BVHNode& node = m_BVHNodes[nodeIndex];

		if (node.triangleCount <= 2)
		{
			return;
		}

		XMVECTOR aabbMin = XMLoadFloat4(&node.aabbMin);
		XMVECTOR aabbMax = XMLoadFloat4(&node.aabbMax);

		XMVECTOR extent = XMVectorSubtract(aabbMax, aabbMin);
		int axis = 0;

		XMFLOAT4 ex;
		XMStoreFloat4(&ex, extent);

		if (ex.y > ex.x)
		{
			axis = 1;
		}

		if (ex.z > ((axis == 0) ? ex.x : ex.y))
		{
			axis = 2;
		}

		XMFLOAT4 aabbMinf;
		XMStoreFloat4(&aabbMinf, aabbMin);

		float splitPos = 0.0f;
		if (axis == 0)
		{
			splitPos = aabbMinf.x + ex.x * 0.5f;
		}
		else if (axis == 1)
		{
			splitPos = aabbMinf.y + ex.y * 0.5f;
		}
		else
		{
			splitPos = aabbMinf.z + ex.z * 0.5f;
		}
		
		int i = node.leftFirst;
		int j = i + node.triangleCount - 1;

		while (i <= j)
		{
			XMVECTOR cen = XMLoadFloat4(&m_Triangles[m_TriIndexes[i]].centroid);

			float centroidAxis = 0.0f;
			if (axis == 0)
			{
				centroidAxis = XMVectorGetX(cen);
			}
			else if (axis == 1)
			{
				centroidAxis = XMVectorGetY(cen);
			}
			else
			{
				centroidAxis = XMVectorGetZ(cen);
			}

			if (centroidAxis < splitPos)
			{
				i++;
			}
			else
			{
				std::swap(m_TriIndexes[i], m_TriIndexes[j]);
				j--;
			}
		}

		int leftCount = i - node.leftFirst;

		if (leftCount == 0 || leftCount == node.triangleCount)
		{
			return;
		}

		int leftChildIndex = nodesUsed++;
		int rightChildIndex = nodesUsed++;


		m_BVHNodes[leftChildIndex].leftFirst = node.leftFirst;
		m_BVHNodes[leftChildIndex].triangleCount = leftCount;
		m_BVHNodes[rightChildIndex].leftFirst = i;
		m_BVHNodes[rightChildIndex].triangleCount = node.triangleCount - leftCount;
		node.leftFirst = leftChildIndex;
		node.triangleCount = 0;

		UpdateNodeBounds(leftChildIndex);
		UpdateNodeBounds(rightChildIndex);

		SubDivide(leftChildIndex);
		SubDivide(rightChildIndex);
	}

}