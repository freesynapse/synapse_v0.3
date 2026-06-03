
#ifdef _WIN64
#include <fileapi.h>
#else
#include <fcntl.h>
#endif

#include <fstream>
#include <algorithm>
#include <string.h>

#include "utils/file_io_handler.h"
#include "core.h"
#include "utils/log.h"


// global instance
file_io_handler_t file_io_handler;

// 
#ifdef _WIN64
void file_io_handler_t::shutdown()
{
	for (auto file : m_createdFiles)
	{
		SYN_CORE_TRACE("deleting file '", file, "'.");
		if (!DeleteFileA(file.c_str()))
		{
			std::string error = "could not delete file '" + file + "': ";
			DWORD code = GetLastError();
			switch (code)
			{
				case ERROR_FILE_NOT_FOUND:
					SYN_CORE_WARNING(error, "ERROR_FILE_NOT_FOUND.");
					break;
				case ERROR_ACCESS_DENIED:
					SYN_CORE_WARNING(error, "ERROR_ACCESS_DENIED.");
					break;
				default:
					SYN_CORE_WARNING(error, "unknown error code (", code, ").");
			}
		}
	}
}

#else
void file_io_handler_t::shutdown()
{
	for (auto file : created_files)
	{
	    SYN_INFO("deleting file '%s'.\n", file.c_str());
		if (remove(file.c_str()) != 0) {
			std::string error = "could not delete file '" + file + "': ";
			SYN_WARNING("%s : %d -- %s\n", error.c_str(), errno, strerror(errno));
		}
	}
}
#endif

// 
int file_io_handler_t::tell_file_size(const std::string& _file_path)
{
	std::ifstream file(_file_path, std::ios::binary);

	file.seekg(0, std::ios::end);
	int fileSize = (int)file.tellg();
	file.close();

	return fileSize;
}

// 
int file_io_handler_t::write_buffer_to_file(const std::string& _file_path, const std::string& _buffer, bool _keep_file)
{
	std::ofstream file(_file_path, std::ios::binary | std::ios::trunc);

	if (file.fail() || !file.is_open()) {
		SYN_ERROR("file '%s' could not be opened/created.\n", _file_path.c_str());
		return RETURN_FAILURE;
	}

	file << _buffer;
	file.flush();

	file.close();

	if (!_keep_file) {
		if (std::find(created_files.begin(), created_files.end(), _file_path) == created_files.end())
		created_files.push_back(_file_path);
	}

	return RETURN_SUCCESS;

}

// 
int file_io_handler_t::read_file_to_buffer(const std::string &_file_path, std::vector<unsigned char> &_buffer)
{
	std::ifstream file(_file_path, std::ios::binary);

	// error check
	if (file.fail())
	{
		//events.push_event(new FileIOErrorevent_t(_file_path));
		SYN_ERROR("file '%s' could not be opened.\n", _file_path.c_str());
		return RETURN_FAILURE;
	}

	// seek to end of file
	file.seekg(0, std::ios::end);

	// get file size and return to the top
	unsigned int fileSize = (unsigned int)file.tellg();
	file.seekg(0, std::ios::beg);

	// reduce file size by headers etc.
	fileSize -= (unsigned int)file.tellg();

	// resize the buffer and read the data
	_buffer.resize(fileSize);
	file.read((char *)&(_buffer[0]), fileSize);
	file.close();

	return RETURN_SUCCESS;

}

// 
int file_io_handler_t::read_file_to_buffer(const std::string &_file_path, std::string &_buffer)
{
	std::ifstream file(_file_path, std::ios::binary);

	// error check
	if (file.fail())
	{
	    SYN_ERROR("file '%s' could not be opened.\n", _file_path.c_str());
		return RETURN_FAILURE;
	}

	// seek to end of file
	file.seekg(0, std::ios::end);

	// get file size and return to the top
	unsigned int fileSize = (unsigned int)file.tellg();
	file.seekg(0, std::ios::beg);

	// reduce file size by headers etc.
	fileSize -= (unsigned int)file.tellg();

	// resize the buffer and read the data
	_buffer.resize(fileSize);
	file.read((char *)&(_buffer[0]), fileSize);
	file.close();

	return RETURN_SUCCESS;

}

// 
void file_io_handler_t::print_created_files()
{
	for (auto const& file : created_files)
		SYN_INFO("%s\n", file.c_str());

}

