#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sockets.h"
#include "file.h"

// response.h の先頭に関数のプロトタイプを追加
void send_response(int client_fd, const char *status, const char *content_type, const char *body);
void generate_breadcrumbs(char *breadcrumbs, const char *page);
void send_html_file(int client_fd, const char *filepath);
int savecode(int client_fd,char filename[256],const char *pos);
void send_exe_file(int client_fd, const char *exe_filename);

void handle_request(const char *request, int client_fd) {
    // リクエストの先頭行（GET /path HTTP/1.1）の解析
    if (strncmp(request, "GET /", 5) == 0) {
        if (strncmp(request + 5, "favicon.ico", 11) == 0) {
            // Faviconアイコンのリクエストに対する応答
            send_response(client_fd, "404 Not Found", "text/html", "<h1>404 - Favicon not found</h1>");
            printf("<h1>404 - Favicon not found</h1>\n");
        } else if (strncmp(request + 4, "/", 1) == 0) {
            Breadcrumb*pathlist=GenerateBreadcrumb(request);
            print_breadcrumbs(pathlist);
            if (strcmp(pathlist->label, "helloword") == 0){
                send_response(client_fd, "200 Ok", "text/html", "<h1>Hello word</h1>");
            }
            else if (strcmp(pathlist->label, "hellowolrd") == 0){
                send_response(client_fd, "200 Ok", "text/html", "<h1>Hello 世界</h1>");
            }
            else{
                send_html_file(client_fd,"test.html");
            }
        } else {
            // その他のGETリクエストに対する応答
            send_response(client_fd, "404 Not Found", "text/html", "<h1>404 - Page not found</h1>");
            printf("<h1>404 - Page not found</h1>\n");
        }
    } else if (strncmp(request, "POST /", 6) == 0) {
        Breadcrumb*pathlist=GenerateBreadcrumb(request);
        print_breadcrumbs(pathlist);
        if(strcmp(pathlist->next->label, "exe") == 0){
            // POSTリクエストに対する処理（例：フォーム送信）
            send_response(client_fd, "200 OK", "text/html", "<h1>POST EXEEXErequest received</h1>");
            printf("<h1>POST EXEEXErequest received</h1>\n");

        }
        else if (strcmp(pathlist->next->label, "com") == 0)
        {
            const char *pos = strchr(request, '%');
            char filename[256];
            snprintf(filename, sizeof(filename), "code/client_%d_code.c", client_fd);
            if(savecode(client_fd,filename,pos+1)){
                return;
            }
            char exe_filename[256];
            snprintf(exe_filename, sizeof(exe_filename), "exe/client_%d_code.exe", client_fd);
            char command[600];
            snprintf(command, sizeof(command), "bash -c 'x86_64-w64-mingw32-gcc %s -o %s'", filename, exe_filename);
            int ret = system(command);
            // エラーハンドリング
            if (ret != 0) {
                // エラーコードに基づいて適切なエラーメッセージを表示
                if (ret == 127) {
                    printf("Error: bash not found or unable to execute bash\n");
                } else {
                    printf("Command failed with error code: %d\n", ret);
                }

                // 詳細なエラーメッセージを取得（シェル内で実行する）
                FILE *fp = popen("echo $?", "r");
                if (fp == NULL) {
                    printf("Error: Failed to fetch error code from shell\n");
                } else {
                    int error_code;
                    fscanf(fp, "%d", &error_code);
                    printf("Detailed error code from shell: %d\n", error_code);
                    fclose(fp);
                }

            } else {
                printf("Compilation successful.\n");
            }
            FILE *file = fopen(filename, "rb");

            send_exe_file(client_fd, exe_filename);
            printf("file sended exe\n");
        }
        else{
            send_response(client_fd, "404 Not Found", "text/html", "<h1>404 - Page not found</h1>");
            printf("<h1>404 - Page not found</h1>\n");
        }
        
    } else {
        // その他のHTTPメソッドに対する応答（例：405 Method Not Allowed）
        send_response(client_fd, "405 Method Not Allowed", "text/html", "<h1>405 - Method Not Allowed</h1>");
        printf("<h1>405 - Method Not Allowed</h1>\n");
    }
}

// HTTPレスポンスを送信する関数
void send_response(int client_fd, const char *status, const char *content_type, const char *body) {
    char response[8192]={'\0'};
    // HTTPステータス、ヘッダー、ボディを組み立て
    snprintf(response, sizeof(response),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s; charset=utf-8\r\n"
        "Content-Length: %lu\r\n"
        "\r\n"
        "%s",
        status, content_type, strlen(body), body);
    // レスポンスをクライアントに送信
    write(client_fd, response, strlen(response));
}

void send_html_file(int client_fd, const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        // ファイルが開けない場合は404エラーを送信
        send_response(client_fd, "404 Not Found", "text/html", "<h1>404 - File Not Found</h1>");
        return;
    }

    // ファイルの内容を取得
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_content =(char *)malloc(file_size + 1);
    if (file_content == NULL) {
        // メモリ確保失敗時はエラーを送信
        send_response(client_fd, "500 Internal Server Error", "text/html", "<h1>500 - Internal Server Error</h1>");
        fclose(file);
        return;
    }

    fread(file_content, 1, file_size, file);
    file_content[file_size] = '\0'; // NULL終端

    // ファイル内容をクライアントに送信
    send_response(client_fd, "200 OK", "text/html", file_content);
    printf("file sended\n");
    // 後片付け
    free(file_content);
    fclose(file);
}

int savecode(int client_fd,char filename[256],const char *pos){
    if(pos==NULL)return -1;
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open file");
        return -1;
    }
    fprintf(file, "%s", pos);
    fclose(file);
    return 0;
}
void send_exe_file(int client_fd, const char *filename) {
    // ファイルをバイナリモードで開く
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    // ファイルサイズを取得
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // ファイルの内容を読み込むためのバッファ
    char *body = (char *)malloc(file_size);
    if (body == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return;
    }

    // ファイルの内容を読み込む
    fread(body, 1, file_size, file);
    fclose(file);

    // レスポンスのヘッダーとボディを作成
    char response[4096];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Length: %ld\r\n"
        "\r\n",
        file_size);

    // レスポンスを送信（ヘッダー + バイナリデータ）
    write(client_fd, response, strlen(response));  // ヘッダー部分
    write(client_fd, body, file_size);              // バイナリデータ部分

    // メモリ解放
    free(body);
}