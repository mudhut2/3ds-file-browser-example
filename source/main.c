#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_FILES 512

typedef struct {
	char name[256];
	int isDir;
} Entry;

Entry entries[MAX_FILES];
int entryCount = 0;
int selected = 0;

void loadDirectory(const char* path) {
    entryCount = 0;
    DIR* dir = opendir(path);
    if (!dir) return;

    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL && entryCount < MAX_FILES) {
        snprintf(entries[entryCount].name, sizeof(entries[entryCount].name), "%s", ent->d_name);

        // Check if directory
        if (ent->d_type == DT_DIR) {
            entries[entryCount].isDir = 1;
        } else if (ent->d_type == DT_UNKNOWN) {
            // fallback using stat()
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, ent->d_name);

            struct stat st;
            if (stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) {
                entries[entryCount].isDir = 1;
            } else {
                entries[entryCount].isDir = 0;
            }
        } else {
            entries[entryCount].isDir = 0;
        }
        entryCount++;
    }
    closedir(dir);
}

int main(int argc, char* argv[])
{
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    const char* basePath = "sdmc:/sounds";
    char currentPath[512];
    strcpy(currentPath, basePath);

    loadDirectory(currentPath);

    while (aptMainLoop())
    {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START) break; // exit
        if (kDown & KEY_DOWN) selected = (selected + 1) % entryCount;
        if (kDown & KEY_UP)   selected = (selected - 1 + entryCount) % entryCount;

        if (kDown & KEY_B) {
            // go up one folder
            char* lastSlash = strrchr(currentPath, '/');
            if(lastSlash && lastSlash > currentPath) {
                *lastSlash = '\0';
                loadDirectory(currentPath);
                selected = 0;
            }
        }

        if(kDown & KEY_A) {
            if (entries[selected].isDir) {
                char newPath[512];
                snprintf(newPath, sizeof(newPath), "%s/%s", currentPath, entries[selected].name);
                loadDirectory(newPath);
                strcpy(currentPath, newPath);
                selected = 0;
            } else {
                printf("Selected file: %s/%s\n", currentPath, entries[selected].name);
            }
        }

        consoleClear();
        printf("Browsing: %s\n", currentPath);
        for(int i = 0; i < entryCount; i++){
            if(i == selected) printf("-> %s\n", entries[i].name);
            else              printf("   %s\n", entries[i].name);
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}

