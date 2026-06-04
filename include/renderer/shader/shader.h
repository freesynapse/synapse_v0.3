#ifndef __SHADER_H
#define __SHADER_H

#include <string>
#include <unordered_map>
#include <filesystem>

#ifndef GLAD_INCLUDED
#include "glapi.h"
#endif
#include <glm/glm.hpp>

#include "renderer/shader/shader_types.h"


//
class shader_t
{
public:
	shader_t() {}
	shader_t(const std::string& _shader_file_path);
	shader_t(const std::string& _shader_name, const std::string& _file_path);
	~shader_t() = default;

	void destroy();

	// removing copy and move semantics, so that Shader[N] can be used safely
	shader_t(const shader_t&) = delete;
    shader_t& operator=(const shader_t&) = delete;
    shader_t(shader_t&&) = default;
    shader_t& operator=(shader_t&&) = default;
	
	void reload();
	bool file_has_changed();

	void load_from_file();
	void load_from_source(const std::string &_name, const std::string &_src);

	// new uniform retrieval process
	void reflect_uniforms();
	GLint get_uniform_location(const char *_name);
		
	
protected:
	std::unordered_map<GLenum, std::string> preprocess(const std::string& _source);
	int compile_shader();

public:
	void enable();
	void disable();
	void print_uniforms();

	// accessors -- more below
	// GLint get_uniform_location(const std::string& _uniform_name);
	GLuint get_id() { return m_opengl_id; }
	void set_id(GLuint _program_id) { m_opengl_id = _program_id; }
	const std::string &get_name() { return m_shader_name; }
	bool is_active() { return m_is_active; }
	void set_asset_path(const std::string &_path);
	const std::string &get_asset_path() { return m_asset_path; }
	
private:
	std::string m_shader_name = "";
	std::string m_asset_path = "";
	std::string m_raw_src = "";
	std::unordered_map<GLenum, std::string> m_shader_src;
	// std::unordered_map<std::string, GLint> m_uniforms_map;
	bool m_is_active = false;

	GLuint m_opengl_id = 0;

	uniform_cache_t m_uniform_cache[SYN_MAX_SHADER_UNIFORMS];
	uint32_t m_uniform_count;

	std::filesystem::file_time_type m_last_write_time;


private:
	// accessors -- continued
	void set_uniform_1i(const GLint& _location, const int& _i);
	void set_uniform_1f(const GLint& _location, const float& _f);
	void set_uniform_2iv(const GLint& _location, const glm::ivec2& _v);
	void set_uniform_2fv(const GLint& _location, const glm::vec2& _v);
	void set_uniform_3fv(const GLint& _location, const glm::vec3& _v);
	void set_uniform_4fv(const GLint& _location, const glm::vec4& _v);
	void set_matrix_2fv(const GLint& _location, const glm::mat2& _mat);
	void set_matrix_3fv(const GLint& _location, const glm::mat3& _mat);
	void set_matrix_4fv(const GLint& _location, const glm::mat4& _mat);

public:
	void set_uniform_1i(const char *_name, const int& _i) { set_uniform_1i(get_uniform_location(_name), _i); }
	void set_uniform_1f(const char *_name, const float& _f) { set_uniform_1f(get_uniform_location(_name), _f); }
	void set_uniform_2iv(const char *_name, const glm::ivec2& _v) { set_uniform_2iv(get_uniform_location(_name), _v); }
	void set_uniform_2fv(const char *_name, const glm::vec2& _v) { set_uniform_2fv(get_uniform_location(_name), _v); }
	void set_uniform_3fv(const char *_name, const glm::vec3& _v) { set_uniform_3fv(get_uniform_location(_name), _v); }
	void set_uniform_4fv(const char *_name, const glm::vec4& _v) { set_uniform_4fv(get_uniform_location(_name), _v); }
	void set_matrix_2fv(const char *_name, const glm::mat2& _mat) { set_matrix_2fv(get_uniform_location(_name), _mat); }
	void set_matrix_3fv(const char *_name, const glm::mat3& _mat) { set_matrix_3fv(get_uniform_location(_name), _mat); }
	void set_matrix_4fv(const char *_name, const glm::mat4& _mat) { set_matrix_4fv(get_uniform_location(_name), _mat); }

};


#endif // __SHADER_H
