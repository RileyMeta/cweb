#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdbool.h>

// Custom Header File(s)
#include "parsefile.h"

#define PORT 8080
#define BUFFER_SIZE 8192

// bool show_extensions = false;

char *root_path = "./root";

// Function Prototypes
void handle_get(int socket, const char *filename);
void handle_post(int socket);
void send_response(int socket, int status_code, const char *content_type, const char *body);
void parse_path(const char *request, char *filepath);

typedef enum {
    HTTP_UNKNOWN,
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE
} HttpMethod;

HttpMethod parse_method(const char *request) {
    char method[8];
    sscanf(request, "%s", method);

    if (strcmp(method, "GET") == 0) return HTTP_GET;
    if (strcmp(method, "POST") == 0) return HTTP_POST;
    if (strcmp(method, "PUT") == 0) return HTTP_PUT;
    if (strcmp(method, "DELETE") == 0) return HTTP_DELETE;

    return HTTP_UNKNOWN;
}

void parse_path(const char *request, char *filepath) {
    char method[16], path[256];
    sscanf(request, "%s %s", method, path);

    if (strcmp(path, "/") == 0) {
        strcpy(filepath, "index.html");
    } else {
        snprintf(filepath, 255, "%s", path + 1); // strip leading slash
        
        if (!show_extensions && strchr(filepath, '.') == NULL) {
            strcat(filepath, ".html");
        }
    }
}

char* read_file(const char* filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char *buffer = malloc(length + 1);
    if (!buffer) return NULL;

    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

void send_response(int socket, int status_code, const char *content_type, const char *body) {
    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %lu\r\n"
        "Connection: close\r\n\r\n",
        status_code,
        (status_code == 200) ? "OK" :
        (status_code == 404) ? "Not Found" :
        (status_code == 405) ? "Method Not Allowed" :
        "Bad Request",
        content_type,
        strlen(body)
    );

    send(socket, header, strlen(header), 0);
    send(socket, body, strlen(body), 0);
}

void handle_get(int socket, const char *filename) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", root_path, filename);

    char *content = read_file(path);

    if (!content && !show_extensions && strstr(filename, ".html")) {
        char fallback_path[512];
        strncpy(fallback_path, filename, sizeof(fallback_path));
        fallback_path[sizeof(fallback_path) - 1] = '\0';

        char *ext = strstr(fallback_path, ".html");
        if (ext) {
            strcpy(ext, ".htm");
            snprintf(path, sizeof(path), "%s/%s", root_path, fallback_path);
            content = read_file(path);
        }
    }

    if (content) {
        send_response(socket, 200, "text/html", content);
        free(content);
    } else {
        snprintf(path, sizeof(path), "%s/404.html", root_path);
        char *not_found = read_file(path);
        if (!not_found) not_found = strdup("<h1>404 Not Found</h1>");
        send_response(socket, 404, "text/html", not_found);
        free(not_found);
    }
}

void handle_post(int socket) {
    char path[512];
    snprintf(path, sizeof(path), "%s/post.html", root_path);
    char *content = read_file(path);

    if (!content) content = strdup("<h1>POST received</h1>");
    send_response(socket, 200, "text/html", content);
    free(content);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        root_path = argv[1];
    }

    parse_config("server.config");

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    printf("Server running at http://localhost:%d/\n", PORT);
    // printf("Serving from: %s\n", root_path);
    printf("Show file extensions: %s\n", show_extensions ? "true" : "false");

    do {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        memset(buffer, 0, BUFFER_SIZE);
        read(new_socket, buffer, BUFFER_SIZE - 1);

        HttpMethod method = parse_method(buffer);
        char filepath[256];
        parse_path(buffer, filepath);

        switch (method) {
            case HTTP_GET:
                handle_get(new_socket, filepath);
                break;
            case HTTP_POST:
                handle_post(new_socket);
                break;
            default: {
                char *msg = "<h1>405 Method Not Allowed</h1>";
                send_response(new_socket, 405, "text/html", msg);
                break;
            }
        }

        close(new_socket);
    } while (1);

    return 0;
}
