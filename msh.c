#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

//plugin manager

typedef struct
{
    char name[20];
    int (*initialize)(void);
    int (*run)(char **argv);
} active_plugins;

static active_plugins* plugin_structs[20];
static int count = 0;


void leave(active_plugins** plugins)
{
    for(int i = 0; i <= count; i++)
    {
        free(plugins[i]);
    }
    exit(0);
}

active_plugins* find_plugin(char* arg, active_plugins** plugins)
{
    for (int i = 0; i < count; i++)
    {
        if(strstr(plugins[i]->name, arg) != NULL)
            return plugins[i];
    }
    return NULL;
}

active_plugins* load(char* args[])
{
    if ((count >= 10) || (args[1] == NULL))
    {
        printf("Error: Plugin %s initialization failed!\n", args[1]);
        return NULL;
    }

    char* path;
    char* start = "./";
    char* end = ".so";

    path = malloc(strlen(start)+strlen(args[1])+1);
    if(path == NULL)
    {
        printf("ERROR: malloc failed\n");
        return NULL;
    }

    path[0] = '\0';
    if (strstr(path, start) == NULL)
    {
        strcat(path,start);
        strcat(path,args[1]);
    }

    if (strstr(path, end) == NULL)
        strcat(path, end);
    
    void *handle = dlopen(path, RTLD_LAZY);
    if (handle == NULL)
    {
        printf("Error: Plugin %s initialization failed!\n", args[1]);
        return NULL;
    }

    active_plugins* plugin = find_plugin(path, plugin_structs);
    if (plugin != NULL)
    {
        printf("Error: Plugin %s initialization failed!\n", args[1]);
        return NULL;
    }

    active_plugins* new_plugin = malloc(sizeof(active_plugins));
    if (new_plugin == NULL)
    {
        printf("Error: Plugin %s initialization failed!\n", args[1]);
        return NULL;
    }

    strcpy(new_plugin->name, path);

    new_plugin->initialize = dlsym(handle, "initialize");
    new_plugin->run = dlsym(handle, "run");

    if ((*new_plugin->initialize)() != 0)
    {
        printf("Error: Plugin %s initialization failed!\n", args[1]);
        free(path);
        free(new_plugin);
        return NULL;
    }

    count++;
    free(path);
    return new_plugin;

}

void run_plugin(char* args[], active_plugins* plugin)
{
    (*plugin->run)(args);
    return;
}


int main()
{
    char str_original[201];
    char* args[20];
    char* token;
    char* delim = " \n";

    while(1)
    {
        printf(">");
        fgets(str_original, 201, stdin);
        int i = 0;
        token = strtok(str_original, delim);
        while(token != NULL)
        {
            args[i] = token;
            i++;
            token =strtok(NULL, delim);
        }

        if (strcmp(args[0], "exit") == 0)
        {
            leave(plugin_structs);
        }

        else if (strcmp(args[0], "load") == 0)
        {
            active_plugins* plugin = load(args);
            if (plugin != NULL)
                plugin_structs[count-1] = plugin;
        }

        else
        {   
            active_plugins* plugin = find_plugin(args[0], plugin_structs);
            if (plugin != NULL)
                run_plugin(args, plugin);

            else
            {
                int pid = fork();
                if (pid == 0)
                    execv(args[0], args);
                else
                    wait(NULL);
            }
        }

    }
}