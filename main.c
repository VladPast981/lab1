#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
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
int countOfOptions = 0;
char *Version = "0.44";
struct option *arrayOfOptions;
int foundInversion = 0;
int foundCondition = 0;
char *condition;
FILE *log;
int truthX = 0;
int (*get_info)(struct plugin_info*);
int (*plugin_get_main_function)(const char *fname, struct option *in_opts[], size_t in_opts_len, char *out_buff, size_t out_buff_len);
struct plugin{
    void *plugin_file;
    struct plugin_info *plugin_info;
    int (*plugin_get_main_function)(const char *fname, struct option *in_opts[], size_t in_opts_len, char *out_buff, size_t out_buff_len);
    int option_number;
    int found_option;
};
struct plugin *myPlugins;
int getopt_long(int argc, char * const argv[], const char *optstring, const struct option *longopts, int *longindex);
int makeLog(char *mestringFictage)
{
    fprintf(log, "%s\n", mestringFictage);
    return 0;
}
int addOPt(struct option *array, char *namex, int has_argx, int *flagx, int valx)
{
    if (array == NULL)
    {
        printf("couldn't add option\n");
        return 1;
    }
    array[countOfOptions].name = namex;
    array[countOfOptions].has_arg = has_argx;
    array[countOfOptions].flag = flagx;
    array[countOfOptions].val = valx;
    countOfOptions++;
    return 0;
}
int plugin_append(struct plugin *myPlugins, int position, char *s)
{
    char *stringFict = malloc(sizeof(2 + strlen(s)));
    strncat(stringFict, "./", 2);
    strncat(stringFict, s, strlen(s));
    myPlugins[position].plugin_file = dlopen(stringFict, RTLD_LAZY||RTLD_GLOBAL);
    if (myPlugins[position].plugin_file == NULL)
    {
        printf("error 01 (could not open plugin)\n");
        exit(1);
    }
    else
    {
        myPlugins[position].plugin_info = malloc(sizeof(struct plugin_info));
        get_info = dlsym(myPlugins[position].plugin_file, "plugin_get_info");
        plugin_get_main_function = dlsym(myPlugins[position].plugin_file, "plugin_process_file");
        int err_check;
        err_check = get_info(myPlugins[position].plugin_info);
        if (err_check == 1) {
            printf("error 01 (could not open plugin)\n");
            exit(1);
        }
        else
        {
            myPlugins[position].option_number = countOfOptions;
            arrayOfOptions[countOfOptions].name = myPlugins[position].plugin_info->sup_opts->opt.name;
            arrayOfOptions[countOfOptions].val = myPlugins[position].plugin_info->sup_opts->opt.val;
            countOfOptions++;
            printf("found: %s\n", myPlugins[position].plugin_info->plugin_name);
            printf("option: %s\n", myPlugins[position].plugin_info->sup_opts->opt.name);
        }
        if (plugin_get_main_function != NULL)
        {
            myPlugins[position].plugin_get_main_function = plugin_get_main_function;
            myPlugins[position].found_option = 0;
        }
        else
        {
            printf("error 02 (could not add plugin)\n");
            exit(1);
        }
    }
    return 0;
}
int descrShow(int x)
{
    printf("description\n");
    printf("-h help\n");
    printf("-v version\n");
    printf("-P path to plugins directory\n");
    printf("-l log file\n");
    printf("-C condition\n");
    printf("-N inversion\n");
    exit(0);
}
int show_version(int x)
{
    printf("Version of program  %s\n", Version);
    return 0;
}
int processSearch (const char * dir_name, struct plugin *p, int plugin_count);
int main(int argc, char* argv[]) {
    int path_Option_Flag = 0;
    const char* short_options = "-:P:l:C:Nvh";
    arrayOfOptions = malloc(sizeof(struct option) * 12);
    //int *option_flags = malloc(sizeof(int) * 12);
    char *myPlugins_path = "./";
    int cond; // AND = 0, OR = 1
    char *log_path = malloc(sizeof(char) * 100);
    log_path = ".";
    addOPt(arrayOfOptions, "help", no_argument, NULL, 'h');
    addOPt(arrayOfOptions, "version", no_argument, NULL, 'v');
    addOPt(arrayOfOptions, "inversion", no_argument, NULL, 'N');
    addOPt(arrayOfOptions, "path", required_argument, NULL, 'P');
    addOPt(arrayOfOptions, "logpath", required_argument, NULL, 'l');
    addOPt(arrayOfOptions, "condition", required_argument, NULL, 'C');
    int rez;
    int option_index = -1;
    int flag_plugin_option_found = 0;
    while ((rez = getopt_long(argc, argv, short_options, arrayOfOptions, &option_index)) != -1) {
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
                    myPlugins_path = optarg;
                } else {
                    printf("error 03 (missing argument)\n");
                    exit(1);
                }
                break;
            };
            case 'l': {
                if (optarg) {
                    log_path = optarg;
                } else {
                    printf("Eerror 03 (missing argument)\n");
                    exit(1);
                }
                break;
            }
            case 'N':{
                foundInversion++;
                break;
            }
            case 'C':{
                foundCondition++;
                condition = optarg;
                if (condition == NULL)
                {
                    condition = "AND";
                }
                break;
            }
            case ':':{
                printf("error 03 (missing argument)\n");
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
    if (foundInversion != 0)
    {
        makeLog("Inversion: On");
    }
    option_index = -1;
    optind = 0;
    struct plugin_info *ppi = malloc(sizeof(struct plugin_info));
    DIR *myPlugins_directory;
    myPlugins_directory = opendir(myPlugins_path);
    if (myPlugins_directory == NULL)
    {
        printf("error 04 (could not open directory with plugins)\n");
        makeLog("error 04 (could not open directory with plugins");
        fclose(log);
        exit(1);
    }
    struct plugin *myPlugins;
    struct dirent *file_current;
    myPlugins = malloc(sizeof(struct plugin) * 8);
    int myPlugins_count = 0;
    char *so = ".so";
    while ((file_current = readdir(myPlugins_directory)) != NULL)
    {
        if (strstr(file_current->d_name, so) != NULL)
        {
            plugin_append(myPlugins, myPlugins_count, file_current->d_name);
            makeLog("Found plugin");
            makeLog("Plugin added");
            myPlugins_count++;
        }
    };
    for (int i = 6; i < 6 + myPlugins_count; i++)
    {
        arrayOfOptions[i].has_arg = required_argument;
        arrayOfOptions[i].flag = NULL;
    }
    int found_plugin_options = 0;
    int its_ok = 0;
    addOPt(arrayOfOptions, NULL, 0, NULL, 0);

    while ((rez = getopt_long(argc, argv, "+:P:l:C:Nvh", arrayOfOptions, &option_index)) != -1) {
        switch (rez) {
            case ':':
            {
                printf("error 03 (missing argument)\n");
                makeLog("error 03 (missing argument)");
                fclose(log);
                exit(1);
            }

            case 'h':
            {
                descrShow(0);
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
                makeLog("Found plugin option");
                for (int i = 0; i < myPlugins_count; i++)
                {
                    if ((strcmp(myPlugins[i].plugin_info->sup_opts[0].opt.name, arrayOfOptions[option_index].name) == 0) && (myPlugins[i].found_option == 0))
                    {
                        its_ok = 1;
                        flag_plugin_option_found++;
                        if (optarg == NULL)
                        {
                            printf("error 03 (missing argument)\n");
                            makeLog("error 03 (missing argument)");
                            fclose(log);
                            exit(1);
                        }
                        else
                        {
                            myPlugins[i].plugin_info->sup_opts->opt.flag = optarg;
                            myPlugins[i].found_option = 1;
                            found_plugin_options++;
                        }
                    }
                }
                if (its_ok == 0)
                {
                    printf("error 05 (unknown option)\n");
                    makeLog("error 05 (unknown option)");
                    fclose(log);
                    exit(1);
                }
                break;

            }
                if (flag_plugin_option_found == 0)
                {
                    makeLog("Error: plugin option not found");
                    fclose(log);
                    exit(1);
                }
            case '?':
            {
                printf("error 05 (unknown option)\n");
                makeLog("error 05 (unknown option)");
                fclose(log);
                exit(1);
            }
            default: {
                printf("error 05 (unknown option)\n");
                makeLog("error 05 (unknown option)");
                fclose(log);
                exit(1);
                break;
            };
        };
    };
    if (flag_plugin_option_found == 0)
    {
        printf(" options not found\n");
        exit(1);
    }
    makeLog("\n");
    int processSearch_checker = processSearch(argv[argc - 1], myPlugins, myPlugins_count);
    printf("all:\t%d\n", truthX);
    makeLog("finished");
    fclose(log);
    return 0;
}
int processSearch (const char * dir_name, struct plugin *p, int plugin_count)
{
    int xxx;
    DIR * d;
    d = opendir (dir_name);
    if (d == NULL)
    {
        if (truthX != 0)
        {
            return 0;
        }
        printf("no path\n");
        makeLog("No path");
        descrShow(0);
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
        if (strcmp(condition, "OR") == 0) {
            fl_veritas = 0;
        }
        if (! (entry->d_type & DT_DIR))
        {
            char *gh;
            gh = malloc(sizeof(char) * 355);
            snprintf(gh, 123, "%s/%s", dir_name, d_name);
            makeLog("Opening file");
            makeLog(gh);
            if (strstr(gh, ".txt") != NULL)
            {
                for (int i = 0; i < plugin_count; i++)
                {
                    if (p[i].found_option == 1)
                    {
                        xxx = p[i].plugin_get_main_function(gh, &p[i].plugin_info->sup_opts, p[i].plugin_info->sup_opts_len, buff, buff_size);
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
                if ((fl_veritas == 1) && (foundInversion == 0))
                {
                    printf("file: \t%s\n", gh);
                    makeLog("Found");

                    truthX++;
                }

                if ((fl_veritas == 0) && (foundInversion != 0))
                {
                    printf("file: \t%s\n", gh);
                    makeLog("Found");
                    truthX++;
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
                processSearch(path, p, plugin_count);
            }
        }
    }
    if (closedir (d)) {
        fprintf (stderr, "Could not close '%s': %s\n",
                 dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }
}