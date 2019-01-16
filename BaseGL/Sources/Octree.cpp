#include "Octree.h"

bool Octree::canBeSubdivided(){
  return false;
}

bool Octree::isInOctree(glm::vec3 v){
  return (X.x<=v.x)&&(v.x<=X.y)&&(Y.x<=v.y)&&(v.y<=Y.y)&&(Z.x<=v.z)&&(v.z<=Z.y);
}

bool Octree::isInSubTrees(glm::vec3 v){
  bool res = false;
  for(int i=0; i<subTrees.size(); i++){
    res = res || subTrees.isInSubTrees(v);
  }
}

int Octree::computeInWhichSubCube(glm::vec3 v){
  if(v.x<X.x+(X.x+X.y)/2){ // is in left part
    if(v.y<Y.x+(Y.x+Y.y)/2){ // is in bottom part
      if(v.z<Z.x+(Z.x+Z.y)/2){ // is in front part
        return 3;
      } else { // is in back part
        return 4;
      }
    } else { // is in top part
      if(v.z<Z.x+(Z.x+Z.y)/2){ // is in front part
        return 2;
      } else { // is in back part
        return 1;
      }
    }
  } else { // is in right part
    if(v.y<Y.x+(Y.x+Y.y)/2){ // is in bottom part
      if(v.z<Z.x+(Z.x+Z.y)/2){ // is in front part
        return 7;
      } else { // is in back part
        return 8;
      }
    } else { // is in top part
      if(v.z<Z.x+(Z.x+Z.y)/2){ // is in front part
        return 6;
      } else { // is in back part
        return 5;
      }
    }
  }
}

void Octree::incrementSubCube(glm::vec3 v){
  if(isInOctree){
    if(!isSubdivided){
      int subCubeIndex = computeInWhichSubCube(v);
      perSubCubeVertexNumbers(subCubeIndex) += 1;
      if(perSubCubeVertexNumbers(subCubeIndex)>numOfPerLeafVertices){
        subdivide();
      }
    } else {
      for(int i = 0; i<8; i++){
        (subTrees.at(i)).incrementSubCube();
      }
    }
  }
}
