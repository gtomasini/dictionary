// translator.c : This file contains the 'main' function. Program execution begins and ends there.
//

//way of use, from command line, direct way
//
//translator.exe - s pl2eng.txt - i toTranslate_in.txt - o translated_out.txt -k d
//
//where:
//          pl2eng.txt is the translation dict where each line is "original_word translated_word"
//          toTranslate_in.txt is the input text file name with the text to translate, try to avoid rare characters here.
//          translated_out.txt is the output text file name with the translated text, be carrefull because the program overwrites it if exist
//          d here means direct translation so it maps from original_word to translated_word, you can use i instead in order to do inverse
//
//

//this is because MSVC complains about everithing
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>    
#include <string.h>
#include <stdbool.h>

void ignore_blanks(char* p) {
    for (; *p == ' ' || *p == '\t' || *p == '.' || *p == '-' || *p == '_'; ++p);
};

struct my_entry{
    char *key;
    char *val;
    struct my_entry* nextone;
};

//I kwon this is local global, but in order to avoid so many parameters in the functions
static struct my_entry* dic_ptr = NULL;     //dictionary list, first node ptr
static struct my_entry* lastone_ptr = NULL  ;//dictionary list last node ptr in order to save time adding elements

void add_entry(const char* key, const char *val) {
    struct my_entry* aux = malloc(sizeof(struct my_entry));
    aux->key = malloc(strlen(key) + 1);
    strncpy(aux->key, key, strlen(key)+1);
    aux->val = malloc(strlen(val) + 1);
    strncpy(aux->val, val, strlen(val) +1);
    aux->nextone = NULL;

    if (dic_ptr == NULL) {
        dic_ptr = aux;
        lastone_ptr = aux;
    }
    else {
        lastone_ptr->nextone = aux;
        lastone_ptr = aux;
    }
}

//case insensitive search 
const char* find_entry(const char* key) {
    struct my_entry* aux = dic_ptr;
    for (; aux != NULL; aux = aux->nextone) {
        if (!_stricmp(key, aux->key))
            return aux->val;
    }
    return NULL;
}

//print list
void show_list() {
    struct my_entry* aux = dic_ptr;
    printf("DICTIONARY LIST:\n");
    for (; aux != NULL; aux = aux->nextone) 
        printf("%s -> %s \n", aux->key, aux->val);
}

//free memory
void free_list() {
    struct my_entry* aux = dic_ptr;
    for (; aux != NULL; aux = aux->nextone) {
        free(aux);
    }
}

//read input dict file (key->value)
int read_dict(const char* file, bool direct) {
    FILE* fpi;
    errno_t err = fopen_s(&fpi, file, "r");
    if (err) {
        printf("eror %d, can't open dictionary %s\n", err, file);
        return -1;
    }

    for (int i = 0;; ++i) {//read dictionary
        char buf[128];
        if (fgets(buf, 128, fpi) == NULL) break;
        if (strlen(buf) - 1 >=0 && buf[strlen(buf) - 1] == '\n')
            buf[strlen(buf) - 1] = 0;
        //printf("dic, read: \"%s\"\n", buf);
        char* p = strpbrk(buf, " \t");
        if (p == NULL) {
            printf("error in line %d, %s\n", i, buf);
            continue;
        }
        *p++ = 0;
        ignore_blanks(p);
        printf("dic entry: %s -> %s\n", buf, p);
        if (direct)
            add_entry(buf, p);
        else
            add_entry(p, buf);
    }
    fclose(fpi);
    show_list();
    return 0;
}

int main(int argc, char **argv){
    printf("Translator 0.1\n");
    printf("\tplease write: %s -s dict.txt -i input.txt -o output.txt -k d|i\n", argv[0] );
    printf("\ti.e.: %s -i toTranslate.txt -o output.txt -k d -s pl2eng.txt\n", argv[0]);

    char ifile[64];
    strncpy (ifile, "input.txt", 64);//default input file

    char dic[64];
    strncpy(dic, "dic.txt", 64); //default dictionary

    char ofile[64];
    strncpy(ofile, "translated.txt", 64);

    bool direct = true;//if true direct way translation, if false inverse way

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-' && (i&0x01) == 0x01 && (i+1)<argc) {
            switch (argv[i][1]) {
            case 'i':
                strncpy(ifile, argv[++i], 64);
                break;

            case 's':
                strncpy(dic, argv[++i], 64);
                break;

            case 'o':
                strncpy(ofile, argv[++i], 64);
                break;

            case 'k':
                direct = (argv[++i][0] == 'd');
                break;

            default:
                printf( "error not valid flag: -%s\n", argv[i]);
                return -1;//aborting
            }
        }
    }
    printf("\ninput text file: %s\n", ifile);
    printf("output text file: %s, warning it will be overwerite.\n", ofile);
    printf("text dictionary input: %s\n", dic);
    printf("direct translation way: %d\n", direct);
    read_dict(dic, direct);

    //just 4 test, in direct way it look for 'cat'
    if (direct)
        printf("just 4 test dict:\n%s->%s\n", "kot", find_entry("kot"));
    else
        printf("just 4 test dict:\n%s->%s\n", "cat", find_entry("cat"));
    
    FILE* fpi;
    errno_t err = fopen_s(&fpi, ifile, "r");
    if (err) {
        printf("error %d, can't open input file %s\n", err, ifile);
        return -1;
    }

    FILE* fpo;
    err = fopen_s(&fpo, ofile, "w");
    if (err) {
        printf("error, can't open output file %s\n", ofile);
        return -1;
    }

    //translating input file to output line by line, you can put this in a function perhaps
    const char *ignore=" \t. ? -!_()[]*&${}";
    for (int i = 0;; ++i) {//read input file
        char buf[200];
        if (fgets(buf, 200, fpi) == NULL) break;
        buf[strlen(buf) - 1] = ' ';
        printf( "input, line read: \"%s\"\n", buf);
        char obuf[300];
        obuf[0] = 0;

        char* p1 = buf;
        char* p2 = strpbrk(p1, ignore);
        
        for (; p2 != NULL; ) {
            char aux[2];
            aux[0] = *p2;
            aux[1] = 0;
            *p2++ = 0;
            ignore_blanks(p2);
            if (p1[0] == 0) break;
            const char *key=p1;
            const char* val = find_entry(key);
            printf("***original word:\'%s\', translated to:\t\t \'%s\'\n", key, val);
            if (val == NULL) {//not translation
                strcat(obuf, "[");
                strcat(obuf, key);
                strcat(obuf, "]");
            }
            else 
                strcat(obuf, val);
            strcat(obuf, aux);//add original separator
            p1 = p2;
            p2 = strpbrk(p1, ignore);
        }
        printf("******translated line:\'%s\'\n", obuf);
        strcat(obuf, "\n");
        fputs(obuf, fpo);//writes to output
    }

    fclose(fpi);
    fclose(fpo);
    free_list();
    return 0;
}

