#pragma once

#include "glad/glad.h"
#include "GlmCommon.h"

#include <vector>

template<typename T>
struct Mesh
{
	using Vertex = T;

	GLuint VBO, EBO, VAO;
	GLuint indicesNb = 0;

	~Mesh() {
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

	void unloadVAOs() {
		if (indicesNb != 0) {
			glDeleteVertexArrays(1, &VAO);
		}
	}

	void loadBuffers(const std::vector<T>& faces, const std::vector<GLuint>& indices) {
		indicesNb = indices.size();
		if (indicesNb != 0) {
			// The VBO stores the vertices
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(T) * faces.size(), faces.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			// The EBO stores the indices
			glGenBuffers(1, &EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}

	void draw() const {
		if (indicesNb != 0) {
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, indicesNb, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}
};

namespace {
	struct DefaultVertex {
		vec3 pos;
		vec2 tex;
		vec3 norm;
		GLuint texID;
	};

	struct WaterVertex {
		vec3 pos;
		vec2 tex;
		vec3 norm;
	};
}

struct DefaultMesh : Mesh<DefaultVertex>
{
	void loadVAOs() {
		if (indicesNb != 0) {
			// Create and bind the VAO
			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);
			// Bind the buffers
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			// Attributes of the VAO
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(vec3)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(vec3) + sizeof(vec2)));
			glEnableVertexAttribArray(3);
			glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (GLvoid*)(sizeof(vec3) + sizeof(vec2) + sizeof(vec3)));
			// Unbind all
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			// Unbind the EBO after unbinding the VAO else the EBO will be removed from the VAO
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}
};

struct WaterMesh : Mesh<WaterVertex> {
	void loadVAOs() {
		if (indicesNb != 0) {
			// Create and bind the VAO
			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);
			// Bind the buffers
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			// Attributes of the VAO
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(vec3)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(vec3) + sizeof(vec2)));
			// Unbind all
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			// Unbind the EBO after unbinding the VAO else the EBO will be removed from the VAO
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}
};