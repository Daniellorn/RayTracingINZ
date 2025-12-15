#pragma once

#include <vector>

#include <DirectXMath.h>
#include <filesystem>

namespace App {

	enum class Type
	{
		DIFFUSE = 0,
		SPECULAR,
		EMISSIVE
	};


	// TODO: Na kolory zrobiæ jakiœ enum.
	struct Material
	{
		DirectX::XMFLOAT4 albedo;
		DirectX::XMFLOAT4 EmissionColor;
		float roughness;
		float glossiness;
		float EmissionPower;
	};

	struct Sphere
	{
		DirectX::XMFLOAT4 position;
		float radius;

		int materialIndex;
		int type;

		auto operator<=>(const Sphere& other) const = default;
	};


	struct Lights
	{
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 color;
		float radius;
		float intensity;
	};

	struct Triangle
	{
		DirectX::XMFLOAT4 v1;
		DirectX::XMFLOAT4 v2;
		DirectX::XMFLOAT4 v3;

		DirectX::XMFLOAT4 n1;
		DirectX::XMFLOAT4 n2; 
		DirectX::XMFLOAT4 n3;

		DirectX::XMFLOAT4 centroid;
	};

	struct BVHNode
	{
		DirectX::XMFLOAT4 aabbMin, aabbMax;
		uint32_t leftFirst, triangleCount; 
		// LeftFirst - jesli traingleCount = 0, to leftFirst jest indeksem lewego potomka. 
		// Jesli triangleCount > 0 to znaczy ze jestesmy w lisciu i leftFirst jest indeksem pierwszego trojkata.
	};

	//struct BVHNode2
	//{
	//	union
	//	{
	//		struct
	//		{
	//			DirectX::XMFLOAT3 aabbMin;
	//			uint32_t leftFirst;
	//		};
	//
	//		DirectX::XMFLOAT4 aabbmin;
	//	};
	//
	//	union
	//	{
	//		struct
	//		{
	//			DirectX::XMFLOAT3 aabbMax;
	//			uint32_t triangleCount;
	//		};
	//
	//		DirectX::XMFLOAT4 aabbmax;
	//	};
	//};

	struct Model
	{
		int startTriangle;
		int triangleCount;
		int materialIndex;
	};

	struct SceneConfiguration
	{
		int numOfSpheres;
		int numOfModels;
	};

	struct alignas(16) RenderConfiguration
	{
		int frameIndex = 1;
		int raysPerPixel = 1;
		int numOfBounces = 2;
		int accumulate = 0;
	};

	class Scene
	{
	public:

		Scene(const SceneConfiguration& sceneConfiguration) : m_SceneConfiguration(sceneConfiguration) 
		{};
		~Scene() = default;

		void AddObject(const Sphere& sphere);
		void AddMaterial(const Material& material);

		bool AddObject(const std::filesystem::path& filepath, int materialIndex);
		
		std::vector<Sphere>& GetSpheres() { return m_Objects; }
		std::vector<Material>& GetMaterials() { return m_Materials; }
		std::vector<Triangle>& GetTriangles() { return m_Triangles; }
		std::vector<BVHNode>& GetBVHNodes() { return m_BVHNodes; }
		std::vector<Model>& GetModels() { return m_Models; }
		SceneConfiguration& GetSceneConfiguration() { return m_SceneConfiguration; }
		RenderConfiguration& GetRenderConfiguration() { return m_RenderConfiguration; }

		
		void BuildBVH(int numOfTriangles);

	private:
		void UpdateNodeBounds(uint32_t nodeIndex);
		void SubDivide(uint32_t nodeIndex);

	private:
		std::vector<Sphere> m_Objects;
		std::vector<Material> m_Materials;
		std::vector<Triangle> m_Triangles;
		std::vector<Model> m_Models;
		std::vector<BVHNode> m_BVHNodes;

		uint32_t rootNodeIndex = 0;
		uint32_t nodesUsed = 1;

		SceneConfiguration m_SceneConfiguration;
		RenderConfiguration m_RenderConfiguration;
	};
}