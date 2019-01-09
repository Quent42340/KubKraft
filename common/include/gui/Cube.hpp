/*
 * =====================================================================================
 *
 *       Filename:  Cube.hpp
 *
 *    Description:
 *
 *        Created:  26/06/2018 00:01:26
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#ifndef CUBE_HPP_
#define CUBE_HPP_

#include <gk/gl/IDrawable.hpp>
#include <gk/gl/Transformable.hpp>
#include <gk/gl/VertexBuffer.hpp>

class Block;

class Cube : public gk::IDrawable, public gk::Transformable {
	public:
		Cube(float size = 1.0f);

		void updateVertexBuffer(const Block &block) const;

	private:
		void draw(gk::RenderTarget &target, gk::RenderStates states) const override;

		float m_size = 1.0f;

		const gk::Texture &m_texture;

		gk::VertexBuffer m_vbo;

		glm::mat4 m_modelMatrix{1};
};

#endif // CUBE_HPP_
