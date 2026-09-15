#include "SKSE_resolver.h"
#include "IniParser.h"
#include "common/IDebugLog.h"
#include RESOLVE_PATH(SKSE_FOLDER, GameData.h)


namespace iniparser
{
	#define SUBSECTN_MARKER '|'
	#define SCTN_MRKR '['
	#define SCTN_ENDMRKR ']'

	struct bufferdata
	{
		FILE					*file;
		uint8_t					*buffstart;
		const uint8_t			*buffposition;
		unsigned				red;
		static const unsigned  	bufflen = 2047u;
		static const unsigned	subsectn_max_len = 63u;
		uint8_t					buffer_storage[bufflen + !NULL];

		explicit				bufferdata(void);
		explicit				bufferdata(FILE* file);
								~bufferdata(void);
	};

	bufferdata::bufferdata()
	{	memset(buffer_storage, 0, sizeof(buffer_storage) );
		buffstart = buffer_storage;
		buffposition = buffstart;
		file = nullptr;
		red = 0;
	}
	bufferdata::bufferdata(FILE *f) : bufferdata()
	{	file = f;
	}
	bufferdata::~bufferdata() {}


	static void fillbuffer(bufferdata *data)
	{
		const long pos = ftell((*data).file);

		(*data).red = fread((void*)(*data).buffstart, 1, (*data).bufflen, (*data).file);
		if ((*data).red && *((*data).buffstart + (*data).red - !NULL) == '\n')
		{	fseek((*data).file, -!!'\n', SEEK_CUR);
			*((*data).buffstart + (*data).red--) = 0;
		}
		(*data).buffposition = (*data).buffstart;
	}

	static bool validate(const uint8_t *start, const uint8_t *end, const bufferdata *data)
	{
		char modname[(*data).subsectn_max_len + !NULL] = {0};

		if (end - start <= (*data).subsectn_max_len)
		{	memcpy(modname, start, end - start);
			#ifdef LEGENDARY_EDITION
				return ((*DataHandler::GetSingleton() ).GetModIndex(modname) != 255);
			# else
				const ModInfo* modInfo = (*DataHandler::GetSingleton() ).LookupModByName(modname);
				
				return (modInfo && (*modInfo).IsActive() );
			#endif
		}
		return (false);
	}

	static bool goto_section
	(const uint8_t* section, bufferdata* data, const uint8_t marker, uint8_t endmarker = 0)
	{
		const uint8_t	*pos;
		const uint8_t	sctn_boundary[sizeof(uint16_t) * sizeof(uint8_t)] = {'\n', marker};
		const unsigned	sctn_len = strlen((char*)section);

		endmarker = endmarker ? endmarker : marker;
		do
		{	pos = (*data).buffstart;
			while ((pos = (uint8_t*)memchr(pos, endmarker, (*data).red - (pos - (*data).buffstart) ) ) )
			{	const uint8_t *sctn_start;

				if (pos - (*data).buffstart >= sctn_len + sizeof(sctn_boundary)
				&& *(uint16_t*)((sctn_start = pos - sctn_len) - !!SCTN_MRKR - !!'\n') == *(uint16_t*)sctn_boundary)
				{	if (!memcmp(sctn_start, section, sctn_len) )
					{	(*data).buffposition = sctn_start;
						return (true);
					}
				}
				++pos;
			}
			fillbuffer(data);
		}	while ((*data).red);
		return (false);
	}

	static bool goto_nextsubsctn (bufferdata *data, const uint8_t marker, const uint8_t endmrkr)
	{
		const uint8_t *pos = (*data).buffposition;

		do
		{	const uint8_t *sctnend = (uint8_t*)memchr(pos, endmrkr, (*data).red - (pos - (*data).buffstart) );

			while ((pos = (uint8_t *)memchr(pos, marker, (*data).red - (pos - (*data).buffstart))))
			{	if (pos++ == (*data).buffstart || *(pos - !!SUBSECTN_MARKER - !!'\n') == '\n')
				{	const uint8_t *subsectend = (uint8_t*)memchr(pos, marker, (*data).red - (pos - (*data).buffstart) );

					if (subsectend && validate(pos, subsectend, data) )
					{	(*data).buffposition = subsectend;
						return (true);
					}
				}
			}
		    if (sctnend)
				break ;
			fillbuffer(data);
			pos = (*data).buffstart;
		}	while ((*data).red);
		return (false);
	}

	#define HEX 16
	static void parse_perks(std::vector<UInt32> *perklist, bufferdata *data)
	{
		FILE				*file = (*data).file;
		uint8_t				*buffer = (*data).buffstart;
		const uint8_t		*pos = (*data).buffposition;
		constexpr size_t	perklen = sizeof("0xffffffff") - !NULL;
		unsigned			remaining;

		do
		{	const uint8_t *newline = (uint8_t*)memchr(pos, '\n', (*data).red - (pos - buffer) );

			if (newline)
				pos = newline;
			remaining = ((*data).red - (pos - buffer) );
			while (remaining && !isprint(*pos) )
				++pos, --remaining;
			if ((*pos == (uint8_t)SCTN_MRKR || *pos == (uint8_t)SUBSECTN_MARKER)
 			&& *(pos - (pos != buffer) ) == (uint8_t)'\n')
			{	--pos;
				break;
			}
			if (remaining > perklen || feof(file) )
			{	UInt32 perk;

				if ((*pos == '0' && (*(pos + !!remaining) & 'X') == 'X') || isspace(*pos) )
					if ((perk = strtol((const char*)pos, (char**)&pos, HEX) ) )
						(*perklist).push_back(perk);
				if (!newline)
					pos = (buffer + (*data).red);
				continue;
			}
			fseek(file, -(long)(perklen + !!'\n'), SEEK_CUR);
			remaining = (*data).red = fread((void*)buffer, 1, (*data).bufflen, file);
			*(buffer + (*data).red) = 0;
			pos = buffer;
		}	while (remaining);
		(*data).buffposition = remaining ? pos : buffer + (*data).red;
	}

	extern size_t parse(std::vector<UInt32> *perklist, const char *section)
	{
		FILE *ini;
  
		if ((ini = fopen("Data/SKSE/Plugins/HonedMetal.ini", "rb") ) )
		{	bufferdata data(ini);
			
			if (goto_section((const uint8_t*)section, &data, SCTN_MRKR, SCTN_ENDMRKR) )
			{	parse_perks(perklist, &data);
				while (goto_nextsubsctn(&data, SUBSECTN_MARKER, SCTN_MRKR) )
					parse_perks(perklist, &data);
			}
			fclose(ini);
		}
		else _MESSAGE("Error opening the ini file.");
		return ((*perklist).size() );
	}
}