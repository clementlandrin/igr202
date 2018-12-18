#ifndef MATERIAL_H
#define MATERIAL_H

#include <glad/glad.h>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Material {
private:
  glm::vec3 albedo;
  float kd;
  float metallic;
  float roughness;
public:
  Material(){};
  glm::vec3 getAlbedo(){return albedo;};
  float getKd(){return kd;};
  float getMetallic(){return metallic;};
  float getRoughness(){return roughness;};
  void setAlbedo(glm::vec3 _albedo){albedo=_albedo;};
  void setKd(float _kd){kd=_kd;};
  void setMetallic(float _metallic){metallic=_metallic;};
  void setRoughness(float _roughness){roughness=_roughness;};
};


GLuint loadTextureFromFileToGPU(const std::string & filename, bool isNormalMap);


#endif // MATERIAL_H
