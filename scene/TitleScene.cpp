#include "TitleScene.h"
#include "../Manager/ImGuiManager.h"

void TitleScene::Initialize() {

	input_ = Input::GetInstance();


	viewProjection_.Initialize();
	for (int i = 0; i < kMaxObject; i++) {
		worldTransform_[i].Initialize();
	}

	// カメラの初期位置
	viewProjection_.translation_.z = -5.0f;

	// パーティクルの生成
	particles_ = new Particles();
	particles_->Initialize(true, true);

	sphere_ = new Sphere();
	sphere_->Initialize();

	ground_ = Model::CreateModelFromObj("resources/ground", "terrain.obj");

	isVibration_ = false;
}

void TitleScene::Update() {
	particles_->Update();
	for (int i = 0; i < kMaxObject; i++) {
		worldTransform_[i].UpdateMatrix();
	}

	if (input_->PressKey(DIK_SPACE)) {
		isVibration_ = true;
	}
	else {
		isVibration_ = false;
	}

	if (isVibration_) {
		input_->GamePadVibration(0, 60000, 0000);
	}
	else {
		input_->GamePadVibration(0, 0, 0);
	}

	XINPUT_STATE joyState;
	if (Input::GetInstance()->GetJoystickState(0, joyState)) {
		//// デッドゾーンの設定
		SHORT rightThumbX = Input::GetInstance()->ApplyDeadzone(joyState.Gamepad.sThumbRX);
		const float kRadian = 0.02f;
		viewProjection_.rotation_.y += (float)rightThumbX / SHRT_MAX * kRadian;
	}
	if (Input::GetInstance()->GetJoystickState(0, joyState)) {
		// デッドゾーンの設定
		SHORT leftThumbX = Input::GetInstance()->ApplyDeadzone(joyState.Gamepad.sThumbLX);
		SHORT leftThumbZ = Input::GetInstance()->ApplyDeadzone(joyState.Gamepad.sThumbLY);
		//const float kRadian = 0.02f;
		viewProjection_.translation_.x += (float)leftThumbX / SHRT_MAX * 0.5f;
		viewProjection_.translation_.z += (float)leftThumbZ / SHRT_MAX * 0.5f;
	}

	// Keyboard
	if (Input::GetInstance()->PressKey(DIK_LEFT)) {
		const float speed = -0.1f;
		Vector3 move = { speed,0,0 };

		viewProjection_.translation_ = Add(viewProjection_.translation_, move);
	}
	if (Input::GetInstance()->PressKey(DIK_RIGHT)) {
		const float speed = 0.1f;
		Vector3 move = { speed,0,0 };

		viewProjection_.translation_ = Add(viewProjection_.translation_, move);
	}
	if (Input::GetInstance()->PressKey(DIK_UP)) {
		const float speed = 0.1f;
		Vector3 move = { 0,0, speed };

		viewProjection_.translation_ = Add(viewProjection_.translation_, move);
	}
	if (Input::GetInstance()->PressKey(DIK_DOWN)) {
		const float speed = -0.1f;
		Vector3 move = { 0,0, speed };

		viewProjection_.translation_ = Add(viewProjection_.translation_, move);
	}

	// keyboard
	if (Input::GetInstance()->PressKey(DIK_W)) {
		viewProjection_.rotation_ = Add(viewProjection_.rotation_, { -0.01f,0,0 });
	}
	if (Input::GetInstance()->PressKey(DIK_A)) {
		viewProjection_.rotation_ = Add(viewProjection_.rotation_, { 0,-0.01f,0 });
	}
	if (Input::GetInstance()->PressKey(DIK_S)) {
		viewProjection_.rotation_ = Add(viewProjection_.rotation_, { 0.01f,0,0 });
	}
	if (Input::GetInstance()->PressKey(DIK_D)) {
		viewProjection_.rotation_ = Add(viewProjection_.rotation_, { 0,0.01f,0 });
	}

	viewProjection_.UpdateViewMatrix();
	viewProjection_.TransferMatrix();

	ImGui::Begin("BlendMode");
	sphere_->ImGuiAdjustParameter();
	ground_->ImGuiAdjustParameter();
	ImGui::DragFloat3("worldTransform.translate", &worldTransform_[0].translation_.x, 0.1f, -100.0f, 100.0f);
	ImGui::DragFloat3("groundWorldTransform.translate", &worldTransform_[1].translation_.x, 0.1f, -100.0f, 100.0f);
	ImGui::End();

	particles_->ImGuiAdjustParameter();
}

void TitleScene::Draw() {
	particles_->Draw(viewProjection_, PARTICLE);
	sphere_->Draw(worldTransform_[0], viewProjection_);
	ground_->Draw(worldTransform_[1], viewProjection_, GROUND, 0);
}

void TitleScene::Finalize() {
	for (int i = 0; i < kMaxObject; i++) {
		worldTransform_[i].constBuff_.ReleaseAndGetAddressOf();
	}

	viewProjection_.constBuff_.ReleaseAndGetAddressOf();
	delete particles_;
}