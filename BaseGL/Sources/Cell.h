#ifndef CELL_H
#define CELL_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Cell{
private:
  glm::vec2 X;
  glm::vec2 Y;
  glm::vec2 Z;

public:
  inline void setX(glm::vec2 _X){X = _X;};
  inline void setY(glm::vec2 _Y){Y = _Y;};
  inline void setZ(glm::vec2 _Z){Z = _Z;};

  inline glm::vec2 getX(){return X;};
  inline glm::vec2 getY(){return Y;};
  inline glm::vec2 getZ(){return Z;};

  bool isInCell(glm::vec3 v);
};
#endif //CELL_H
