
#include "../libsandbox/src/libsandbox.h"

#include <stdio.h> // printf
#include <string.h> // strcmp
#include <stdlib.h> // relloc

#define ARG_NET_ON "--net-on"
#define ARG_NET_OFF "--net-off"
#define ARG_FS_META_ON "--fs-meta-on"
#define ARG_FS_META_OFF "--fs-meta-off"
#define ARG_DEFAULT_MODE_PERMISSIVE "--default-mode-permissive"
#define ARG_DEFAULT_MODE_RESTRICTIVE "--default-mode-restrictive"
#define ARG_END "--"
#define ARGP_PATH_ALLOW "-pa:"
#define ARGP_PATH_ALLOW_LEN strlen(ARGP_PATH_ALLOW)
#define ARGP_PATH_DENY "-pd:"
#define ARGP_PATH_DENY_LEN strlen(ARGP_PATH_DENY)

#define PATH_CAP 400

//////
////// str related
//////

int startswith(char * str, char * prefix, size_t prefix_len){
    return strncmp(prefix, str, prefix_len) == 0;
}

int parent_contains_child_node(char * parent_node, size_t parent_node_len, char * child_node, size_t child_node_len){

    if(parent_node_len > child_node_len){
        return 0;
    }

    size_t idx = 0;

    for(; idx<parent_node_len; ++idx){

        char ch_parent = parent_node[idx];
        char ch_child = child_node[idx];

        if(ch_parent != ch_child){
            return 0;
        }

    }

    char ch_child = child_node[idx];

    if((ch_child == 0) || (ch_child == '/')){
        return 1;
    }

    return 0;
}

//////
////// path arr
//////

struct path_arr{
    char * * path;
    size_t * path_len;
    size_t cap;
    size_t len;
};

void path_arr_init(struct path_arr * arr){
    arr->path = NULL;
    arr->path_len = NULL;
    arr->cap = 0;
    arr->len = 0;
}

void path_arr_append(struct path_arr * arr, char * str){

    if(arr->len + 1 > arr->cap){

        arr->cap = (arr->len + 1) * 2;

        arr->path = realloc(arr->path, sizeof(* arr->path) * arr->cap);

        arr->path_len = realloc(arr->path_len, sizeof(* arr->path_len) * arr->cap);

        if(!arr->path || !arr->path_len){
            printf("out of memory\n");
            exit(1);
        }

    }

    char * path = malloc(PATH_CAP);

    ssize_t path_len_or_err = libsandbox_str_to_path(str, path, PATH_CAP);

    if(path_len_or_err < 0){
        printf("`libsandbox_str_to_path` failure\n");
        exit(1);
    }

    size_t path_len = path_len_or_err;

    arr->path[arr->len] = path;
    arr->path_len[arr->len] = path_len;

    arr->len += 1;
}

void path_arr_print(struct path_arr * arr){
    printf("path_arr< ");
    for(size_t idx=0; idx<arr->len; ++idx){
        char * path = arr->path[idx];
        printf("%ld:`%s` ", idx, path);
    }
    printf(">");
}

int path_arr_contains_child_node(struct path_arr * arr, char * child, size_t child_len){

    for(size_t idx=0; idx<arr->len; ++idx){

        char * parent = arr->path[idx];
        size_t parent_len = arr->path_len[idx];

        if(parent_contains_child_node(parent, parent_len, child, child_len)){
            return 1;
        }

    }

    return 0;
}

//////
////// main/related
//////

int path_is_allowed(enum libsandbox_rule_default default_mode, struct path_arr * arr_path_allow, struct path_arr * arr_path_deny, char * path, size_t path_len){

    if(path_arr_contains_child_node(arr_path_deny, path, path_len)){
        return 0;
    }

    if(default_mode == LIBSANDBOX_RULE_DEFAULT_RESTRICTIVE){
        return path_arr_contains_child_node(arr_path_allow, path, path_len);
    }else{
        return 1;
    }
}

int main(int argc, char * * argv){

    int net_set = 0;
    int net_on;

    int fs_meta_set = 0;
    int fs_meta_on;

    int default_mode_set = 0;
    enum libsandbox_rule_default default_mode;

    int arg_end_set = 0;
    int arg_end_idx;

    struct path_arr arr_path_allow;
    path_arr_init(& arr_path_allow);

    struct path_arr arr_path_deny;
    path_arr_init(& arr_path_deny);

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
        }else if(strcmp(arg, ARG_DEFAULT_MODE_PERMISSIVE) == 0){
            default_mode_set = 1;
            default_mode = LIBSANDBOX_RULE_DEFAULT_PERMISSIVE;
        }else if(strcmp(arg, ARG_DEFAULT_MODE_RESTRICTIVE) == 0){
            default_mode_set = 1;
            default_mode = LIBSANDBOX_RULE_DEFAULT_RESTRICTIVE;
        }else if(strcmp(arg, ARG_END) == 0){
            arg_end_idx = arg_idx;
            arg_end_set = 1;
            break;

        }else if(startswith(arg, ARGP_PATH_ALLOW, ARGP_PATH_ALLOW_LEN)){
            path_arr_append(& arr_path_allow, arg + ARGP_PATH_ALLOW_LEN);
        }else if(startswith(arg, ARGP_PATH_DENY, ARGP_PATH_DENY_LEN)){
            path_arr_append(& arr_path_deny, arg + ARGP_PATH_DENY_LEN);

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

    if(!default_mode_set){
        printf("you need to set the default mode, use either `%s` or `%s`\n", ARG_DEFAULT_MODE_PERMISSIVE, ARG_DEFAULT_MODE_RESTRICTIVE);
        return 1;
    }

    if(!arg_end_set){
        printf("you need signify the end of the cmdline arguments for `%s` and the beginning of the command that is to be executed with `%s`\n", argv[0], ARG_END);
        return 1;
    }

    printf("allowed paths: ");
    path_arr_print(& arr_path_allow);
    printf("\n");

    printf("denied paths: ");
    path_arr_print(& arr_path_deny);
    printf("\n");

    struct libsandbox_rules rules;
    libsandbox_rules_init(& rules, default_mode);
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
    size_t path0_len = 0;
    char path1[path_size];
    size_t path1_len = 0;

    for(int running = 1; running;){

        switch(libsandbox_next_syscall(ctx_private, & summary, path_size, path0, & path0_len, path1, & path1_len)){

            case LIBSANDBOX_RESULT_FINISHED:{
                running = 0;
            }break;

            case LIBSANDBOX_RESULT_ERROR:{
                printf("something went wrong\n");
                return 1;
            }break;

            case LIBSANDBOX_RESULT_ACCESS_ATTEMPT_PATH0:{

                if(path_is_allowed(default_mode, & arr_path_allow, & arr_path_deny, path0, path0_len)){

                    printf("ms2:P %s\n", path0);

                    if(libsandbox_syscall_allow(ctx_private)){
                        printf("something went wrong\n");
                        return 1;
                    }

                }else{

                    printf("ms2:D %s\n", path0);

                    if(libsandbox_syscall_deny(ctx_private)){
                        printf("something went wrong\n");
                        return 1;
                    }

                }

            }break;

            case LIBSANDBOX_RESULT_ACCESS_ATTEMPT_PATH0_PATH1:{

                if(path_is_allowed(default_mode, & arr_path_allow, & arr_path_deny, path0, path0_len)){

                    printf("ms2:P %s\n", path0);

                    if(path_is_allowed(default_mode, & arr_path_allow, & arr_path_deny, path1, path1_len)){

                        printf("ms2:P %s\n", path1);

                        if(libsandbox_syscall_allow(ctx_private)){
                            printf("something went wrong\n");
                            return 1;
                        }

                    }else{

                        printf("ms2:D %s\n", path1);

                        if(libsandbox_syscall_deny(ctx_private)){
                            printf("something went wrong\n");
                            return 1;
                        }

                    }

                }else{

                    printf("ms2:D %s\n", path0);

                    if(libsandbox_syscall_deny(ctx_private)){
                        printf("something went wrong\n");
                        return 1;
                    }

                }

            }break;

        }

    }

    printf("process finished\n");
    printf("`%d` syscalls automatically blocked\n", summary.auto_blocked_syscalls);
    printf("return code `%d`\n", summary.return_code);
    
    return summary.return_code;
}
