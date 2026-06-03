#ifndef __FILE_IO_HANDLER_H
#define __FILE_IO_HANDLER_H


#include <string>
#include <vector>


// static class
class file_io_handler_t
{
public:
	void shutdown();

	int tell_file_size(const std::string& _file_path);
	int write_buffer_to_file(const std::string& _file_path, const std::string& _buffer, bool _keep_file=false);
	int read_file_to_buffer(const std::string& _file_path, std::vector<unsigned char> &_buffer);
	int read_file_to_buffer(const std::string& _file_path, std::string &_buffer);

	void print_created_files();

private:
	std::vector<std::string> created_files;

};

extern file_io_handler_t file_io_handler;

#endif // __FILE_IO_HANDLER_H
