#define _USE_MATH_DEFINES

#include "Mesh.h"
#include "OctreeNode.h"
#include "Data.h"

#include <cmath>
#include <algorithm>
#include <iostream>
using namespace std;

Mesh::~Mesh () 
{
	clear ();
}
void Mesh::subdivide()
{
	int index0;
	int index1;
	int index2;

	int newIndex0;
	int newIndex1;
	int newIndex2;

	glm::vec3 pos0;
	glm::vec3 pos1;
	glm::vec3 pos2;

	glm::vec3 newPos0;
	glm::vec3 newPos1;
	glm::vec3 newPos2;

	std::cout<<m_triangleIndices.size()<<std::endl;
	std::vector<glm::uvec3> triangleIndicesCopy = m_triangleIndices;
	
	for(int i = 0; i<m_triangleIndices.size();i++)
	{
			index0 = m_triangleIndices.at(i).x;
			index1 = m_triangleIndices.at(i).y;
			index2 = m_triangleIndices.at(i).z;

			pos0 = m_vertexPositions.at(index0);
			pos1 = m_vertexPositions.at(index1);
			pos2 = m_vertexPositions.at(index2);

			newPos0 = (pos1-pos0)/2.0f + pos0;
			newPos1 = (pos2-pos1)/2.0f + pos1;
			newPos2 = (pos0-pos2)/2.0f + pos2;

			newIndex0 = m_vertexPositions.size();
			newIndex1 = m_vertexPositions.size()+1;
			newIndex2 = m_vertexPositions.size()+2;

			m_vertexPositions.push_back(newPos0);
			m_vertexPositions.push_back(newPos1);
			m_vertexPositions.push_back(newPos2);

			// modify the current triangle
			triangleIndicesCopy.at(i).x = newIndex0;
			triangleIndicesCopy.at(i).y = index1;
			triangleIndicesCopy.at(i).z = newIndex1;

			// add the three other triangles
			triangleIndicesCopy.push_back(glm::vec3(index0,newIndex0,newIndex2));
			triangleIndicesCopy.push_back(glm::vec3(newIndex0,newIndex1,newIndex2));
			triangleIndicesCopy.push_back(glm::vec3(newIndex2,newIndex1,index2));
	}

	m_triangleIndices = triangleIndicesCopy;
	init();
	//push_buffers();
}

void Mesh::computeMinMaxCoordinates()
{
	float maxX = m_vertexPositions.at(0)[0];
	float maxY = m_vertexPositions.at(0)[1];
	float maxZ = m_vertexPositions.at(0)[2];
	float minX = m_vertexPositions.at(0)[0];
	float minY = m_vertexPositions.at(0)[1];
	float minZ = m_vertexPositions.at(0)[2];

	for(int i = 0;i<m_vertexPositions.size();i++)
	{
		if(m_vertexPositions.at(i)[0]>maxX)
		{
			maxX = m_vertexPositions.at(i)[0];
		}

		if(m_vertexPositions.at(i)[1]>maxY)
		{
			maxY = m_vertexPositions.at(i)[1];
		}

		if(m_vertexPositions.at(i)[2]>maxZ)
		{
			maxZ = m_vertexPositions.at(i)[2];
		}

		if(m_vertexPositions.at(i)[0]<minX)
		{
			minX = m_vertexPositions.at(i)[0];
		}

		if(m_vertexPositions.at(i)[1]<minY)
		{
			minY = m_vertexPositions.at(i)[1];
		}

		if(m_vertexPositions.at(i)[2]<minZ)
		{
			minZ = m_vertexPositions.at(i)[2];
		}
	}

		zMin = minZ;
		zMax = maxZ;
		xMin = minX;
		xMax = maxX;
		yMin = minY;
		yMax = maxY;
}

void Mesh::push_buffers()
{
	size_t vertexBufferSize = sizeof (glm::vec3) * m_vertexPositions.size (); // Gather the size of the buffer from the CPU-side vector
	size_t texCoordBufferSize = sizeof (glm::vec2) * m_vertexTexCoords.size ();
	size_t indexBufferSize = sizeof (glm::uvec3) * m_triangleIndices.size ();

	glNamedBufferSubData (m_ibo, 0, indexBufferSize, m_triangleIndices.data ());
	glNamedBufferSubData (m_normalVbo, 0, vertexBufferSize, m_vertexNormals.data ());
	glNamedBufferSubData (m_texCoordVbo, 0, texCoordBufferSize, m_vertexTexCoords.data ());
	glNamedBufferSubData (m_posVbo, 0, vertexBufferSize, m_vertexPositions.data ());
	glNamedBufferSubData (m_tanVbo, 0, vertexBufferSize, m_vertexTangents.data ());
	glNamedBufferSubData (m_biVbo, 0, vertexBufferSize, m_vertexBitangents.data ());
}

void travelTree(std::vector<Data>* datas,OctreeNode * node)
{
	if(!node->getIsALeaf())
	{
		std::vector<OctreeNode*> * children = new std::vector<OctreeNode*>();
		children = node->getChildren();
		for(int i = 0; i<8; i++)
		{
			travelTree(datas, (*children).at(i));
		}
	} 
	else 
	{
		datas->push_back(*node->getData());
	}
}

bool isInCell(Data data, glm::vec3 vertexPos)
{
		if((vertexPos.x>data.point.x && vertexPos.x<data.point.x+data.width)&&
			 (vertexPos.y>data.point.y && vertexPos.y<data.point.y+data.height)&&
			 (vertexPos.z>data.point.z && vertexPos.z<data.point.z+data.depth))
		{
				 return true;
		} 
		else 
		{
			return false;
		}
}

void Mesh::adaptiveSimplify(unsigned int numOfPerLeafVertices)
{
	OctreeNode * octreeNode;

	float maxX = xMax +(xMax-xMin)/100.0;
	float minX = xMin -(xMax-xMin)/100.0;

	float maxY = yMax +(yMax-yMin)/100.0;
	float minY = yMin -(yMax-yMin)/100.0;

	float maxZ = zMax +(zMax-zMin)/100.0;
	float minZ = zMin -(zMax-zMin)/100.0;

	float height = maxY-minY;
	float width = maxX-minX;
	float depth = maxZ-minZ;
	glm::vec3 positionBottomLeftFront = glm::vec3(minX,minY,minZ);
	Data data;
	data.height = height;
	data.width = width;
	data.depth = depth;
	data.point = positionBottomLeftFront;
	octreeNode = OctreeNode::buildOctree(0,16,data, numOfPerLeafVertices, m_vertexPositions);
	// go through octree
	std::vector<Data>* datas = new std::vector<Data>();
	travelTree(datas, octreeNode);
	int dataSize = (*datas).size();

	std::vector<glm::vec3> perCellVertexPositions;
	std::vector<glm::vec3> perCellVertexNormals;
	std::vector<int> perCellVertexNumbers;
	std::vector<int> perVertexCellIndices;
	perVertexCellIndices.resize(m_vertexPositions.size());
	perCellVertexPositions.resize(dataSize, glm::vec3(0.0));
	perCellVertexNormals.resize(dataSize, glm::vec3(0.0));
	perCellVertexNumbers.resize(dataSize, 0);

	for (int i = 0; i<m_vertexPositions.size();i++)
	{
		for(int j = 0; j<dataSize;j++)
		{
			if(isInCell((*datas).at(j),m_vertexPositions[i]))
			{
				perVertexCellIndices.at(i) = j;
				perCellVertexNumbers.at(j) = perCellVertexNumbers.at(j) + 1;
				perCellVertexPositions.at(j) = perCellVertexPositions.at(j) + m_vertexPositions.at(i);
				perCellVertexNormals.at(j) = perCellVertexNormals.at(j) + m_vertexNormals.at(i);
			} 
		}
	}
	for(int j = 0; j<dataSize;j++)
	{
		int n = perCellVertexNumbers.at(j);
		perCellVertexPositions.at(j) = perCellVertexPositions.at(j) *(1.0f/n);
		perCellVertexNormals.at(j) = perCellVertexNormals.at(j)*(1.0f/n);
	}

	for (int i = 0; i<m_triangleIndices.size(); i++)
	{
		int vertexIndex0 = m_triangleIndices.at(i).x;
		int vertexIndex1 = m_triangleIndices.at(i).y;
		int vertexIndex2 = m_triangleIndices.at(i).z;
		int cell0 = perVertexCellIndices.at(vertexIndex0);
		int cell1 = perVertexCellIndices.at(vertexIndex1);
		int cell2 = perVertexCellIndices.at(vertexIndex2);

		m_vertexPositions.at(vertexIndex0) = perCellVertexPositions.at(cell0);
		m_vertexPositions.at(vertexIndex1) = perCellVertexPositions.at(cell1);
		m_vertexPositions.at(vertexIndex2) = perCellVertexPositions.at(cell2);

		m_vertexNormals.at(vertexIndex0) = perCellVertexNormals.at(cell0);
		m_vertexNormals.at(vertexIndex1) = perCellVertexNormals.at(cell1);
		m_vertexNormals.at(vertexIndex2) = perCellVertexNormals.at(cell2);
	}

	push_buffers();
}

void Mesh::simplify(unsigned int resolution)
{
	std::vector<int> perVertexCellIndices;
	std::vector<glm::vec3> perCellVertexPositions;
	std::vector<glm::vec3> perCellVertexNormals;
	std::vector<int> perCellVertexNumbers;
	perVertexCellIndices.resize(m_vertexPositions.size());

	perCellVertexPositions.resize(resolution*resolution*resolution, glm::vec3(0.0));
	perCellVertexNormals.resize(resolution*resolution*resolution, glm::vec3(0.0));
	perCellVertexNumbers.resize(resolution*resolution*resolution, 0);

	float maxX = xMax +(xMax-xMin)/100.0;
	float minX = xMin -(xMax-xMin)/100.0;

	float maxY = yMax +(yMax-yMin)/100.0;
	float minY = yMin -(yMax-yMin)/100.0;

	float maxZ = zMax +(zMax-zMin)/100.0;
	float minZ = zMin -(zMax-zMin)/100.0;

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 h = glm::vec3((xMax-xMin)/(resolution-1),(yMax-yMin)/(resolution-1),(zMax-zMin)/(resolution-1));

	for (int i = 0; i<m_vertexPositions.size(); i++)
	{
		position = m_vertexPositions.at(i);
		normal = m_vertexNormals.at(i);
		int indexX = int((position.x-xMin)/h.x);
		int indexY = int((position.y-yMin)/h.y);
		int indexZ = int((position.z-zMin)/h.z);
		int cellIndex = indexX*resolution*resolution+indexY*resolution+indexZ;
		perCellVertexPositions.at(cellIndex) = perCellVertexPositions.at(cellIndex) + position;
		perCellVertexNormals.at(cellIndex) = perCellVertexNormals.at(cellIndex)+normal;
		perVertexCellIndices.at(i) = cellIndex;
	}

	for (int i = 0; i<perVertexCellIndices.size(); i++)
	{
		int cellIndex = perVertexCellIndices.at(i);
		perCellVertexNumbers.at(cellIndex) = perCellVertexNumbers.at(cellIndex) + 1;
	}

	for (int i = 0; i<perCellVertexPositions.size(); i++)
	{
		int n = perCellVertexNumbers.at(i);
		perCellVertexPositions.at(i) = perCellVertexPositions.at(i)*(1.0f/n);
		perCellVertexNormals.at(i) = normalize(perCellVertexNormals.at(i));
	}

	for (int i = 0; i<m_triangleIndices.size(); i++)
	{
		int vertexIndex0 = m_triangleIndices.at(i).x;
		int vertexIndex1 = m_triangleIndices.at(i).y;
		int vertexIndex2 = m_triangleIndices.at(i).z;
		int cell0 = perVertexCellIndices.at(vertexIndex0);
		int cell1 = perVertexCellIndices.at(vertexIndex1);
		int cell2 = perVertexCellIndices.at(vertexIndex2);

		m_vertexPositions.at(vertexIndex0) = perCellVertexPositions.at(cell0);
		m_vertexPositions.at(vertexIndex1) = perCellVertexPositions.at(cell1);
		m_vertexPositions.at(vertexIndex2) = perCellVertexPositions.at(cell2);

		m_vertexNormals.at(vertexIndex0) = perCellVertexNormals.at(cell0);
		m_vertexNormals.at(vertexIndex1) = perCellVertexNormals.at(cell1);
		m_vertexNormals.at(vertexIndex2) = perCellVertexNormals.at(cell2);
	}

	push_buffers();
}

void Mesh::computePlanarParameterization()
{
	computeMinMaxCoordinates();
	m_vertexTexCoords.resize(m_vertexPositions.size());

	for(int i = 0; i<m_vertexPositions.size();i++)
	{
		m_vertexTexCoords.at(i) = glm::vec2((m_vertexPositions.at(i)[0]-xMin)/(xMax-xMin),
																	      (m_vertexPositions.at(i)[1]-yMin)/(yMax-yMin));
	}
}

void Mesh::laplacianFilter(float alpha, bool cotangentWeights)
{
	glm::vec3 p0 ;
	glm::vec3 p1 ;
	glm::vec3 p2 ;
	int index0;
	int index1;
	int index2;
	float angle0;
	float angle1;
	float angle2;
	float cotan0;
	float cotan1;
	float cotan2;

	std::vector<std::vector<glm::vec3>> vertexDelta;
	vertexDelta.resize(m_vertexPositions.size());

	std::vector<std::vector<float>> vertexCotangent;
	vertexCotangent.resize(m_vertexPositions.size());

	for (int i = 0; i<m_triangleIndices.size(); i++)
	{
		index0 = m_triangleIndices.at(i)[0];
		index1 = m_triangleIndices.at(i)[1];
		index2 = m_triangleIndices.at(i)[2];
		p0 = m_vertexPositions.at(index0);
		p1 = m_vertexPositions.at(index1);
		p2 = m_vertexPositions.at(index2);

		angle0 = std::acos(dot(p1-p0,p2-p0)/(length(p1-p0)*length(p2-p0)));
		angle1 = std::acos(dot(p0-p1,p2-p1)/(length(p0-p1)*length(p2-p1)));
		angle2 = std::acos(dot(p0-p2,p1-p2)/(length(p0-p2)*length(p1-p2)));
		cotan0 = min(abs(1/tan(angle0)),10.0f);
		cotan1 = min(abs(1/tan(angle1)),10.0f);
		cotan2 = min(abs(1/tan(angle2)),10.0f);

		(vertexCotangent.at(index0)).push_back(cotan2);
		(vertexDelta.at(index0)).push_back(p1-p0);
		(vertexCotangent.at(index0)).push_back(cotan1);
		(vertexDelta.at(index0)).push_back(p2-p0);

		(vertexCotangent.at(index1)).push_back(cotan2);
		(vertexDelta.at(index1)).push_back(p0-p1);
		(vertexCotangent.at(index1)).push_back(cotan0);
		(vertexDelta.at(index1)).push_back(p2-p1);

		(vertexCotangent.at(index2)).push_back(cotan1);
		(vertexDelta.at(index2)).push_back(p0-p2);
		(vertexCotangent.at(index2)).push_back(cotan0);
		(vertexDelta.at(index2)).push_back(p1-p2);
	}

	for (int i = 0; i<m_vertexPositions.size();i++)
	{
		float weight = 0.0;
		for(int j = 0; j<(vertexCotangent.at(i)).size(); j++)
		{
			if(cotangentWeights)
			{
				weight = weight + (vertexCotangent.at(i)).at(j);
			} 
			else 
			{
				weight = weight + 1.0;
			}
		}

		glm::vec3 delta = glm::vec3(0.0);
		float kWeight = 1.0f;
		for(int k = 0; k<(vertexDelta.at(i)).size(); k++)
		{
			if(cotangentWeights)
			{
				kWeight = (vertexCotangent.at(i)).at(k);
			}

			delta = delta + alpha * kWeight*(vertexDelta.at(i)).at(k);
		}

		delta = delta / weight;
		m_vertexPositions.at(i) = m_vertexPositions.at(i) + delta;
	}

	recomputePerVertexNormals(true);
	push_buffers();
}

void Mesh::computeBoundingSphere (glm::vec3 & center, float & radius) const 
{
	center = glm::vec3 (0.0);
	radius = 0.f;
	for (const auto & p : m_vertexPositions)
		center += p;
	center /= m_vertexPositions.size ();

	for (const auto & p : m_vertexPositions)
		radius = std::max (radius, distance (center, p));
}

void Mesh::recomputePerVertexNormals (bool angleBased) 
{
	computePlanarParameterization();
	m_vertexNormals.clear ();
	m_vertexTangents.clear ();
	m_vertexBitangents.clear ();

	glm::vec3 p0 ;
	glm::vec3 p1 ;
	glm::vec3 p2 ;
	int index0;
	int index1;
	int index2;
	float angle0;
	float angle1;
	float angle2;
	glm::vec3 normal;

	// Tangent and bitangent variables

	glm::vec3 edge1;
	glm::vec3 edge2;

	glm::vec2 uv1;
	glm::vec2 uv2;
	glm::vec2 uv3;

	glm::vec2 deltaUV1;
	glm::vec2 deltaUV2;

	int vertexIndex1;
	int vertexIndex2;
	int vertexIndex3;

	glm::vec3 vertexPos1;
	glm::vec3 vertexPos2;
	glm::vec3 vertexPos3;

	float f;

	glm::vec3 tangent;
	glm::vec3 bitangent;

	m_vertexNormals.resize(m_vertexPositions.size(), glm::vec3(0.0,0.0,0.0));
	m_vertexTangents.resize(m_vertexPositions.size(), glm::vec3(0.0,0.0,0.0));
	m_vertexBitangents.resize(m_vertexPositions.size(), glm::vec3(0.0,0.0,0.0));

	for (int i = 0; i < m_triangleIndices.size() ; i++)
	{
		// normal computation
		index0 = m_triangleIndices.at(i)[0];
		p0 = m_vertexPositions.at(index0);
		index1 = m_triangleIndices.at(i)[1];
		p1 = m_vertexPositions.at(index1);
		index2 = m_triangleIndices.at(i)[2];
		p2 = m_vertexPositions.at(index2);
		normal = normalize(cross(p1-p0,p2-p0));
		angle0 = 1.0;
		angle1 = 1.0;
		angle2 = 1.0;

		if(angleBased)
		{
			angle0 = std::acos(dot(p1-p0,p2-p0)/(length(p1-p0)*length(p2-p0)));
			angle1 = std::acos(dot(p0-p1,p2-p1)/(length(p0-p1)*length(p2-p1)));
			angle2 = std::acos(dot(p0-p2,p1-p2)/(length(p0-p2)*length(p1-p2)));
		}

		m_vertexNormals.at(index0) = m_vertexNormals.at(index0) + angle0*normal;
		m_vertexNormals.at(index1) = m_vertexNormals.at(index1) + angle1*normal;
		m_vertexNormals.at(index2) = m_vertexNormals.at(index2) + angle2*normal;

		// tangent and bitangent computation

		edge1 = p1 - p0;
		edge2 = p2 - p0;

		uv1 = m_vertexTexCoords.at(index0);
		uv2 = m_vertexTexCoords.at(index1);
		uv3 = m_vertexTexCoords.at(index2);

		deltaUV1 = uv2 - uv1;
		deltaUV2 = uv3 - uv1;

		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * f;

		//tangent = glm::normalize(tangent);
		m_vertexTangents.at(index0) = m_vertexTangents.at(index0) + angle0*tangent;
		m_vertexTangents.at(index1) = m_vertexTangents.at(index1) + angle1*tangent;
		m_vertexTangents.at(index2) = m_vertexTangents.at(index2) + angle2*tangent;

		bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * f;

		//bitangent = glm::normalize(bitangent);
		m_vertexBitangents.at(index0) = m_vertexBitangents.at(index0) + angle0*bitangent;
		m_vertexBitangents.at(index1) = m_vertexBitangents.at(index1) + angle1*bitangent;
		m_vertexBitangents.at(index2) = m_vertexBitangents.at(index2) + angle2*bitangent;
	}

	for(int i = 0; i<m_vertexPositions.size();i++)
	{
		m_vertexNormals.at(i) = normalize(m_vertexNormals.at(i));
		glm::vec3 n = m_vertexNormals.at(i);
		m_vertexTangents.at(i) = normalize(m_vertexTangents.at(i));
		glm::vec3 t = normalize(m_vertexTangents.at(i));
		m_vertexBitangents.at(i) = normalize(m_vertexBitangents.at(i));
		m_vertexTangents.at(i) = normalize(t - n * glm::dot(n,t));
	}
}

void Mesh::init () 
{
	computePlanarParameterization();
	recomputePerVertexNormals (true);
	glCreateBuffers (1, &m_posVbo); // Generate a GPU buffer to store the positions of the vertices
	size_t vertexBufferSize = sizeof (glm::vec3) * m_vertexPositions.size (); // Gather the size of the buffer from the CPU-side vector
	glNamedBufferStorage (m_posVbo, vertexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT); // Create a data store on the GPU
	glNamedBufferSubData (m_posVbo, 0, vertexBufferSize, m_vertexPositions.data ()); // Fill the data store from a CPU array

	glCreateBuffers (1, &m_normalVbo); // Same for normal
	glNamedBufferStorage (m_normalVbo, vertexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData (m_normalVbo, 0, vertexBufferSize, m_vertexNormals.data ());

	glCreateBuffers (1, &m_texCoordVbo); // Same for texture coordinates
	size_t texCoordBufferSize = sizeof (glm::vec2) * m_vertexTexCoords.size ();
	glNamedBufferStorage (m_texCoordVbo, texCoordBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData (m_texCoordVbo, 0, texCoordBufferSize, m_vertexTexCoords.data ());

	glCreateBuffers (1, &m_ibo); // Same for the index buffer, that stores the list of indices of the triangles forming the mesh
	size_t indexBufferSize = sizeof (glm::uvec3) * m_triangleIndices.size ();
	glNamedBufferStorage (m_ibo, indexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData (m_ibo, 0, indexBufferSize, m_triangleIndices.data ());

	glCreateBuffers (1, &m_tanVbo); // Same for the tangent buffer
	glNamedBufferStorage (m_tanVbo, vertexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData (m_tanVbo, 0, vertexBufferSize, m_vertexTangents.data ());

	glCreateBuffers (1, &m_biVbo); // Same for the bitangent buffer
	glNamedBufferStorage (m_biVbo, vertexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData (m_biVbo, 0, vertexBufferSize, m_vertexBitangents.data ());

	glCreateVertexArrays (1, &m_vao); // Create a single handle that joins together attributes (vertex positions, normals) and connectivity (triangles indices)
	glBindVertexArray (m_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, m_posVbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, m_normalVbo);
	glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
	glEnableVertexAttribArray (2);
	glBindBuffer (GL_ARRAY_BUFFER, m_texCoordVbo);
	glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat), 0);
	glEnableVertexAttribArray (3);
	glBindBuffer (GL_ARRAY_BUFFER, m_tanVbo);
	glVertexAttribPointer (3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
	glEnableVertexAttribArray (4);
	glBindBuffer (GL_ARRAY_BUFFER, m_biVbo);
	glVertexAttribPointer (4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBindVertexArray (0); // Desactive the VAO just created. Will be activated at rendering time.
}

void Mesh::render () 
{
	glBindVertexArray (m_vao); // Activate the VAO storing geometry data
	glDrawElements (GL_TRIANGLES, static_cast<GLsizei> (m_triangleIndices.size () * 3), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
}

void Mesh::clear () 
{
	m_vertexPositions.clear ();
	m_vertexNormals.clear ();
	m_vertexTexCoords.clear ();
	m_triangleIndices.clear ();
	m_vertexTangents.clear ();
	m_vertexBitangents.clear ();

	if (m_vao) 
	{
		glDeleteVertexArrays (1, &m_vao);
		m_vao = 0;
	}

	if(m_posVbo)
	{
		glDeleteBuffers (1, &m_posVbo);
		m_posVbo = 0;
	}

	if (m_normalVbo)
	{
		glDeleteBuffers (1, &m_normalVbo);
		m_normalVbo = 0;
	}

	if (m_texCoordVbo) 
	{
		glDeleteBuffers (1, &m_texCoordVbo);
		m_texCoordVbo = 0;
	}

	if (m_ibo) 
	{
		glDeleteBuffers (1, &m_ibo);
		m_ibo = 0;
	}

	if (m_tanVbo) 
	{
		glDeleteBuffers (1, &m_tanVbo);
	}

	if (m_biVbo)
	{
		glDeleteBuffers (1, &m_biVbo);
	}
}
