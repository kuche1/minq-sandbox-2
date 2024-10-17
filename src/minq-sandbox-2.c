
#include "../libsandbox/src/libsandbox.h"

#include <stdio.h> // printf
#include <string.h> // strcmp

#define ARG_NET_ON "--net-on"
#define ARG_NET_OFF "--net-off"
#define ARG_FS_META_ON "--fs-meta-on"
#define ARG_FS_META_OFF "--fs-meta-off"

int main(int argc, char * * argv){

    int net_set = 0;
    int net_on = 0;

    int fs_meta_set = 0;
    int fs_meta_on = 0;

    for(int arg_idx=1; arg_idx<argc; ++arg_idx){

        char * arg = argv[arg_idx];

        printf("arg: %s\n", arg);

        if(strcmp(arg, ARG_NET_ON) == 0){
            net_on = 1;
            net_set = 1;
        }else if(strcmp(arg, ARG_NET_OFF) == 0){
            net_on = 0;
            net_set = 1;
        }else if(strcmp(arg, ARG_FS_META_ON) == 0){
            fs_meta_on = 1;
            fs_meta_set = 1;
        }else if(strcmp(arg, ARG_FS_META_OFF) == 0){
            fs_meta_on = 0;
            fs_meta_set = 1;
        }else{
            printf("unknown argument `%s`\n", arg);
            return 1;
        }

    }

    if(!net_set){
        printf("you need to set networking state, use either `%s` or `%s`\n", ARG_NET_ON, ARG_NET_OFF);
        return 1;
    }

    if(!fs_meta_set){
        printf("you need to set filesystem metadata state, use either `%s` or `%s`\n", ARG_FS_META_ON, ARG_FS_META_OFF);
        return 1;
    }

    struct libsandbox_rules rules;
    libsandbox_rules_init(& rules, 1); // `1` stands for permissive, `0` for non-permissive // TODO make these into constants
    rules.networking_allow_all = net_on;
    rules.filesystem_allow_metadata = fs_meta_on;
    
    return 0;

}
