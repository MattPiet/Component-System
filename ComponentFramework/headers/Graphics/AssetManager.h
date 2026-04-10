#pragma once
#include <string>
#include <iostream>
#include <unordered_map> 
#include <Core/Component.h>
#include <Core/Debug.h>
#include <fstream>

#include <tinyxml2.h>

using namespace tinyxml2;

class AssetManager {
public:
    static AssetManager* GetInstance() {
        if (instance == nullptr)
        {
            instance = new AssetManager();
        }
        return instance;
    }
    
    AssetManager(const AssetManager&) = delete;
    void operator=(const AssetManager&) = delete;
    
    void Initialize(const char* assetDirectory);
    
    bool OnCreate();
    void OnDestroy();

    void RemoveAllComponents();
    void ListAllComponents() const;

    template<typename ComponentTemplate, typename ... Args>
    void AddComponent(const char* name, Args&& ... args_) {
        Ref<ComponentTemplate> comp = std::make_shared<ComponentTemplate>(std::forward<Args>(args_)...);
        componentCatalog[name] = comp;
    }

    template<typename ComponentTemplate>
    Ref<ComponentTemplate> GetComponent(const char* name) const {
        auto id = componentCatalog.find(name);
#ifdef _DEBUG
        if (id == componentCatalog.end()) {
            Debug::Error("Can't find requested component", __FILE__, __LINE__);
            return Ref<ComponentTemplate>(nullptr);
        }
#endif 
        return std::dynamic_pointer_cast<ComponentTemplate>(id->second);
    }
    
private:
    AssetManager(); 
    ~AssetManager();
    static AssetManager* instance;
    std::unordered_map<std::string , Ref<Component> > componentCatalog;
    XMLDocument document_;
};
