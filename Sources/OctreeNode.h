#ifndef OCTREENODE_H
#define OCTREENODE_H

#include "Data.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <vector>

class OctreeNode {
private:
	std::vector<OctreeNode*> * children;
	Data * data;
	bool isALeaf;
	bool stopCriteria(Data data, unsigned int numOfPerLeafVertices, std::vector<glm::vec3> vertexPositions);
	bool isInCube(Data _data, glm::vec3 vertexPosition);
	void init(Data data);
	void dataSpatialSplit(Data data, Data childData[8]);
	void setIsALeaf(bool b){isALeaf = b;};
public:
	OctreeNode();
	static OctreeNode * buildOctree(int depth, int maxDepth, Data _data, unsigned int numOfPerLeafVertices, std::vector<glm::vec3> vertexPositions);
	bool getIsALeaf(){return isALeaf;};
	Data * getData(){return data;};
	std::vector<OctreeNode*> * getChildren(){return children;};
};

#endif // OCTREE_H
