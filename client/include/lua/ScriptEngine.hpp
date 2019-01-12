/*
 * =====================================================================================
 *
 *       Filename:  ScriptEngine.hpp
 *
 *    Description:
 *
 *        Created:  20/12/2018 00:41:50
 *
 *         Author:  Quentin Bazin, <quent42340@gmail.com>
 *
 * =====================================================================================
 */
#ifndef SCRIPTENGINE_HPP_
#define SCRIPTENGINE_HPP_

#include <glm/vec3.hpp>

#include <sol.hpp>

class Chunk;
class Player;
class World;

class ScriptEngine {
	public:
		void init();

		sol::state &lua() { return m_lua; }

		static ScriptEngine &getInstance() { return *s_instance; }
		static void setInstance(ScriptEngine *instance) { s_instance = instance; }

	private:
		void initUsertypes();

		static ScriptEngine *s_instance;

		sol::state m_lua;
};

#endif // SCRIPTENGINE_HPP_
