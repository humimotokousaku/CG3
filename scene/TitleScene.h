#pragma once
#include "IScene.h"
#include "../components/Input.h"
#include "../object/Sprite.h"
#include "../object/Sphere.h"
#include "../base/WorldTransform.h"
#include "../base/ViewProjection.h"
#include "../base/Model.h"
#include "../Manager/PipelineManager.h"
#include "../Particles.h"
#include <iostream>
#include <cmath>

class GameManager;

class TitleScene : public IScene
{
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize()override;
private:
	Input* input_;
	ViewProjection viewProjection_;

	Particles* particles_;

	bool isVibration_;
};