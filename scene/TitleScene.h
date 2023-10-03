#pragma once
#include "IScene.h"
#include "../components/Input.h"
#include "../Block.h"
#include "../Cube.h"
#include "../object/Sphere.h"
#include "../base/WorldTransform.h"
#include "../base/ViewProjection.h"
#include "../Model.h"

class GameManager;

class TitleScene : public IScene
{
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize()override;
private:
	Model* block_;
	Model* axis_;
	const static int kMaxCube = 100;
	int textureNum_;
	Input* input_;
	Vector3 pos_;
	WorldTransform cubeWorldTransform_[kMaxCube];
	WorldTransform worldTransform_;
	ViewProjection viewProjection_;
};