#include <Core/Actor.h>
#include <Core/Debug.h>
#include <Utils/MemoryMonitor.h>

Actor::Actor(Component* parent_):Component(parent_) {}

bool Actor::OnCreate() {
	if (isCreated) return true;
	Debug::Info("Loading assets for Actor: ", __FILE__, __LINE__);
	for (auto component : components) {
		if (component->OnCreate() == false) {
			Debug::Error("Component failed OnCreate: " + std::string(typeid(*component).name()), __FILE__, __LINE__);
			isCreated = false;
			return isCreated;
		}
	}
	isCreated = true;
	return isCreated;
}

Actor::~Actor() {
	OnDestroy();
	
}

void Actor::OnDestroy() {
	Debug::Info("Deleting assets for Actor: ", __FILE__, __LINE__);
	RemoveAllComponents();
	isCreated = false;
}



void Actor::Update(const float deltaTime) {
	std::cout << "Hello from Update\n";
}

void Actor::Render()const {}

void Actor::RemoveAllComponents() {
	for (auto component : components) {
		if (component != nullptr) {
			component->OnDestroy(); // Ensure the component cleans up its own GL/SDL resources
			delete component;       // This actually calls the destructor and frees the heap memory
		}
	}
	components.clear(); // Now it's safe to empty the vector
}

void Actor::ListComponents() const {
	std::cout << typeid(*this).name() << " contains the following components:\n";
	for (Component* component : components) {
		std::cout << typeid(*component).name() << std::endl;
	}
	std::cout << '\n';
}
//
//MATH::Matrix4 Actor::GetModelMatrix() {
//	
//	Ref<TransformComponent> transform = GetComponent<TransformComponent>();
//	if (transform.get()) {
//		modelMatrix = transform->GetTransformMatrix();
//	} else {
//		modelMatrix.loadIdentity();
//	}
//	if (parent) {
//		modelMatrix = dynamic_cast<Actor*>(parent)->GetModelMatrix() * modelMatrix;
//	}
//	return modelMatrix;
//}
//
MATH::Matrix4 Actor::GetModelMatrix() {

	MATH::Matrix4 modelMatrix;
	TransformComponent* transform = GetComponent<TransformComponent>();
	if (transform != nullptr) {
		modelMatrix = transform->GetTransformMatrix();
	} else {
		modelMatrix.loadIdentity();
	}
	if (parent) {
		modelMatrix = dynamic_cast<Actor*>(parent)->GetComponent<TransformComponent>()->GetTranslate_Rotate_Matrix() * modelMatrix;
	}
	return modelMatrix;
}