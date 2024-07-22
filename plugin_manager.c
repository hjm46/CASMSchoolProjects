#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

typedef struct
{
    int (*initialize)(void);
    int (*run)(void);
    int (*cleanup)(void);
} func_pointers;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("ERROR: NO PLUGIN GIVEN\n");
        return -1;
    }

    char* path;
    char* start = "./";
    char* end = ".so";

    path = malloc(strlen(start)+strlen(argv[1])+1);
    if(path == NULL)
    {
        printf("ERROR: malloc failed");
        return 1;
    }

    path[0] = '\0';
    strcat(path,start);
    strcat(path,argv[1]);

    if (strstr(path, end) == NULL)
        strcat(path, end);
    
    void *handle = dlopen(path, RTLD_LAZY);
    if (handle == NULL)
    {
        fprintf(stderr, "ERROR: CANNOT OPEN FILE %s \n %s\n", argv[2], dlerror());
        return 1;
    }

    func_pointers* pointers = (func_pointers*)malloc(sizeof(func_pointers));
    if(pointers == NULL)
    {
        printf("ERROR: could not allocate memory");
        return 1;
    }

    pointers->initialize = dlsym(handle, "initialize");
    pointers->run = dlsym(handle, "run");
    pointers->cleanup = dlsym(handle, "cleanup");

    (*pointers->initialize)();
    (*pointers->run)();
    (*pointers->cleanup)();

    free(pointers);
    dlclose(handle);

    return 0;
}