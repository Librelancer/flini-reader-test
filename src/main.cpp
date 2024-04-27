#include "Common.h"
#include <stdio.h>
#include <stdlib.h>

void replace_all(char* str, char search, char replace)
{
	char* pos = strchr(str, search);
	while (pos) {
		*pos = replace;
		pos = strchr(pos, search);
	}
}

char *str_escape(char* buffer, int buffer_size, const char *orig) {
    const char *matches = "\\\"";

    // sanity checks and initialization
    if (!orig || !buffer)
        return NULL;
	memset(buffer, 0, buffer_size);

    const char *tok = orig;
	char *ins = buffer;
	const char *tmp;
    for (int count = 0; tmp = strpbrk(tok, matches); ++count) {
        strncpy_s(ins, buffer_size - (ins - buffer), tok, tmp - tok);
		ins += tmp - tok;
		ins[0] = '\\';
		ins[1] = tmp[0];
		ins[2] = 0;
		ins += 2;
		tok = tmp + 1;
    }
	strncat_s(ins, buffer_size - (ins - buffer), tok, strlen(tok));
    return buffer;
}

void as_csharp(INI_Reader* reader, char *command_line, char *path) {
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

void as_text(INI_Reader* reader) {
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
    INI_Reader* reader = new INI_Reader();
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
