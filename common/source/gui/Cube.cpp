/*
 * =====================================================================================
 *
 *       Filename:  Cube.cpp
 *
 *    Description:
 *
 *        Created:  26/06/2018 00:02:19
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#include <glm/gtc/matrix_transform.hpp>

#include <gk/graphics/Color.hpp>
#include <gk/gl/Vertex.hpp>
#include <gk/resource/ResourceHandler.hpp>

#include "Block.hpp"
#include "Config.hpp"
#include "Cube.hpp"

Cube::Cube(float size) : m_texture(gk::ResourceHandler::getInstance().get<gk::Texture>("texture-blocks")) {
	m_size = size;

	// FIXME: Using Transform may be better here
	m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(-180.0f), glm::vec3{0, 0, 1});
	m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(-30.0f), glm::vec3{1, 0, 0});
	m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(-45.0f), glm::vec3{0, 1, 0});
}

void Cube::updateVertexBuffer(const Block &block) const {
	gk::Vertex vertices[6 * 4] = {
		// Left
		{{0, 0, 0, 2}},
		{{0, 0, m_size, 2}},
		{{0, m_size, m_size, 2}},
		{{0, m_size, 0, 2}},

		// Right
		{{m_size, 0, m_size, -1}},
		{{m_size, 0, 0, -1}},
		{{m_size, m_size, 0, -1}},
		{{m_size, m_size, m_size, -1}},

		// Bottom
		{{0, 0, 0, -1}},
		{{m_size, 0, 0, -1}},
		{{m_size, 0, m_size, -1}},
		{{0, 0, m_size, -1}},

		// Top
		{{m_size, m_size, 0, 3}},
		{{0, m_size, 0, 3}},
		{{0, m_size, m_size, 3}},
		{{m_size, m_size, m_size, 3}},

		// Front
		{{m_size, 0, 0, 4}},
		{{0, 0, 0, 4}},
		{{0, m_size, 0, 4}},
		{{m_size, m_size, 0, 4}},

		// Back
		{{0, 0, m_size, -1}},
		{{m_size, 0, m_size, -1}},
		{{m_size, m_size, m_size, -1}},
		{{0, m_size, m_size, -1}},
	};

	for (u8 i = 0 ; i < 6 ; ++i) {
		const glm::vec4 &blockTexCoords = block.getTexCoords(i, 0);
		float faceTexCoords[2 * 4] = {
			blockTexCoords.x, blockTexCoords.w,
			blockTexCoords.z, blockTexCoords.w,
			blockTexCoords.z, blockTexCoords.y,
			blockTexCoords.x, blockTexCoords.y
		};
		for(u8 j = 0 ; j < 4 ; j++) {
			vertices[j + i * 4].texCoord[0] = faceTexCoords[j * 2];
			vertices[j + i * 4].texCoord[1] = faceTexCoords[j * 2 + 1];

			vertices[j + i * 4].coord3d[0] = vertices[j + i * 4].coord3d[0] * block.boundingBox().width  + block.boundingBox().x;
			vertices[j + i * 4].coord3d[1] = vertices[j + i * 4].coord3d[1] * block.boundingBox().height + block.boundingBox().y;
			vertices[j + i * 4].coord3d[2] = vertices[j + i * 4].coord3d[2] * block.boundingBox().depth  + block.boundingBox().z;
		}
	}

	gk::Color color = gk::Color::White;
	for (u8 i = 0 ; i < 6 * 4 ; ++i) {
		vertices[i].color[0] = color.r;
		vertices[i].color[1] = color.g;
		vertices[i].color[2] = color.b;
		vertices[i].color[3] = color.a;
	}

	gk::VertexBuffer::bind(&m_vbo);
	m_vbo.setData(sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	gk::VertexBuffer::bind(nullptr);
}

void Cube::draw(gk::RenderTarget &target, gk::RenderStates states) const {
	states.transform *= m_modelMatrix;
	states.transform *= getTransform();

	states.viewMatrix = gk::Transform::Identity;

	states.projectionMatrix = glm::ortho(0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, -40.0f, DIST_FAR);

	states.texture = &m_texture;
	states.vertexAttributes = gk::VertexAttribute::Only2d;

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	target.draw(m_vbo, GL_QUADS, 4 * 0, 4, states);
	// target.draw(m_vbo, GL_QUADS, 4 * 1, 4, states);
	// target.draw(m_vbo, GL_QUADS, 4 * 2, 4, states);
	target.draw(m_vbo, GL_QUADS, 4 * 3, 4, states);
	target.draw(m_vbo, GL_QUADS, 4 * 4, 4, states);
	// target.draw(m_vbo, GL_QUADS, 4 * 5, 4, states);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

}

