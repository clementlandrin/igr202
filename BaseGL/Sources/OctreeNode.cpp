#include "OctreeNode.h"
#include <iostream>

OctreeNode::OctreeNode(){
	isALeaf = false;
	data = new Data();
}
OctreeNode * OctreeNode::buildOctree(int depth, int maxDepth, Data data, unsigned int numOfPerLeafVertices, std::vector<glm::vec3> vertexPositions) {
	OctreeNode * nodePtr = new OctreeNode();
	if(nodePtr->stopCriteria(data, numOfPerLeafVertices, vertexPositions)||depth>maxDepth){
		nodePtr->init(data); // fill children with NULL and affect data
	} else {
		Data childData[8];
		nodePtr->data = nullptr;
		nodePtr->children = new std::vector<OctreeNode*>();
		nodePtr->dataSpatialSplit(data, childData); // 8-split of the bounding
		for(int i = 0; i < 8; i++){
			nodePtr->children->push_back(nodePtr->buildOctree(depth+1, maxDepth,childData[i], numOfPerLeafVertices, vertexPositions));
		}
		nodePtr->setIsALeaf(false);
	}
	return nodePtr;
}

bool OctreeNode::stopCriteria(Data data, unsigned int numOfPerLeafVertices, std::vector<glm::vec3> vertexPositions){
	int numVertices = 0;
	for(int i = 0; i < vertexPositions.size(); i++){
		if(isInCube(data, vertexPositions[i])){
			numVertices = numVertices + 1;
		}
	}
	if(numVertices>numOfPerLeafVertices){
		return false;
	} else {
		return true;
	}
}

bool OctreeNode::isInCube(Data data, glm::vec3 vertexPosition){
	if((vertexPosition.x<data.point.x+data.width && vertexPosition.x>data.point.x)&&
	   (vertexPosition.y<data.point.y+data.height && vertexPosition.y>data.point.y)&&
	   (vertexPosition.z<data.point.z+data.depth && vertexPosition.z>data.point.z)){
		return true;
	} else {
		return false;
	}
}

void OctreeNode::init(Data data){
	*this->data = data;
	this->children = nullptr;
	this->setIsALeaf(true);
}

void OctreeNode::dataSpatialSplit(Data data, Data childData[8]){
	Data tmp = data;
	tmp.point = data.point;
	tmp.height = data.height/2;
	tmp.depth = data.depth/2;
	tmp.width = data.width/2;
	childData[0] = tmp; // bottom left front cube
	tmp.point.z = data.point.z + data.depth/2;
	childData[1] = tmp; // bottom left back cube
	tmp.point.z = data.point.z;
	tmp.point.x = data.point.x + data.width/2;
	childData[2] = tmp; // bottom right front cube
	tmp.point.z = data.point.z + data.depth/2;
	childData[3] = tmp; // bottom right back cube
	tmp.point = data.point;
	tmp.point.y = data.point.y + data.height/2;
	childData[4] = tmp; //top left front cube
	tmp.point.z = data.point.z + data.depth/2;
	childData[5] = tmp; //top left back cube
	tmp.point.z = data.point.z;
	tmp.point.x = data.point.x + data.width/2;
	childData[6] = tmp; //top right front cube
	tmp.point.z = data.point.z + data.depth/2;
	childData[7] = tmp; //top right back cube
}
