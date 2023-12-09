#pragma once
#include "IScene.h"
#include "../components/Input.h"
#include "../object/Sprite.h"
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
	const static int kMaxObject = 2;
	Input* input_;
	WorldTransform worldTransform_;
	ViewProjection viewProjection_;

	Particles* particles_;

	int blendMode_ = kBlendModeNone;

	bool isVibration_;


};