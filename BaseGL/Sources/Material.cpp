#include <string>
#include "Material.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

GLuint loadTextureFromFileToGPU(const std::string & filename, bool isNormalMap){
  int width,height,numComponents;

  unsigned char * data = stbi_load(filename.c_str(),&width,&height,&numComponents,0);

  GLuint texID;
  glGenTextures(1,&texID);
  glBindTexture(GL_TEXTURE_2D,texID);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  if(isNormalMap){
    glTexImage2D(GL_TEXTURE_2D,0,(numComponents==1?GL_RED:GL_RGB16F),width,height,0,(numComponents==1?GL_RED:GL_RGB),GL_FLOAT,data);
  }
  else {
    glTexImage2D(GL_TEXTURE_2D,0,(numComponents==1?GL_RED:GL_RGB),width,height,0,(numComponents==1?GL_RED:GL_RGB),GL_UNSIGNED_BYTE,data);
  }
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D,0);
  return texID;
};
