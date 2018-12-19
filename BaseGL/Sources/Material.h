#ifndef MATERIAL_H
#define MATERIAL_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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


GLuint loadTextureFromFileToGPU(const std::string & filename, bool isNormalMap){
  int width,height,numComponents;
  GLuint texID;
  glGenTextures(1,&texID);
  glBindTexture(GL_TEXTURE_2D,texID);
  if(isNormalMap){
    float * data = stbi_loadf(filename.c_str(),&width,&height,&numComponents,0);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,width,height,0,GL_RGBA,GL_FLOAT,data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
  }
  else {
    unsigned char * data = stbi_load(filename.c_str(),&width,&height,&numComponents,0);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,(numComponents==1?GL_RED:GL_RGB),width,height,0,(numComponents==1?GL_RED:GL_RGB),GL_UNSIGNED_BYTE,data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
  }
  glBindTexture(GL_TEXTURE_2D,0);
  return texID;
};


#endif // MATERIAL_H
