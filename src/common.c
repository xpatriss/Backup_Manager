#include "common.h"

void usage(char* pname, char* args)
{
    fprintf(stderr, "USAGE: %s %s\n", pname, args);
    //exit(EXIT_FAILURE);
}

char* join_path(char *dir, char *name) {
    // 1. Obliczamy długość: długość_katalogu + '/' + długość_pliku + '\0'
    size_t len = strlen(dir) + strlen(name) + 2; 

    // 2. Alokujemy pamięć
    char *result = malloc(len);
    if (result == NULL) {
        ERR("malloc");
    }

    // 3. Sklejamy bezpiecznie używając snprintf
    // Format "%s/%s" automatycznie dodaje slash pomiędzy
    snprintf(result, len, "%s/%s", dir, name);

    return result;
}
/*
void* safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        ERR("malloc");
    }
    return ptr;
}
*/