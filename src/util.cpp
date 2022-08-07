#include <time.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>

std::string get_file_contents(const char *filename)
{
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in)
  {
    std::ostringstream contents;
    contents << in.rdbuf();
    in.close();
    return(contents.str());
  }
  std::cout << "Could not read file: \"" << filename << "\"" << std::endl;
  return std::string();
}

std::string new_filename(char const* path, char const* ext)
{
    std::string filename;
	time_t t = time(0);
	tm& ts = *(localtime(&t));

    std::stringstream f;
	f.fill('0');
    f << path << std::setw(2) << ts.tm_mon+1
            << "-" << std::setw(2) << ts.tm_mday
            << "-" << std::setw(2) << ts.tm_hour
            << "-" << std::setw(2) << ts.tm_min
            << "-" << std::setw(2) << ts.tm_sec;

	bool valid_filename_found = false;
	for(int count = 1; !valid_filename_found; ++count) {
        std::stringstream out(f.str());
        out.seekp(0, std::ios_base::end);
		if(count > 1)  out << "-" << count;
		filename = out.str() + ext;
        std::ifstream file(filename);
		valid_filename_found = !file.is_open();
	}

	return filename;
}
