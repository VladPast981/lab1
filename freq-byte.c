#ifndef _PLUGIN_API_H
#define _PLUGIN_API_H

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <regex.h>

/*
    Структура, описывающая опцию, поддерживаемую плагином.
*/
struct plugin_option {
    /* Опция в формате, поддерживаемом getopt_long (man 3 getopt_long). */
    struct option opt;
    /* Описание опции, которое предоставляет плагин. */
    const char *opt_descr;
};

/*
    Структура, содержащая информацию о плагине.
*/
struct plugin_info {
    /* Название плагина */
    const char *plugin_name;
    /* Длина списка опций */
    size_t sup_opts_len;
    /* Список опций, поддерживаемых плагином */
    struct plugin_option *sup_opts;
};

/*
    Функция, позволяющая получить информацию о плагине.

    Аргументы:
        ppi - адрес структуры, которую заполняет информацией плагин.

    Возвращаемое значение:
        0 - в случае успеха,
        1 - в случае неудачи (в этом случае продолжать работу с этим плагином нельзя).
*/
int plugin_get_info(struct plugin_info* ppi)
{
    if (ppi == NULL)
    {
        return 1;
    }
    ppi->plugin_name = "freq-byte";
    ppi->sup_opts_len = 1;
    struct plugin_option *ip_option = malloc(sizeof(struct plugin_option) * ppi->sup_opts_len);
    ip_option->opt_descr = "Searches files where byte is most popular";
    struct option *ip_opt = malloc(sizeof(struct option));
    ip_opt->name = "freq-byte";
    ip_opt->has_arg = required_argument;
    ip_opt->flag = NULL;
    ip_opt->val = 0;
    ip_option->opt = *ip_opt;

    //ppi->sup_opts = ip_option;

    ppi->sup_opts = malloc(sizeof(struct plugin_option));
    ppi->sup_opts->opt.name = "freq-byte";
    ppi->sup_opts->opt.has_arg = required_argument;
    ppi->sup_opts->opt.flag = NULL;
    ppi->sup_opts->opt.val = 0;

    return 0;
};

/*
    Фунция, позволяющая выяснить, отвечает ли файл заданным критериям.

    Аргументы:
        in_opts - список опций (критериев поиска), которые передаются плагину.
           struct option {
               const char *name;
               int         has_arg;
               int        *flag;
               int         val;
           };
           Поле name используется для передачи имени опции, поле flag - для передачи
           значения опции (в виде строки). Если у опции есть аргумент, поле has_arg
           устанавливается в ненулевое значение.

        in_opts_len - длина списка опций.

        out_buff - буфер, предназначенный для возврата данных от плагина. В случае ошибки
            в этот буфер, если в данном параметре передано ненулевое значение,
            копируется сообщение об ошибке.

        out_buff_len - размер буфера. Если размера буфера оказалось недостаточно, функция
            завершается с ошибкой.

    Возвращаемое значение:
          0 - файл отвечает заданным критериям,
        > 0 - файл НЕ отвечает заданным критериям,
        < 0 - в процессе работы возникла ошибка (код ошибки).
*/

typedef struct
{
    unsigned char sym;
    int count;
} byte_dict;

int plugin_process_file(const char *fname,
                        struct option *in_opts[],
                        size_t in_opts_len,
                        char *out_buff,
                        size_t out_buff_len)
{
    FILE *current_file;
    if (strstr(fname, "share") != NULL)
    {
        return 10;
    }
;
    current_file = fopen(fname, "rb");

    if (current_file == NULL)
    {
        fclose(current_file);

        return -1;
    }

    if (in_opts[0]->flag == NULL)
    {
        printf("Error: missing argument\n");
        fclose(current_file);
        return -1;
    }

    char *byte = (char *) in_opts[0]->flag;
    //printf("%s\n", ip1);
    //mstrcpy(ip,

    int decx = 0;
    int binx = 0;
    int hexx = 0;
    int err,regerr;
    unsigned long byte_ch;
    char *end;
    //printf("byte = %s\n", byte);
    if (strstr(byte, "0x") != NULL)
    {
        hexx++;
        if (strlen(byte) != 4)
        {
            printf("Error: invalid syntax\n");
            strcpy(out_buff, "invalid syntax");
            exit(1);
            //return -1;
        }
        byte_ch = strtol(&byte[2], &end, 16);
        if ((byte_ch == 0) && (strlen(byte) != 10))
        {
            printf("Error: invalid syntax\n");
            strcpy(out_buff, "invalid syntax");
            exit(1);
            //return -1;
        }
    }
    if (strstr(byte, "bx") != NULL)
    {
        binx++;
        if (strlen(byte) != 10)
        {
        	//printf("strlen\n");
            printf("Error: invalid syntax\n");
            strcpy(out_buff, "invalid syntax");
            exit(1);
            //return -1;
        }
        byte_ch = strtol(&byte[2], &end, 2);
        //printf("byte ch = %d\n", byte_ch);
        if ((byte_ch == 0) && (strlen(byte) != 10))
        {
        
            printf("Error: invalid syntax\n");
            strcpy(out_buff, "invalid syntax");
            exit(1);
            //return -1;
        }
    }
    int byteX;
    //printf("hex = %d\n", hexx);
    //printf("bin = %d\n", binx);
    if ((hexx == 0) && (binx == 0))
    {
        decx++;
        //printf("DEC++\n");
        if ((atoi(byte) > 255) || ((atoi(byte) == 0) && (strlen(byte) != 1)))
        {
            printf("Error: invalid syntax\n");
            strcpy(out_buff, "invalid syntax");
            exit(1);
            //return -1;
        }
        byte_ch = strtol(byte, &end, 10);
        //printf("--ch = %d\n", byte_ch);
    }
    byte_dict *dict = malloc(sizeof(byte_dict) * 256);
   
    int i = 0;
    char unsigned ch;
    int max = -1;
    //printf("HHH\n");
    while ((ch = getc(current_file)) != EOF)
    {
    	//printf("ch = %d\n", ch);
    	if (ch == 255)
    	{
    		break;
    	}
        i++;
        if (i > 400)
        {
            //fclose(current_file);
            //return -1;
        }
        dict[ch].count++;
        //printf("dict[ch].count = %d\n", dict[byte_ch].count);
        if (dict[ch].count > max)
        {
            max = dict[ch].count;
        }


    }
    	//printf("ch = %d\n", byte_ch);
	//printf("dict[byte_ch].count = %d\n", dict[byte_ch - 1].count);
    if (dict[byte_ch].count == max)
    {
        return 0;
    }

    fclose(current_file);
    return 10;
};

#endif
