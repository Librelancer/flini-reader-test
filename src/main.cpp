#include "Common.h"
#include <stdio.h>
#include <stdlib.h>

static const char CP1252_UNICODE_TABLE[] =
    "u20ACu0020u201Au0192u201Eu2026u2020u2021"
    "u02C6u2030u0160u2039u0152u0020u017Du0020"
    "u0020u2018u2019u201Cu201Du2022u2013u2014"
    "u02DCu2122u0161u203Au0153u0020u017Eu0178"
    "u00A0u00A1u00A2u00A3u00A4u00A5u00A6u00A7"
    "u00A8u00A9u00AAu00ABu00ACu00ADu00AEu00AF"
    "u00B0u00B1u00B2u00B3u00B4u00B5u00B6u00B7"
    "u00B8u00B9u00BAu00BBu00BCu00BDu00BEu00BF"
    "u00C0u00C1u00C2u00C3u00C4u00C5u00C6u00C7"
    "u00C8u00C9u00CAu00CBu00CCu00CDu00CEu00CF"
    "u00D0u00D1u00D2u00D3u00D4u00D5u00D6u00D7"
    "u00D8u00D9u00DAu00DBu00DCu00DDu00DEu00DF"
    "u00E0u00E1u00E2u00E3u00E4u00E5u00E6u00E7"
    "u00E8u00E9u00EAu00EBu00ECu00EDu00EEu00EF"
    "u00F0u00F1u00F2u00F3u00F4u00F5u00F6u00F7"
    "u00F8u00F9u00FAu00FBu00FCu00FDu00FEu00FF";

void replace_all(char *str, char search, char replace)
{
	char *pos = strchr(str, search);
	while (pos) {
		*pos = replace;
		pos = strchr(pos, search);
	}
}

char *mystrpbrk(const char *str, const char *brk)
{
	while (*str)
	{
		if (*str < 0 || strchr(brk, *str)) return (char*)str;
		str++;
	}
	return NULL;
}

char *str_escape(char *buffer, int buffer_size, const char *orig) {
    const char *matches = "\\\"";

    // sanity checks and initialization
    if (!orig || !buffer)
        return NULL;
	memset(buffer, 0, buffer_size);

    const char *tok = orig;
	char *ins = buffer;
	const char *tmp;
    for (int count = 0; tmp = mystrpbrk(tok, matches); ++count) {
        strncpy_s(ins, buffer_size - (ins - buffer), tok, tmp - tok);
		ins += tmp - tok;
		ins[0] = '\\';
		ins++;
		if (*tmp < 0) {
			int table_pos = (((unsigned char)*tmp) - 0x80) * 5;
			strncpy_s(ins, buffer_size - (ins - buffer), &CP1252_UNICODE_TABLE[table_pos], 5);
			ins += 5;
		}
		else {
			ins[0] = tmp[0];
			ins[1] = 0;
			ins++;
		}
		tok = tmp + 1;
    }
	strncat_s(ins, buffer_size - (ins - buffer), tok, strlen(tok));
    return buffer;
}

void as_csharp(INI_Reader *reader, char *command_line, char *path) {
	int header_count = 0;
	char escape_buffer[1024];
	int escape_buffer_len = sizeof(escape_buffer);

	// Replace \ with / in path for windows compatibility
	replace_all(path, '\\', '/');

	// Calculate the test name and filename from the path
	char test_name[256];
	memset(test_name, 0, sizeof(test_name));
	char *filename = strrchr(path, '/');
	filename = filename == NULL ? path : filename + 1;
	int len = strlen(filename);
	char *ext = strchr(filename, '.');
	if (ext != NULL) len = ext - filename;
	strncat_s(test_name, filename, len);

	printf("// Auto-generated code - DO NOT Modify.\n");
	printf("// This code was generated with the following command-line:\n// \t%s\n\n", command_line);
	
	printf("using System.Linq;\n\n");
	printf("using FluentAssertions;\nusing Xunit;\n\n");

	printf("namespace LibreLancer.Tests.Ini;\n\n");
	printf("public partial class IniTests\n{\n    [Fact]\n    public void %sTest()\n    {\n", test_name);
	printf("        var ini = ParseFile(TestAsset.Open<IniTests>(\"%s\"), false, false).ToList();\n", filename);
	while (reader->read_header()) {
		printf("        ini[%d].Name.Should().Be(\"%s\");\n", 
			header_count, 
			str_escape(escape_buffer, escape_buffer_len, reader->get_header_ptr()));
		int entry_count = 0;
		while (reader->read_value()) {
			printf("        ini[%d][%d].Name.Should().Be(\"%s\");\n", 
				header_count, 
				entry_count, 
				str_escape(escape_buffer, escape_buffer_len, reader->get_name_ptr()));
			int parms = reader->get_num_parameters();
			printf("        ini[%d][%d].Should().HaveCount(%d);\n", 
				header_count, 
				entry_count, 
				parms);
			for (int i = 0; i < parms; i++) {
				printf("        ini[%d][%d][%d].ToString().Should().Be(\"%s\");\n", 
					header_count, 
					entry_count, 
					i, 
					str_escape(escape_buffer, escape_buffer_len, reader->get_value_string((UINT)i)));
				
				printf("        ini[%d][%d][%d].ToBoolean().Should().Be(%s);\n", 
					header_count, 
					entry_count, 
					i, 
					reader->get_value_bool((UINT)i) ? "true" : "false");
				printf("        ini[%d][%d][%d].ToInt32().Should().Be(%d);\n",
					header_count, 
					entry_count, 
					i, 
					reader->get_value_int((UINT)i));
				printf("        ini[%d][%d][%d].ToSingle().Should().Be((float)%f);\n", 
					header_count, 
					entry_count, 
					i, 
					reader->get_value_float((UINT)i));
			}
			entry_count++;
		}
		printf("        ini[%d].Count.Should().Be(%d);\n", 
			header_count,
			entry_count);
		header_count++;
	}
	printf("        ini.Count.Should().Be(%d);\n", 
			header_count);	
	printf("    }\n}\n");
}

void as_text(INI_Reader *reader) {
	while (reader->read_header()) {
		printf("[%s]\n", reader->get_header_ptr());
		while (reader->read_value()) {
			printf("%s = ", reader->get_name_ptr());
			int parms = reader->get_num_parameters();
			if (parms) {
				printf("%s", reader->get_value_string(0));
				for (int i = 1; i < parms; i++) {
					printf(", %s", reader->get_value_string((UINT)i));
				}
			}
			printf("\n");
		}
		printf("\n");
	}
}

int main(int argc, char **argv)
{
    INI_Reader *reader = new INI_Reader();
    if (argc <= 1) {
		printf("%s <inifile> [--text] [--csharp]", argv[0]);
		return 1;
	}

	// Make a string from the command line arguments for use later
    char command_line[256];
	if (argc > 2) {
		snprintf(command_line, sizeof(command_line), "%s %s %s", argv[0], argv[1], argv[2]);
	} else {
		snprintf(command_line, sizeof(command_line), "%s %s", argv[0], argv[1]);
	}

	reader->open(argv[1], true);
	
	if (argc > 2) {
		if (strncmp(argv[2], "--csharp", 9) == 0) {
			as_csharp(reader, command_line, argv[1]);
		} else if (strncmp(argv[2], "--text", 7) == 0) {
			as_text(reader);
		}
	} else {	
		while (reader->read_header()) {
			printf("HEADER: %s\n", reader->get_header_ptr());
			while (reader->read_value()) {
				printf("ENTRY: %s", reader->get_name_ptr());
				int parms = reader->get_num_parameters();
				if (parms) {
					printf(", values = ");
					printf("%s", reader->get_value_string(0));
					for (int i = 1; i < parms; i++) {
						printf(", %s", reader->get_value_string((UINT)i));
					}
				}
				printf("\n");
			}
		}
	}
	
    reader->close();
    delete reader;
    return 0;
}
