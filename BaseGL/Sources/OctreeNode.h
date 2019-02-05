#ifndef OCTREENODE_H
#define OCTREENODE_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <vector>
#include "Cell.h"

class OctreeNode {
private:
	OctreeNode children[8];
	Data data;
	bool stopCriteria(unsigned int numOfPerLeafVertices, std::vector<glm::vec3> vertexPositions);
	bool isInCube(Data _data, glm::vec3 vertexPosition);
public:
	OctreeNode buildOctree(Data _data, unsigned int numOfPerLeafVertices, std::vector<glm::vec3> vertexPositions);
};

#endif // OCTREE_H
