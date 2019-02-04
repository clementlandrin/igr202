#include "OctreeNode.h"

OctreeNode OctreeNode::buildOctree(Data data, unsigned int numOfPerLeafVertices, std::vector<glm::vec3> vertexPositions) {
	OctreeNode node;
	if(stopCriteria(Data data, numOfPerLeafVertices, std::vector<glm::vec3> vertexPositions)){
		init(node, data); // fill children with NULL and affect data
	} else {
		Data childData[8];
		dataSpatialSplit(data, childData); // 8-split of the bounding
						   // cube and partitioning of data
		for(int i = 0; i < 8; i++){
			node.children[i] = buildOctree(childData[i], std::vector<glm::vec3> vertexPositions);
			node.data= NULL;
		}
	}
	return node;
}

bool OctreeNode::stopCriteria(Data data, unsigned int numOfPerLeafVertices, std::vector<glm::vec3> vertexPositions){
	numVertices = 0;
	for(int i = 0; i < vertexPositions.size; i++){
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

void OctreeNode::init(Node node, Data data){
	node.children = NULL;
	node.data = data;
}

void dataSpatialSplit(Data data, Data childData[8]){
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
