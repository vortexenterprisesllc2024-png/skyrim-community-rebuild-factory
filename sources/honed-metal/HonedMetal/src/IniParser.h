#ifndef INI_PARSER_H
# define INI_PARSER_H

# include <vector>

namespace iniparser
{
	size_t parse(std::vector<UInt32> *out, const char *setting);
}
#endif //IniParser.h
