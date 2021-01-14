
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ftw.h>
#include <dlfcn.h>
#include <limits.h>
#include <errno.h>
#include "plugin_api.h"
#include <inttypes.h>
#include <limits.h>
#define _XOPEN_SOURCE 500
#define _DEFAULT_SOURCE



#define no_argument            0
#define required_argument      1
#define optional_argument      2

int option_count = 0;
char *Version = "0.88";
struct option *long_options;
int inversion_flag = 0;
int condition_flag = 0;
char *condition;
FILE *log;

int founds = 0;
int (*get_info)(struct plugin_info*);
int (*plugin_get_main_function)(const char *fname,
                           struct option *in_opts[],
                           size_t in_opts_len,
                           char *out_buff,
                           size_t out_buff_len);

struct plugin{
    void *plugin_file;
    struct plugin_info *plugin_info;
    int (*plugin_get_main_function)(const char *fname,
                                    struct option *in_opts[],
                                    size_t in_opts_len,
                                    char *out_buff,
                                    size_t out_buff_len);
    int option_number;
    int found_option;
};

struct plugin *plugins;


int getopt_long(int argc,
                char * const argv[],
                const char *optstring,
                const struct option *longopts,
                int *longindex);

int log_print(char *message)
{
	fprintf(log, "%s\n", message);
	return 0;
}


int option_append(struct option *list, char *namex, int has_argx, int *flagx, int valx)
{
    if (list == NULL)
    {
        printf("Failed to append option\n");
        return 1;
    }
    
    //printf("222\n");
    
    list[option_count].name = namex;
    list[option_count].has_arg = has_argx;
    list[option_count].flag = flagx;
    list[option_count].val = valx;

    option_count++;
    return 0;
}

int plugin_append(struct plugin *plugins, int pos, char *s)
{
    //plugins[pos].plugin_file = dlopen(s, RTLD_LAZY);
    //printf("---%s\n", current_plugin->d_name);
    printf("%s\n", s);
    //printf("%d\n", pos);
	//plugins[pos] = malloc(sizeof(struct plugin));
	char *ss = malloc(sizeof(2 + strlen(s)));
	strncat(ss, "./", 2);
	strncat(ss, s, strlen(s));
	//printf("+++ %s\n", ss);
    plugins[pos].plugin_file = dlopen(ss, RTLD_LAZY||RTLD_GLOBAL);
    if (plugins[pos].plugin_file == NULL)
    {
        printf("Error: could not open plugin\n");
        //printf("+11+\n");
        exit(1);
    }
    else 
    {
        plugins[pos].plugin_info = malloc(sizeof(struct plugin_info));
        get_info = dlsym(plugins[pos].plugin_file, "plugin_get_info");
        plugin_get_main_function = dlsym(plugins[pos].plugin_file, "plugin_process_file");
        int err_check;
        err_check = get_info(plugins[pos].plugin_info);
        if (err_check == 1) {
            //printf("+12+\n");
            printf("Error: could not open plugin\n");
            exit(1);
        }
        else
        {

            plugins[pos].option_number = option_count;

            //plugins[pos].plugin_info->sup_opts = malloc(sizeof(struct option));

            /*option_append(long_options,
                          plugins[pos].plugin_info->sup_opts->opt.name,
                          plugins[pos].plugin_info->sup_opts->opt.has_arg,
                          plugins[pos].plugin_info->sup_opts->opt.flag,
                          plugins[pos].plugin_info->sup_opts->opt.val);*/

            long_options[option_count].name = plugins[pos].plugin_info->sup_opts->opt.name;
            //long_options[option_count].has_arg = plugins[pos].plugin_info->sup_opts->opt.has_arg;
            //long_options[option_count].flag = plugins[pos].plugin_info->sup_opts->opt.flag;

            //long_options[option_count].has_arg = required_argument;
            //long_options[option_count].flag = NULL;

            long_options[option_count].val = plugins[pos].plugin_info->sup_opts->opt.val;
            option_count++;


            printf("Plugin found: %s\n", plugins[pos].plugin_info->plugin_name);
            printf("Plugin option: %s\n", plugins[pos].plugin_info->sup_opts->opt.name);
            printf("Plugin main function added\n");


        }
        if (plugin_get_main_function != NULL)
        {
            plugins[pos].plugin_get_main_function = plugin_get_main_function;
            plugins[pos].found_option = 0;
            //printf("3\n");
        }
        else
        {
            printf("Error: failed to add plugin\n");
            exit(1);
        }

    }

    return 0;
}

int show_help(int x)
{
    printf("OPTIONS\n");
    printf("-h shows this page\n");
    printf("-v show version of program\n");
    printf("-P path to plugins directory\n");
    printf("-l add path to log file\n");
    printf("-C add condition for searching\n");
    printf("-N inversion of searching criteria\n");
    exit(0);
}

int show_version(int x)
{
    printf("Version %s\n", Version);
    return 0;
}

int search (const char * dir_name, struct plugin *p, int plugin_count)
{
    int xxx;

    DIR * d;

    d = opendir (dir_name);
    if (d == NULL)
    {
    	if (founds != 0)
    	{
    		return 0;
    	}
        printf("No path to start from\n");
        log_print("No path");
    	
        show_help(0);
        return 234;
    }
    if (! d) {
        fprintf (stderr, "Cannot open directory '%s': %s\n",
                 dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }

    while (1) {
        struct dirent * entry;
        const char * d_name;
        entry = readdir (d);
        if (! entry) {
            
            break;
        }
        d_name = entry->d_name;
       
        char *buff;
        size_t buff_size;
        buff = malloc(sizeof(char) * 25);
        buff_size = 25;
        int fl_veritas;

        if (strcmp(condition, "AND") == 0)
        {
            fl_veritas = 1;
        }
        if (strcmp(condition, "OR") == 0)
        {
            fl_veritas = 0;
        }

        if (! (entry->d_type & DT_DIR))
        {
	    char *gh;
            gh = malloc(sizeof(char) * 355);
            
		snprintf(gh, 123, "%s/%s", dir_name, d_name);	
		log_print("Opening file");
		log_print(gh);
    
            if (strstr(gh, ".txt") != NULL)
            {
                for (int i = 0; i < plugin_count; i++)
                {
                    if (p[i].found_option == 1)
                    {
                        xxx = p[i].plugin_get_main_function(gh,
                                                            &p[i].plugin_info->sup_opts,
                                                            p[i].plugin_info->sup_opts_len,
                                                            buff,
                                                            buff_size);
                        if (strcmp(condition, "AND") == 0)
                        {
                            if (xxx != 0)
                            {
                                fl_veritas = 0;
                            }
                        }
                        if (strcmp(condition, "OR") == 0)
                        {
                            if (xxx == 0)
                            {
                                fl_veritas = 1;
                                break;
                            }
                        }
                    }


                }
                if ((fl_veritas == 1) && (inversion_flag == 0))
                {
                    printf("Found in file: \t%s\n", gh);
                    log_print("Found");
    
                    founds++;
                }

                if ((fl_veritas == 0) && (inversion_flag != 0))
                {
                    printf("Found in file: \t%s\n", gh);
                    log_print("Found");
                    founds++;
                }
            }
            
	    }




        if (entry->d_type & DT_DIR) {

            if (strcmp (d_name, "..") != 0 &&
                strcmp (d_name, ".") != 0) {
                int path_length;
                char path[PATH_MAX];

                path_length = snprintf (path, PATH_MAX,
                                        "%s/%s", dir_name, d_name);
                if (path_length >= PATH_MAX) {
                    fprintf (stdout, "Path length has got too long.\n");
                    exit (EXIT_FAILURE);
                }
                search(path, p, plugin_count);
            }
        }
    }
    
    if (closedir (d)) {
        fprintf (stderr, "Could not close '%s': %s\n",
                 dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }
}



int main(int argc, char* argv[]) {
    int path_Option_Flag = 0;
    const char* short_options = "-:P:l:C:Nvh";
    long_options = malloc(sizeof(struct option) * 12);
    //int *option_flags = malloc(sizeof(int) * 12);
    char *plugins_path = "./";
    int cond; // AND = 0, OR = 1
    char *log_path = malloc(sizeof(char) * 100);
    log_path = ".";
    option_append(long_options, "help", no_argument, NULL, 'h');
    option_append(long_options, "version", no_argument, NULL, 'v');
    option_append(long_options, "inversion", no_argument, NULL, 'N');
    option_append(long_options, "path", required_argument, NULL, 'P');
    option_append(long_options, "logpath", required_argument, NULL, 'l');
    option_append(long_options, "condition", required_argument, NULL, 'C');
    int rez;
    int option_index = -1;

    int flag_plugin_option_found = 0;
	
    while ((rez = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        switch (rez) {
            case 'h': {
                break;
            };
            case 'v': {
                show_version(0);
                exit(0);
            };

            case 'P': {
                if (optarg) {
                    plugins_path = optarg;
                } else {
                    printf("Error: option -P required argument\n");
                    exit(1);
                }
                break;
            };

            case 'l': {
                if (optarg) {
                    log_path = optarg;
                } else {
                    printf("Error: option -l required argument\n");
                    exit(1);
                }
                break;
            }

            case 'N':
            {
                inversion_flag++;
                break;
            }
            case 'C':
            {
                condition_flag++;
                condition = optarg;
                if (condition == NULL)
                {

                    condition = "AND";
                }
                break;
            }

            case ':':
            {
                printf("Error: missing argument\n");
                exit(1);
            }

            case '?':
            default: {
                break;
            };
        };
    };

    if (condition == NULL)
    {

        condition = "AND";
    }




char *lg = malloc(sizeof(char) * 100);
strncat(lg, log_path, 100);

	strncat(lg, "/log.txt", 9);

	log = fopen(lg, "w");
	log_print("Start...");
	if (inversion_flag != 0)
	{
		log_print("Inversion: On");
	}
	
	



    option_index = -1;
    optind = 0;
    struct plugin_info *ppi = malloc(sizeof(struct plugin_info));

    DIR *plugins_directory;
    plugins_directory = opendir(plugins_path);
    if (plugins_directory == NULL)
    {
        printf("Error: could not open directory with plugins\n");
        log_print("Error: could not open directory with plugins");
        fclose(log);
        exit(1);
    }
    struct plugin *plugins;
    struct dirent *file_current;
    plugins = malloc(sizeof(struct plugin) * 8); //$$$
    int plugins_count = 0;
    char *so = ".so";

    while ((file_current = readdir(plugins_directory)) != NULL)
    {
        if (strstr(file_current->d_name, so) != NULL)
        {
            plugin_append(plugins, plugins_count, file_current->d_name);
            log_print("Found plugin");
            log_print("Plugin added");
            plugins_count++;
        }
    };
    printf("Found %d plugin(s)\n", plugins_count);
	log_print("End working with plugins, start...");




    for (int i = 6; i < 6 + plugins_count; i++)
    {
        long_options[i].has_arg = required_argument;
        long_options[i].flag = NULL;
    }
    int found_plugin_options = 0;

    int its_ok = 0;
    option_append(long_options, NULL, 0, NULL, 0);

    while ((rez = getopt_long(argc, argv, "+:P:l:C:Nvh", long_options, &option_index)) != -1) {
        switch (rez) {
            case ':':
            {
                printf("Error: missing argument\n");
                log_print("Error: missing argument");
                fclose(log);
                exit(1);
            }

            case 'h':
            {
                show_help(0);
                log_print("-h found, showing help");
                break;
            }
            case 'v':
            {
                break;
            }
            case 'P':
            {
                break;
            }
            case 'l':
            {
                break;
            }
            case 'N':
            {
                break;
            }
            case 'C':
            {
                break;
            }
            case 0:
            {
            	log_print("Found plugin option");
            	printf("Found plugin option\n");
                for (int i = 0; i < plugins_count; i++)
                {
                    if ((strcmp(plugins[i].plugin_info->sup_opts[0].opt.name, long_options[option_index].name) == 0) && (plugins[i].found_option == 0))
                    {
                        its_ok = 1;
                        flag_plugin_option_found++;
                        if (optarg == NULL)
                        {

                            printf("Error: missing argument\n");
                            log_print("Error: missing argument");
                		fclose(log);
                            exit(1);
                        }
                        else
                        {
                            plugins[i].plugin_info->sup_opts->opt.flag = optarg;
                            plugins[i].found_option = 1;
                            found_plugin_options++;
                        }
                    }
                }
                if (its_ok == 0)
                {
                    printf("Error: unknown option\n");
                    log_print("Error: unknown option");
                	fclose(log);
                    exit(1);
                }
                break;

            }
                if (flag_plugin_option_found == 0)
                {
                    printf("Error: plugins options not found\n");
                    log_print("Error: plugin option not found");
                	fclose(log);
                    exit(1);
                }


            case '?':
            {
                printf("Error: unknown option\n");
                log_print("Error: unknown option");
                	fclose(log);
                exit(1);
            }
            default: {
                printf("Error: unknown option\n");
                log_print("Error: unknown option");
                	fclose(log);
                exit(1);
                break;
            };
        };
    };
    

	if (flag_plugin_option_found == 0)
	{
		printf("Plugin options not found\n");
		exit(1);
	}
    log_print("Searching...");
    log_print("\n");
    
    int search_checker = search(argv[argc - 1], plugins, plugins_count);
    
    printf("Found:\t%d\n", founds);
    log_print("ENDED");
    fclose(log);

    return 0;
}
