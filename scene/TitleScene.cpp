#include "TitleScene.h"
#include "../Manager/ImGuiManager.h"

void TitleScene::Initialize() {
	plane_ = Model::CreateModelFromObj("resources", "plane.obj");

	input_ = Input::GetInstance();

	for (int i = 0; i < kMaxSprite; i++) {
		spriteWorldTransform_[i].Initialize();
	}
	worldTransform_.Initialize();
	viewProjection_.Initialize();

	for (int i = 0; i < kMaxSprite; i++) {
		spriteWorldTransform_[i].translation_.z = (float)4;
	}

	// カメラの初期位置
	viewProjection_.translation_.z = -1.0f;
}

void TitleScene::Update() {
	for (int i = 0; i < kMaxSprite; i++) {
		spriteWorldTransform_[i].UpdateMatrix();
	}
	worldTransform_.UpdateMatrix();

	//// 追従対象からカメラまでのオフセット
	//Vector3 offset = { 0.0f, 4.0f, -10.0f };
	//// カメラの角度から回転行列を計算
	//Matrix4x4 rotateMatrix = MakeRotateMatrix(viewProjection_.viewProjection_.rotation);

	//// オフセットをカメラの回転に合わせて回転
	//offset = TransformNormal(offset, rotateMatrix);

	//// 座標をコピーしてオフセット分ずらす
	//viewProjection_.translation_ = Add(worldTransform_.translation_, offset);
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

		// 移動ベクトルをカメラの角度だけ回転
		move = TransformNormal(move, viewProjection_.matView);

		viewProjection_.translation_ = Add(viewProjection_.translation_, move);
	}
	if (Input::GetInstance()->PressKey(DIK_RIGHT)) {
		const float speed = 0.1f;

		Vector3 move = { speed,0,0 };

		// 移動ベクトルをカメラの角度だけ回転
		move = TransformNormal(move, viewProjection_.matView);

		viewProjection_.translation_ = Add(viewProjection_.translation_, move);
	}
	if (Input::GetInstance()->PressKey(DIK_UP)) {
		const float speed = 0.1f;

		Vector3 move = { 0,0, speed };

		// 移動ベクトルをカメラの角度だけ回転
		move = TransformNormal(move, viewProjection_.matView);

		viewProjection_.translation_ = Add(viewProjection_.translation_, move);
	}
	if (Input::GetInstance()->PressKey(DIK_DOWN)) {
		const float speed = -0.1f;

		Vector3 move = { 0,0, speed };

		// 移動ベクトルをカメラの角度だけ回転
		move = TransformNormal(move, viewProjection_.matView);

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

	plane_->ImGuiAdjustParameter();

	ImGui::Begin("BlendMode");
	ImGui::SliderInt("Mode",&blendMode_, 0, 5);
	ImGui::End();
}

void TitleScene::Draw() {
	plane_->Draw(worldTransform_, viewProjection_, blendMode_);
}

void TitleScene::Finalize() {
	for (int i = 0; i < kMaxSprite; i++) {
		spriteWorldTransform_[i].constBuff_.ReleaseAndGetAddressOf();
	}
	delete plane_;

	worldTransform_.constBuff_.ReleaseAndGetAddressOf();
	viewProjection_.constBuff_.ReleaseAndGetAddressOf();
}