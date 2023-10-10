#pragma once
#include "IScene.h"
#include "../components/Input.h"
#include "../object/Sprite.h"
#include "../base/WorldTransform.h"
#include "../base/ViewProjection.h"
#include "../base/Model.h"
#include "../Manager/PipelineManager.h"

class GameManager;

class TitleScene : public IScene
{
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize()override;
private:
	Model* plane_;
	const static int kMaxSprite = 1;
	Input* input_;
	WorldTransform spriteWorldTransform_[kMaxSprite];
	WorldTransform worldTransform_;
	ViewProjection viewProjection_;

	int blendMode_ = kBlendModeNone;
};