#ifndef OCTREE_H
#define OCTREE_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <vector>
#include "Cell.h"

class Octree {
private:
  int numOfPerLeafVertices;
  std::vector<Cell> cells;
  std::vector<Octree> subTrees;
  glm::vec2 X;
  glm::vec2 Y;
  glm::vec2 Z;
  std::vector<int> perSubCubeVertexNumbers;
  bool isSubdivided = false;
  int maxDepth = 16;
  int numberOfVertices;
  int localisationInSpace = 0; // equals 0 if root
                               // 1 if top back left
                               // 2 if top front left
                               // 3 if bottom front left
                               // 4 if botton back left
                               // 5 if top back right
                               // 6 if top front right
                               // 7 if bottom front right
                               // 8 if bottom back right


public:
  inline glm::vec2 getX(){return X;};
  inline glm::vec2 getY(){return Y;};
  inline glm::vec2 getZ(){return Z;};

  inline void setX(glm::vec2 _X){X = _X;};
  inline void setY(glm::vec2 _Y){Y = _Y;};
  inline void setZ(glm::vec2 _Z){Z = _Z;};

  bool canBeSubdivided();

  void incrementSubCube(glm::vec3 v);
};

#endif // OCTREE_H
