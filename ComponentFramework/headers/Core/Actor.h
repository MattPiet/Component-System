#pragma once
#include <vector>
#include <iostream>
#include <Matrix.h>
#include <Core/Component.h>
#include <Physics/TransformComponent.h>
#include <memory>

#include <Utils/MemoryMonitor.h>

class Actor : public Component {
	Actor(const Actor&) = delete;
	Actor(Actor&&) = delete;
	Actor& operator= (const Actor&) = delete;
	Actor& operator=(Actor&&) = delete;
protected:
	std::vector<Component*> components;
//	std::vector<std::shared_ptr<Component>> sharedComponents;

public:
	Actor(Component* parent_);
	virtual ~Actor();
	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;

	//template<typename ComponentTemplate>
	//void AddComponent(std::shared_ptr<ComponentTemplate> componentObject) {
	//	sharedComponents.push_back(componentObject);
	//}

	template<typename ComponentTemplate, typename ... Args>
	void AddComponent(Args&& ... args_) {
		ComponentTemplate* componentObject = M_new ComponentTemplate(std::forward<Args>(args_)...);
		components.push_back(componentObject);

	}

	template<typename ComponentTemplate>
	ComponentTemplate* GetComponent() {
		for (auto component : components) {
			if (dynamic_cast<ComponentTemplate*>(component) != nullptr) {
				return dynamic_cast<ComponentTemplate*>(component);
			}
		}
		return nullptr;
	}
	//template<typename ComponentTemplate>
	//std::shared_ptr<ComponentTemplate> GetSharedComponent() {
	//	for (auto component : sharedComponents) {
	//		if (dynamic_cast<std::shared_ptr<ComponentTemplate>>(component) != nullptr) {
	//			return dynamic_cast<std::shared_ptr<ComponentTemplate>>(component);
	//		}
	//	}
	//	return nullptr;
	//}

	template<typename ComponentTemplate> 
	void RemoveComponent() {
		for (size_t i = 0; i < components.size(); i++) {
			if (dynamic_cast<ComponentTemplate*>(components[i]) != nullptr) {
				components[i]->OnDestroy();
				delete components[i];
				components.erase(components.begin() + i);
				break;
			}
		}
	}

	void ListComponents() const;
	void RemoveAllComponents();

	MATH::Matrix4 GetModelMatrix();

};

