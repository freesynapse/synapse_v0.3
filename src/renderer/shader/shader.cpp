
#include <map>
#include <sstream>
#include <array>
#include <string.h>

#include "external/glad/glad.h"

#include "renderer/shader/shader.h"
#include "utils/log.h"
#include "utils/file_io_handler.h"

// 
static GLenum shader_type_from_str(const std::string& _type)
{
	if (_type == "VERTEX" || _type == "VERTEX_SHADER" || _type == "vertex")		
		return GL_VERTEX_SHADER;
	if (_type == "FRAGMENT" || _type == "FRAGMENT_SHADER" || _type == "fragment" || _type == "pixel")	
		return GL_FRAGMENT_SHADER;
	if (_type == "GEOMETRY" || _type == "GEOMETRY_SHADER" || _type == "geometry")
		return GL_GEOMETRY_SHADER;
	return GL_NONE;
}

// 
static std::string shader_str_from_type(GLenum _type)
{
	std::string ret;
	switch (_type)
	{
		case GL_VERTEX_SHADER: 		ret = "VERTEX_SHADER"; 		break;
		case GL_FRAGMENT_SHADER:	ret = "FRAGMENT_SHADER";	break;
		case GL_GEOMETRY_SHADER:	ret = "GEOMETRY_SHADER";	break;
		default:					ret = "GL_NONE";			break;
	};

	return ret;
}

// 
static std::string g_str;
inline const char* leading_blank_spaces(int _line_num, int _error_code=0)
{
	/* error code 1 = error, 2 = warning */
	
	// N.B:: max 10000 lines of shader code permitted
	// get number of digits in _line_num
	int numChars = _line_num > 0 ? (int)log10((double)_line_num) + 1 : 1;
	switch (_error_code)
	{
		case 0: g_str = "   "; break;
		case 1: g_str = "  E"; break;
		case 2: g_str = "  W"; break;
	}
	
	for (int i = numChars; i < 5; i++)
		g_str.append(" ");

	return g_str.c_str();
}

/* 
Function for parsing the error message for an erronous shader source (failed
compilation). The source is outputted and the lines raising errors are marked.
*/
void annotate_shader_error_msg(const std::string& _source, const std::string& _error_msg)
{
	// error messages line number are given as 0:line_number(character_number). 
	// First, let's find that line number.
	// Example:
	// 0:19(2): error: value of type vec4 cannot be assigned to variable of type vec2
	
	// map of the line number and the error code (0 = error, 1 = warning)
	std::map<int, int> errorLinesMap;
	std::istringstream errorLineStream(_error_msg);
	std::string line;
	// split error message into lines and parse each line for 
	// erronous line and error code (warning or error).
	while (std::getline(errorLineStream, line, '\n')) {
		size_t lineNumStart = line.find('(');
		size_t lineNumEnd =  line.find(':') - 1;
		if (lineNumStart == std::string::npos || lineNumEnd == std::string::npos)
			continue;
		
		// extract the line number
		std::string s = line.substr(line.find(':')+1, lineNumEnd - lineNumStart);
		int errLineNumber = atoi(s.c_str());
		
		// find if warning or error
		int errCode;
		if (line.find("error") != std::string::npos)
			errCode = 1;
		else if (line.find("warning") != std::string::npos)
			errCode = 2;
		else
			errCode = 0;

		errorLinesMap.insert(std::pair<int, int>(errLineNumber, errCode));
	}

	// read the source line by line
	std::vector<std::string> srcLines;
	std::istringstream sstream(_source);
	int lineNumber = 1;
	
	while (std::getline(sstream, line, '\n')) {
		// add leading blanks and the line number to each line.
		// the erronous line will be marked with 'E' or 'W'.

		// scan the marked errors for the current line number
		int errCode = 0;
		for (auto& item : errorLinesMap) {
			if (item.first == lineNumber)
				errCode = item.second;
		}

		// create a line prefix with line number and optional 'E' or 'W'
		std::string linePrefix = std::string(leading_blank_spaces(lineNumber, errCode));
		linePrefix += std::to_string(lineNumber);
		linePrefix += ": ";

		line.insert(0, linePrefix);
		srcLines.push_back(line);
		lineNumber++;
		
	}

	// concatenate message for logging
	std::string logMsg = "";
	for (auto& line : srcLines)
		logMsg = logMsg + line + '\n';

	SYN_INFO("\n%s\n", logMsg.c_str());

}

//-----------------------------------------------------------------------------------
// Shader class functions
// 
shader_t::shader_t(const std::string &_shader_file_path) : 
	m_asset_path(_shader_file_path)
{
	// extract name from path
	size_t lastSlash = _shader_file_path.find_last_of("/\\");
	lastSlash = (lastSlash == std::string::npos ? _shader_file_path.size() : lastSlash + 1);
	size_t lastDot = _shader_file_path.rfind(".");
	lastDot = (lastDot == std::string::npos ? _shader_file_path.size() : lastDot);
	m_shader_name = _shader_file_path.substr(lastSlash, (lastDot - lastSlash));

	load_from_file();
	reload();

}

// 
shader_t::shader_t(const std::string &_shader_name, const std::string &_file_path) :
	m_shader_name(_shader_name), m_asset_path(_file_path)
{
	load_from_file();
	reload();
}

// 
void shader_t::destroy()
{
    if (m_opengl_id != 0) {
        glDeleteProgram(m_opengl_id);
    }
    memset(m_uniform_cache, 0, sizeof(uniform_cache_t) * SYN_MAX_SHADER_UNIFORMS);
    m_uniform_count = 0;
    m_opengl_id = 0;
    m_is_active = false;
}


// 
void shader_t::load_from_file()
{
	int result = file_io_handler.read_file_to_buffer(m_asset_path, m_raw_src);
	if (result != 0) {
	    SYN_INFO("shader not loaded, could not open file '%s'.\n", m_asset_path.c_str());
		return;
	}
}

// 
void shader_t::load_from_source(const std::string &_name, const std::string &_src)
{
	m_shader_name = _name;
	m_raw_src = _src;
	reload();
	
}

// 
void shader_t::reload()
{
	// reset setup flag
	m_is_active = false;

	if (!m_asset_path.empty()) {
        load_from_file();
	}
	
	if (m_raw_src.empty()) {
		SYN_INFO("no shader source provided.");
		return;
	}

	// preprocess, i.e. get vertex and fragment shaders separated
	m_shader_src = preprocess(m_raw_src);

	// parse all uniforms for resolving below, threaded
	// std::vector<std::string> uniforms = parse_uniforms();

	// compile the shader program
	int res = compile_shader();
	if (res != 0) {
		SYN_WARNING("%s: couldn't compile shader.\n", m_shader_name.c_str());
		return;
	}

	// resolve uniforms
	// resolve_uniforms(uniforms);
	reflect_uniforms();

	// update flag
	m_is_active = true;

	if (!m_asset_path.empty()) {
	    try {
			m_last_write_time = std::filesystem::last_write_time(m_asset_path);
		} catch (...) {}
	}
	
}

// 
bool shader_t::file_has_changed()
{
    if (m_asset_path.empty()) return false;

    try {
        auto current_time = std::filesystem::last_write_time(m_asset_path);
        return current_time != m_last_write_time;
    } catch (...) {
        return false;
    }
}

// 
std::unordered_map<GLenum, std::string> shader_t::preprocess(const std::string& _source)
{
	std::unordered_map<GLenum, std::string> shader_sources;

	const char *type_token = "#type";
	size_t len_token = strlen(type_token);
	// find first token
	size_t pos = _source.find(type_token, 0);

	while (pos != std::string::npos) {
		size_t eol = _source.find_first_of("\r\n", pos);
		size_t begin = pos + len_token + 1;
		GLenum type = shader_type_from_str(_source.substr(begin, eol - begin));
		if (type == GL_NONE) {
			SYN_WARNING("unknown shader type -- loading default backup shader.");
			m_is_active = false;
			break;
		}

		size_t nextLinePos = _source.find_first_not_of("\r\n", eol);
		pos = _source.find(type_token, nextLinePos);
		shader_sources[type] = _source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? _source.size() - 1 : nextLinePos));
	}

	return shader_sources;
}

//
void shader_t::reflect_uniforms()
{
    memset(m_uniform_cache, 0, sizeof(uniform_cache_t) * SYN_MAX_SHADER_UNIFORMS);
    m_uniform_count = 0;

    GLint count = 0;
    glGetProgramiv(m_opengl_id, GL_ACTIVE_UNIFORMS, &count);

    for (GLint i = 0; i < count; i++) {
        if (m_uniform_count >= SYN_MAX_SHADER_UNIFORMS) {
            SYN_WARNING("Shader ID %d exceeded maximum uniform allocation limit!\n", m_opengl_id);
            break;
        }

        GLsizei len;
        GLint size;
        GLenum type;
        char uniform_name[SYN_MAX_UNIFORM_NAME_LEN];
        glGetActiveUniform(m_opengl_id, (GLuint)i, SYN_MAX_UNIFORM_NAME_LEN - 1, &len, &size, &type, uniform_name);

        GLint location = glGetUniformLocation(m_opengl_id, uniform_name);
        if (location != -1) {
            uniform_cache_t &cache_slot = m_uniform_cache[m_uniform_count];
            strncpy(cache_slot.name, uniform_name, SYN_MAX_UNIFORM_NAME_LEN - 1);
            cache_slot.location = location;
            m_uniform_count++;
        }
        
    }
}

GLint shader_t::get_uniform_location(const char *_name)
{
    for (uint32_t i = 0; i < m_uniform_count; i++) {
        if (strcmp(m_uniform_cache[i].name, _name) == 0) {
            return m_uniform_cache[i].location;
        }
    }
    return -1;
}

// 
int shader_t::compile_shader()
{
	std::array<GLuint, 4> shaderIDs;
	int index = 0;
	GLuint program;

	// create the shader program server side
	program = glCreateProgram();
	
	#ifdef DEBUG_SHADER_SETUP
	SYN_INFO("creating shader '%s' [%d].\n", m_shader_name.c_str(), program);
	#endif

	// step through each key-value pair in shader sources
	for (auto& kv : m_shader_src) {
		GLenum type = kv.first;
		std::string& src = kv.second;

		// create and compile shader of type from source
		GLuint shaderID = glCreateShader(type);
		const GLchar* srcCstr = (const GLchar*)src.c_str();
		glShaderSource(shaderID, 1, &srcCstr, 0);

		glCompileShader(shaderID);

		// compilation status
		GLint isCompiled;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint len = 0;
			glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &len);
			std::vector<char> errorLog(len);
			glGetShaderInfoLog(shaderID, len, &len, errorLog.data());

			std::string msg = "";
			for (auto c : errorLog)
				msg += c;
			SYN_ERROR("%s\n", msg.c_str());
			SYN_ERROR("Annotated source '%s' (%s):\n", m_shader_name.c_str(), shader_str_from_type(type).c_str());
			annotate_shader_error_msg(src, msg);

			// prevent mem leak
			glDeleteShader(shaderID);

			return -1;
		}

		shaderIDs[index++] = shaderID;
		glAttachShader(program, shaderID);

	}

	// link the program
	#ifdef DEBUG_SHADER_SETUP
	SYN_INFO("linking program %d.\n", program);
	#endif

	glLinkProgram(program);

	// error checking
	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
	if (isLinked == GL_FALSE) {
		GLint len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		std::vector<char> errorLog(len);

		glGetProgramInfoLog(program, len, &len, errorLog.data());

		std::string msg = "";
		for (auto c : errorLog)
			msg += c;
		SYN_ERROR("%s\n", msg.c_str());

		// release program and shaders
		glDeleteProgram(program);

		for (auto id : shaderIDs)
			glDeleteShader(id);

		return -1;
	}

	// detach and delete shaders after linking
	for (auto id : shaderIDs)
		glDetachShader(program, id);


	m_opengl_id = program;

	return 0;

}
 
// 
void shader_t::enable()
{
	glUseProgram(m_opengl_id);
}

// 
void shader_t::disable()
{
	glUseProgram(0);
}

// 
void shader_t::print_uniforms()
{
	std::string str = "shader '"+ m_shader_name +"' uniforms:\n";
	for (uint32_t i = 0; i < m_uniform_count; i++) {
	    str += '\t' + std::string(m_uniform_cache[i].name) + ": " + std::to_string(m_uniform_cache[i].location) + '\n';
	}
	str = str.substr(0, str.size()-1);
	SYN_INFO("%s\n", str.c_str());
}

// 
void shader_t::set_asset_path(const std::string &_path)
{
    m_asset_path = _path;
    if (!m_asset_path.empty()) {
        try {
            m_last_write_time = std::filesystem::last_write_time(m_asset_path);
        } catch (...) {}
    }
}

//-----------------------------------------------------------------------------------
// Uniform accessors
//
void shader_t::set_uniform_1i(const GLint& _location, const int& _i) { glUniform1i(_location, _i); }
void shader_t::set_uniform_1f(const GLint& _location, const float& _f) { glUniform1f(_location, _f); }
void shader_t::set_uniform_2iv(const GLint& _location, const glm::ivec2& _v) { glUniform2iv(_location, 1, (GLint*)(&_v)); }
void shader_t::set_uniform_2fv(const GLint& _location, const glm::vec2& _v) { glUniform2fv(_location, 1, (GLfloat*)(&_v)); }
void shader_t::set_uniform_3fv(const GLint& _location, const glm::vec3& _v) { glUniform3fv(_location, 1, (GLfloat*)(&_v)); }
void shader_t::set_uniform_4fv(const GLint& _location, const glm::vec4& _v) { glUniform4fv(_location, 1, (GLfloat*)(&_v)); }
void shader_t::set_matrix_2fv(const GLint& _location, const glm::mat2& _mat) { glUniformMatrix2fv(_location, 1, GL_FALSE, (GLfloat*)& _mat); }
void shader_t::set_matrix_3fv(const GLint& _location, const glm::mat3& _mat) { glUniformMatrix3fv(_location, 1, GL_FALSE, (GLfloat*)& _mat); }
void shader_t::set_matrix_4fv(const GLint& _location, const glm::mat4& _mat) { glUniformMatrix4fv(_location, 1, GL_FALSE, (GLfloat*)& _mat); }

