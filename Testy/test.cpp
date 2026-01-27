#include "pch.h"
#include "../RayTracingINZ/src/Scene.h"

using namespace App;

class SceneTest : public ::testing::Test
{
protected:
	std::unique_ptr<Scene> scene;

protected:

	void SetUp()
	{
		SceneConfiguration sceneConfig;
		sceneConfig.numOfModels = 1;
		sceneConfig.numOfSpheres = 1;

		scene = std::make_unique<Scene>(sceneConfig);
	}
};

TEST_F(SceneTest, ReturnsFalseWhenFileDoesNotExist) 
{
	bool result = scene->AddObject("testPath/testFileName", 0);
	EXPECT_FALSE(result);
	EXPECT_EQ(scene->GetTriangles().size(), 0);
}

TEST_F(SceneTest, ReturnsFalseWhenPathIsEmpty)
{
	bool result = scene->AddObject("", 0);
	EXPECT_FALSE(result);
}

TEST_F(SceneTest, CorrectNumberOfTriangles)
{
	bool result = scene->AddObject("../../RayTracingINZ/res/models/model.obj", 0);
	EXPECT_EQ(scene->GetTriangles().size(), 264);
}