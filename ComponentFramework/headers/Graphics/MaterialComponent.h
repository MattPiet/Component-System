#pragma once
#include <glew.h>
#include <Core/Component.h>
class MaterialComponent: public Component {
private:
	GLuint textureID;
	const char* filename;
public:
	bool LoadImage(const char* filename);
	MaterialComponent(std::weak_ptr<Component> parent_,const char* filename_);
	virtual ~MaterialComponent();
	
	inline GLuint getTextureID() const { return textureID; }

	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void Update(const float deltaTime_);
	virtual void Render()const;
};

