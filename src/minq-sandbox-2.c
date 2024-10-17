
#include "../libsandbox/src/libsandbox.h"

#include <stdio.h> // printf
#include <string.h> // strcmp

#define ARG_NET_ON "--net-on"
#define ARG_NET_OFF "--net-off"
#define ARG_FS_META_ON "--fs-meta-on"
#define ARG_FS_META_OFF "--fs-meta-off"
#define ARG_END "--"

int main(int argc, char * * argv){

    int net_set = 0;
    int net_on = 0;

    int fs_meta_set = 0;
    int fs_meta_on = 0;

    int arg_end_set = 0;
    int arg_end_idx = 0;

    for(int arg_idx=1; arg_idx<argc; ++arg_idx){

        char * arg = argv[arg_idx];

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
        }else if(strcmp(arg, ARG_END) == 0){
            arg_end_idx = arg_idx;
            arg_end_set = 1;
            break;
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

    if(!arg_end_set){
        printf("you need signify the end of the cmdline arguments for `%s` and the beginning of the command that is to be executed with `%s`\n", argv[0], ARG_END);
        return 1;
    }

    struct libsandbox_rules rules;
    libsandbox_rules_init(& rules, 1); // `1` stands for permissive, `0` for non-permissive // TODO make these into constants
    rules.networking_allow_all = net_on;
    rules.filesystem_allow_metadata = fs_meta_on;

    size_t ctx_private_size = libsandbox_get_ctx_private_size();
    char ctx_private[ctx_private_size];

    // don't worry - argv is null terminated
    if(libsandbox_fork(& argv[arg_end_idx+1], & rules, ctx_private)){
        printf("fork failed\n");
        return 1;
    }

    struct libsandbox_summary summary;
    libsandbox_summary_init(&summary);

    size_t path_size = 400;
    char path0[path_size];
    char path1[path_size];

    for(int running = 1; running;){

        switch(libsandbox_next_syscall(ctx_private, & summary, path_size, path0, path1)){

            case LIBSANDBOX_RESULT_FINISHED:{
                running = 0;
            }break;

            case LIBSANDBOX_RESULT_ERROR:{
                printf("something went wrong\n");
                return 1;
            }break;

            case LIBSANDBOX_RESULT_ACCESS_ATTEMPT_PATH0:{
                printf("attempt to access path `%s`\n", path0);
                if(libsandbox_syscall_allow(ctx_private)){
                    printf("something went wrong\n");
                    return 1;
                }
            }break;

            case LIBSANDBOX_RESULT_ACCESS_ATTEMPT_PATH0_PATH1:{
                printf("attempt to access paths `%s` and `%s`\n", path0, path1);
                if(libsandbox_syscall_allow(ctx_private)){
                    printf("something went wrong\n");
                    return 1;
                }
            }break;

        }

    }

    printf("process finished\n");
    printf("`%d` syscalls automatically blocked\n", summary.auto_blocked_syscalls);
    printf("return code `%d`\n", summary.return_code);
    
    return summary.return_code;
}
