#include <Graphics/AssetManager.h>
#include <Graphics/MaterialComponent.h>
#include <Graphics/MeshComponent.h>
#include <Graphics/ShaderComponent.h>
#include <Physics/TransformComponent.h>
#include <Graphics/SkyBoxComponent.h>
#include <Physics/PhysicsComponent.h>
#include <Physics/CollisionComponent.h>

// Constructor is now parameterless
AssetManager::AssetManager() 
{
}

AssetManager::~AssetManager()
{
    OnDestroy();
}

// New method to handle what the constructor used to do
void AssetManager::Initialize(const char* assetDirectory)
{
    document_.LoadFile(assetDirectory);
}

bool AssetManager::OnCreate()
{
    bool status = document_.Error();
    if (status) {
        std::cout << document_.ErrorIDToName(document_.ErrorID()) << std::endl;
        return false;
    }
    /// Jump to the first node or "root"
    XMLElement* rootData = document_.RootElement();
    
    for (const XMLElement* element = rootData->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
    {
        std::string tagName = element->Value();
        std::cout << "Element [" << element->Value() << "]: ";
        if (element->GetText() != nullptr) {
            std::cout << element->GetText() << '\n';
        }

        if (tagName == "Mesh") {
            const char* name = element->Attribute("meshName");
            const char* file = element->Attribute("filename");
            if (name && file) {
                AddComponent<MeshComponent>(name, std::weak_ptr<Component>(), file);
            }
        } 
        else if (tagName == "Material") {
            const char* name = element->Attribute("textureName");
            const char* file = element->Attribute("filename");
            if (name && file) {
                AddComponent<MaterialComponent>(name, std::weak_ptr<Component>(), file);
            }
        }
        else if (tagName == "Shader") {
            const char* name = element->Attribute("shaderName");
            const char* vert = element->Attribute("vertexShader");
            const char* frag = element->Attribute("fragmentShader");
            if (name && vert && frag) {
                AddComponent<ShaderComponent>(name, std::weak_ptr<Component>(), vert, frag);
            }
        }
        else if (tagName == "LightPosition") {
            const char* name = element->Attribute("posName"); 

            if (name) {
                float x = 0.0f, y = 0.0f, z = 0.0f;
                element->QueryFloatAttribute("x", &x);
                element->QueryFloatAttribute("y", &y);
                element->QueryFloatAttribute("z", &z);
                AddComponent<PhysicsComponent>(name, std::weak_ptr<Component>(), Vec3(x, y, z), Quaternion());
                std::cout << "Added Light Position: " << name << " at (" << x << ", " << y << ", " << z << ")" << std::endl;
            }
        }
        std::cout << '\n';
    }
    
    for (const auto& [Name, component] : componentCatalog) {
        std::cout << "Component [" << Name << "]: " << typeid(*component).name() << std::endl;
        component->OnCreate();
    }
    return true;
}

void AssetManager::OnDestroy()
{
    RemoveAllComponents();
}

void AssetManager::RemoveAllComponents()
{
    componentCatalog.clear();
}

void AssetManager::ListAllComponents() const
{
    std::cout << typeid(*this).name() << " contains the following components:\n";
    for (const auto& [Name, component] : componentCatalog) {
        std::cout << typeid(*component).name() << std::endl;
    }
    std::cout << '\n';
}
std::unique_ptr<AssetManager> AssetManager::instance = nullptr;