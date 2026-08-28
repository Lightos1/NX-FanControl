#include <fancontrol.hpp>

#define FC_PATH_MAX 64

void CreateDir(const char *dir) {
    char buf[FC_PATH_MAX];
    size_t len = strlen(dir);

    if (len == 0)
        return;
    if (len >= FC_PATH_MAX)
        len = FC_PATH_MAX - 1;

    memcpy(buf, dir, len);
    buf[len] = '\0';

    for (size_t i = 1; i <= len; i++) {
        if (buf[i] == '/' || buf[i] == '\0') {
            char saved = buf[i];
            buf[i] = '\0';
            if (access(buf, F_OK) == -1)
                mkdir(buf, 0777);
            buf[i] = saved;
        }
    }
}

void InitLog(void) {
    if (access(FC_LOG_DIR, F_OK) == -1)
        CreateDir(FC_LOG_DIR);

    if (access(FC_LOG_FILE, F_OK) != -1)
        remove(FC_LOG_FILE);
}

void WriteLog(const char *buffer) {
    FILE *log = fopen(FC_LOG_FILE, "a");
    if (log == NULL)
        return;
    fputs(buffer, log);
    fputc('\n', log);
    fclose(log);
}
