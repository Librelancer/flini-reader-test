#include "Common.h"
#include <stdio.h>
int main(int argc, char **argv)
{
    INI_Reader* reader = new INI_Reader();
    reader->open(argv[1], true);
    while(reader->read_header()) {
        printf("HEADER: %s\n", reader->get_header_ptr());
        while(reader->read_value()) {
            printf("ENTRY: %s", reader->get_name_ptr());
            int parms = reader->get_num_parameters();
            if(parms)
                printf(", values = ");
            printf("%s", reader->get_value_string(0));
            for(int i = 1; i < parms; i++) {
                printf(", %s", reader->get_value_string((UINT)i));
            }
            printf("\n");
        }
    }
    reader->close();
    delete reader;
    return 0;
}
