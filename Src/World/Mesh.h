#pragma once

#include "glad/glad.h"
#include "Maths/GlmCommon.h"

#include <vector>
#include <atomic>
#include <cstddef>

template<typename V, typename Derived>
struct Mesh {
public:
	using Vertex = V;

	GLuint VBO, EBO, VAO;
	GLuint indicesNb = 0;
	bool loadedVBO = false, loadedVAO = false;

	friend void swap(Mesh& first, Mesh& second) noexcept {
		using std::swap;
		swap(first.VAO, second.VAO);
		swap(first.VBO, second.VBO);
		swap(first.EBO, second.EBO);
		swap(first.indicesNb, second.indicesNb);
		swap(first.loadedVBO, second.loadedVBO);
		swap(first.loadedVAO, second.loadedVAO);
	}

	void clear() {
		unloadVAOs();
		if (indicesNb != 0 && loadedVBO) {
			glDeleteBuffers(1, &VBO);
			glDeleteBuffers(1, &EBO);
		}
		indicesNb = 0;
		loadedVBO = false;
	}

	Mesh() { }

	~Mesh() {
		clear();
	}

	Mesh(Mesh&& other) noexcept : Mesh{} {
		swap(*this, other);
	}

	Mesh& operator=(Mesh other) noexcept {
		clear();
		swap(*this, other);
		return *this;
	}

	// Kept public so a renderer can drop its VAO without destroying the buffers.
	void unloadVAOs() {
		if (indicesNb != 0 && loadedVAO) {
			glDeleteVertexArrays(1, &VAO);
		}
		loadedVAO = false;
	}

	void loadBuffers(const std::vector<V>& faces, const std::vector<GLuint>& indices) {
		indicesNb = indices.size();
		if (indicesNb != 0) {
			// The VBO stores the vertices
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(V) * faces.size(), faces.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			// The EBO stores the indices
			glGenBuffers(1, &EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
		loadedVBO = true;
	}

	void draw() const {
		if (indicesNb != 0) {
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, indicesNb, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}

	void loadVAOs() {
		if (indicesNb != 0) {
			// Create and bind the VAO
			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);
			// Bind the buffers
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			// Attributes of the VAO
			static_cast<Derived*>(this)->loadAttributes();
			// Unbind all
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			// Unbind the EBO after unbinding the VAO else the EBO will be removed from the VAO
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
		loadedVAO = true;
	}

protected:
	void addFloatVertexAttribPointer(GLuint id, GLint size, std::size_t offset) {
		glEnableVertexAttribArray(id);
		glVertexAttribPointer(id, size, GL_FLOAT, GL_TRUE, sizeof(V), (GLvoid*)offset);
	}

	void addIntVertexAttribPointer(GLuint id, GLint size, std::size_t offset) {
		glEnableVertexAttribArray(id);
		glVertexAttribIPointer(id, size, GL_UNSIGNED_INT, sizeof(V), (GLvoid*)offset);
	}
};

struct DefaultVertex {
	vec3 pos;
	vec2 tex;
	vec3 norm;
	GLuint texID;
	GLfloat ao;
	vec2 light;
};

struct WaterVertex {
	vec3 pos;
	vec3 norm;
};

struct PostProcessingVertex {
	vec2 pos;
	vec2 tex;
};

struct DefaultMesh : Mesh<DefaultVertex, DefaultMesh> {
	void loadAttributes() {
		addFloatVertexAttribPointer(0, 3, offsetof(DefaultVertex, pos));
		addFloatVertexAttribPointer(1, 2, offsetof(DefaultVertex, tex));
		addFloatVertexAttribPointer(2, 3, offsetof(DefaultVertex, norm));
		addIntVertexAttribPointer(3, 1, offsetof(DefaultVertex, texID));
		addFloatVertexAttribPointer(4, 1, offsetof(DefaultVertex, ao));
		addFloatVertexAttribPointer(5, 2, offsetof(DefaultVertex, light));
	}
};

struct WaterMesh : Mesh<WaterVertex, WaterMesh> {
	void loadAttributes() {
		addFloatVertexAttribPointer(0, 3, 0);
		addFloatVertexAttribPointer(2, 3, sizeof(vec3));
	}
};

struct PostProcessingMesh : Mesh<PostProcessingVertex, PostProcessingMesh> {
	void loadAttributes() {
		addFloatVertexAttribPointer(0, 2, 0);
		addFloatVertexAttribPointer(1, 2, sizeof(vec2));
	}
};