#pragma once

#include <vector>

#include <DirectXMath.h>

namespace App {

	enum class Model
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

	struct SceneConfiguration
	{
		int numOfSpheres;
		int numOfBounces;
	};

	class Scene
	{
	public:

		Scene(const SceneConfiguration& sceneConfiguration) : m_SceneConfiguration(sceneConfiguration) {};
		~Scene() = default;

		void AddObject(const Sphere& sphere);
		void AddMaterial(const Material& material);

		std::vector<Sphere>& GetSpheres() { return m_Objects; }
		std::vector<Material>& GetMaterials() { return m_Materials; }
		SceneConfiguration& GetSceneConfiguration() { return m_SceneConfiguration; }

	private:
		std::vector<Sphere> m_Objects;
		std::vector<Material> m_Materials;

		SceneConfiguration m_SceneConfiguration;
	};
}