#include <Graphics/SkyBoxComponent.h>
#include <SDL_image.h>
#include <Utils/MemoryMonitor.h>

SkyBoxComponent::SkyBoxComponent(std::weak_ptr<Component> parent_,const char* posXFileName_, const char* negXFileName_,
	const char* posYFileName_, const char* negYFileName_,
	const char* posZFileName_, const char* negZFileName_) : Component(parent_), posXFileName(posXFileName_),
negXFileName(negXFileName_),
posYFileName(posYFileName_),
negYFileName(negYFileName_),
posZFileName(posZFileName_),
negZFileName(negZFileName_),
ID(0){

}

SkyBoxComponent::~SkyBoxComponent() {
}

bool SkyBoxComponent::OnCreate() {
	if (!posXFileName) {
		std::cerr << "Skybox texture paths not set!" << std::endl;
		return false;
	}
	std::vector<const char*> faces = {
	 posXFileName,
	 negXFileName,
	 posYFileName,
	 negYFileName,
	 posZFileName,
	 negZFileName
	};
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	for (unsigned int i = 0; i < faces.size(); i++) {
		SDL_Surface* textureSurface = IMG_Load(faces[i]);
		if (textureSurface) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, textureSurface->w, textureSurface->h, 0,
				GL_RGB, GL_UNSIGNED_BYTE, textureSurface->pixels);
			SDL_DestroySurface(textureSurface);
		}
		else {
			std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			return false;
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	return true;
}

void SkyBoxComponent::OnDestroy() {
	if (ID != 0) {
		glDeleteTextures(1, &ID);
		ID = 0;
	}
	posXFileName = nullptr;
	negXFileName = nullptr;
	posYFileName = nullptr;
	negYFileName = nullptr;
	posZFileName = nullptr;
	negZFileName = nullptr;
}

void SkyBoxComponent::Update(const float deltaTime_)
{
}

void SkyBoxComponent::Render() const
{

}


