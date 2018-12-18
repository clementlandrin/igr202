#define _USE_MATH_DEFINES

#include "Mesh.h"

#include <cmath>
#include <algorithm>
#include <iostream>
using namespace std;

Mesh::~Mesh () {
	clear ();
}

void Mesh::computePlanarParameterization(){
	float maxX = m_vertexPositions.at(0)[0];
	float maxY = m_vertexPositions.at(0)[1];
	float maxZ = m_vertexPositions.at(0)[2];
	float minX = m_vertexPositions.at(0)[0];
	float minY = m_vertexPositions.at(0)[1];
	float minZ = m_vertexPositions.at(0)[2];

	for(int i = 0;i<m_vertexPositions.size();i++){
		if(m_vertexPositions.at(i)[0]>maxX){
			maxX = m_vertexPositions.at(i)[0];
		}
		if(m_vertexPositions.at(i)[1]>maxY){
			maxY = m_vertexPositions.at(i)[1];
		}
		if(m_vertexPositions.at(i)[2]>maxZ){
			maxZ = m_vertexPositions.at(i)[2];
		}
		if(m_vertexPositions.at(i)[0]<minX){
			minX = m_vertexPositions.at(i)[0];
		}
		if(m_vertexPositions.at(i)[1]<minY){
			minY = m_vertexPositions.at(i)[1];
		}
		if(m_vertexPositions.at(i)[2]<minZ){
			minZ = m_vertexPositions.at(i)[2];
		}
	}

	m_vertexTexCoords.resize(m_vertexPositions.size());
	for(int i = 0; i<m_vertexPositions.size();i++){
		m_vertexTexCoords.at(i) = glm::vec2((m_vertexPositions.at(i)[0]-minX)/(maxX-minX),
																	      (m_vertexPositions.at(i)[1]-minY)/(maxY-minY));
	}

	zMin = minZ;
	zMax = maxZ;
}

void Mesh::computeBoundingSphere (glm::vec3 & center, float & radius) const {
	center = glm::vec3 (0.0);
	radius = 0.f;
	for (const auto & p : m_vertexPositions)
		center += p;
	center /= m_vertexPositions.size ();
	for (const auto & p : m_vertexPositions)
		radius = std::max (radius, distance (center, p));
}

void Mesh::recomputePerVertexNormals (bool angleBased) {
	m_vertexNormals.clear ();
	// Change the following code to compute a proper per-vertex normal
	glm::vec3 p0 ;
	glm::vec3 p1 ;
	glm::vec3 p2 ;
	int index0;
	int index1;
	int index2;
	float angle0;
	float angle1;
	float angle2;
	m_vertexNormals.resize(m_vertexPositions.size(), glm::vec3(1.0,1.0,1.0));
	glm::vec3 normal;
	for (int i = 0; i < m_triangleIndices.size() ; i++){
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
		if(angleBased){
			angle0 = std::acos(dot(p1-p0,p2-p0)/(length(p1-p0)*length(p2-p0)));
			angle1 = std::acos(dot(p0-p1,p2-p1)/(length(p0-p1)*length(p2-p1)));
			angle2 = std::acos(dot(p0-p2,p1-p2)/(length(p0-p2)*length(p1-p2)));
		}
		m_vertexNormals.at(index0) = m_vertexNormals.at(index0) + angle0*normal;
		m_vertexNormals.at(index1) = m_vertexNormals.at(index1) + angle1*normal;
		m_vertexNormals.at(index2) = m_vertexNormals.at(index2) + angle2*normal;
	}

	for(int i = 0; i<m_vertexPositions.size();i++){
		m_vertexNormals.at(i) = normalize(m_vertexNormals.at(i));
	}
}

void Mesh::init () {
	computePlanarParameterization();
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
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBindVertexArray (0); // Desactive the VAO just created. Will be activated at rendering time.
}

void Mesh::render () {
	glBindVertexArray (m_vao); // Activate the VAO storing geometry data
	glDrawElements (GL_TRIANGLES, static_cast<GLsizei> (m_triangleIndices.size () * 3), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
}

void Mesh::clear () {
	m_vertexPositions.clear ();
	m_vertexNormals.clear ();
	m_vertexTexCoords.clear ();
	m_triangleIndices.clear ();
	if (m_vao) {
		glDeleteVertexArrays (1, &m_vao);
		m_vao = 0;
	}
	if(m_posVbo) {
		glDeleteBuffers (1, &m_posVbo);
		m_posVbo = 0;
	}
	if (m_normalVbo) {
		glDeleteBuffers (1, &m_normalVbo);
		m_normalVbo = 0;
	}
	if (m_texCoordVbo) {
		glDeleteBuffers (1, &m_texCoordVbo);
		m_texCoordVbo = 0;
	}
	if (m_ibo) {
		glDeleteBuffers (1, &m_ibo);
		m_ibo = 0;
	}
}
