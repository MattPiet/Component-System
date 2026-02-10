#pragma once
#include <glew.h>
#include <Core/Actor.h>
#include <Matrix.h>
#include <MMath.h>
#include <QMath.h>

using namespace MATH;

class LightActor : public Actor
{
	Vec4 Specular = Vec4(0.0, 0.0, 0.0, 0.0);
	Vec4 Diffuse = Vec4(0.0, 0.6, 0.0, 0.0);
	Vec4 Ambient = Vec4(0.0, 0.6, 0.0, 0.0);
public:
	LightActor(Actor* parent_, Vec4 Specular_ = Vec4(0.0, 0.0, 0.0, 0.0), Vec4 Diffuse_ = Vec4(0.0, 0.0, 0.0, 0.0), Vec4 Ambient = Vec4(0.0, 0.0, 0.0, 0.0));
	~LightActor();

	void OnDestroy() override;
	void Update(const float deltaTime) override;
	bool OnCreate() override;
	void Render() const override;

	void SetSpecular(const Vec4& specular) { Specular = specular; }	
	void SetDiffuse(const Vec4& diffuse) { Diffuse = diffuse; }
	void SetAmbient(const Vec4& ambient) { Ambient = ambient; }

	Vec4 GetSpecular() const { return Specular; }
	Vec4 GetDiffuse() const { return Diffuse; }
	Vec4 GetAmbient() const { return Ambient; }

};


