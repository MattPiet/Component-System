#pragma once
#include <iostream>
#include <glew.h>
#include <Core/Component.h>
class SkyBoxComponent : public Component {
	const char* posXFileName, * negXFileName, * posYFileName, * negYFileName, * posZFileName, * negZFileName;
	GLuint ID;
public:

	SkyBoxComponent(std::weak_ptr<Component> parent_, const char* posXFileName_, const char* negXFileName_,
		const char* posYFileName_, const char* negYFileName_,
		const char* posZFileName_, const char* negZFileName_);
	~SkyBoxComponent();
	inline GLuint getTextureID() const { return ID; }

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime_) override;
	virtual void Render()const override;

};
