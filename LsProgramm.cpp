#include <stdio.h>
#include <cstdlib> 
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>

static bool l_flag = false;
static bool a_flag = false;
static bool r_flag = false;
// Можно добавить флаги по мере необходимости

int translate_argv(int argc, char** argv, int* path_index){

    int path_num = 0;
    
    for(int i = 1; i < argc; i++){

        if(argv[i][0] != '-'){
            path_index[path_num] = i;
            path_num++;
            continue;
        }
        
        if(sizeof(argv[i]) > 1){
            switch (argv[i][1])
            {
            case 'l':
                l_flag = true;
                break;
            case 'a':
                a_flag = true;
                break;
            case 'r':
                r_flag = true;
                break;
            //Можно добавить флаги по мере необходимости
            default:
                break;
            }
        }
    }
    
    return path_num;
}

void convert_timespec_to_local_date(timespec filetime, char* buf, int buf_size){

   struct tm formtedtime = *localtime(&filetime.tv_sec);
    
    setlocale (LC_TIME, "");
    time_t curtime = time(NULL);

    if(formtedtime.tm_year == localtime(&curtime)->tm_year)
        strftime(buf, buf_size,"%b %d %H:%M",&formtedtime);
    else
        strftime(buf, buf_size,"%b %d %Y",&formtedtime);
        
}

int compare_asc(const void *a, const void *b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

int compare_desc(const void *a, const void *b) {
    return strcmp(*(const char**)b, *(const char**)a);
}

char** get_files_names(const char *path, int *count) {
    DIR *dir;
    struct dirent *entry;
    char **files = NULL;
    int capacity = 10;
    int size = 0;
    
    dir = opendir(path);
    
    // Выделяем начальный массив
    files = (char**)malloc(capacity * sizeof(char*));
    if (files == NULL) {
        closedir(dir);
        return NULL;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        
        if (!a_flag && entry->d_name[0] == '.') 
            continue;
        
        if (size >= capacity) {
            capacity *= 2;
            char **new_files = (char**)realloc(files, capacity * sizeof(char*));

            if (new_files == NULL) {

                for (int i = 0; i < size; i++) {
                    free(files[i]);
                }

                free(files);
                closedir(dir);
                return NULL;
            }
            files = new_files;
        }
        
        files[size] = strdup(entry->d_name);
        
        if (files[size] == NULL) {
            // Обработка ошибки выделения памяти
            for (int i = 0; i < size; i++) {
                free(files[i]);
            }
            free(files);
            closedir(dir);
            return NULL;
        }
        
        size++;
    }
    
    closedir(dir);
    *count = size;

    if(!r_flag)
        qsort(files, *count, sizeof(char*), compare_asc);
    else
        qsort(files, *count, sizeof(char*), compare_desc);
    
    return files;
}

void free_files_names(char **files, int count) {
    
    if (files == NULL) return;
    
    for (int i = 0; i < count; i++)
        free(files[i]);

    free(files);
}


void process_folder(char* path){

    DIR* curdirectory;
    int count = 0;

    if((curdirectory = opendir(path)) == NULL){
        printf("\nНевозможно открыть деректорию %s\n", path);
        return;
    }

    char** files = get_files_names(path, &count);

    if(!files){
         printf("Ошибка получения списка файлов %s\n", path);
        return;
    }

    printf("\nСписок файлов в деректории %s:\n",path);

    for(int i = 0; i < count; i++){
        char flink[256];
        struct stat fileinfo;

        sprintf(flink,"%s/%s",path,files[i]);//file->d_name);

        if(l_flag){

            if(!stat(flink,&fileinfo)){

                char time_buffer[50];

                printf( (fileinfo.st_mode & S_IRUSR) ? " r" : " -");
                printf( (fileinfo.st_mode & S_IWUSR) ? "w" : "-");
                printf( (fileinfo.st_mode & S_IXUSR) ? "x" : "-");
                printf( (fileinfo.st_mode & S_IRGRP) ? "r" : "-");
                printf( (fileinfo.st_mode & S_IWGRP) ? "w" : "-");
                printf( (fileinfo.st_mode & S_IXGRP) ? "x" : "-");
                printf( (fileinfo.st_mode & S_IROTH) ? "r" : "-");
                printf( (fileinfo.st_mode & S_IWOTH) ? "w" : "-");
                printf( (fileinfo.st_mode & S_IXOTH) ? "x " : "- ");
                printf("%2d ",fileinfo.st_nlink);
                printf("%3s ",getpwuid(fileinfo.st_uid)->pw_name);
                printf("%3s ",getgrgid(fileinfo.st_gid)->gr_name);
                printf("%10d ",fileinfo.st_size);

                convert_timespec_to_local_date(fileinfo.st_mtim, time_buffer, 50);
                printf("%3s ", time_buffer);
            }
        }
        printf("%s \n",files[i]);
    }

    free_files_names(files, count);
}

int main(int argc, char** argv){

    int path_index[50];
    char path[256];
    int path_num = translate_argv(argc,argv,path_index);

    if(path_num == 0){
        getcwd(path, 256);
        process_folder(path);
        return 0;
    }

    for(int i = 0; i < path_num; i++){
        snprintf(path, 256, "%s", argv[path_index[i]]);
        process_folder(path);
        memset(path,0,sizeof(path));
    }

return 0;
}