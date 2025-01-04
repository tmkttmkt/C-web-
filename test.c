#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "sockets.h"
#include "response.h"
#define PORT 8080

// プロトタイプ宣言
void *handle_client(void *arg);

int main() {
    int server_fd, *client_fd;
    struct sockaddr_in server_addr;
    pthread_t thread_id;

    // ソケットの作成
    server_fd = create_server_socket();

    // アドレスとポートの設定
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(5000);

    // ソケットのバインド
    bind_server_socket(server_fd,&server_addr);

    // リスン状態にする
    start_listening(server_fd, 10);

    printf("Server is running on http://localhost:8080\n");

    // 接続を受け入れて処理
    while (1) {
        client_fd = malloc(sizeof(int)); // クライアントソケットを格納するメモリを確保
        if (!client_fd) {
            perror("Malloc failed");
            continue;
        }

        *client_fd = accept(server_fd, NULL, NULL);
        if (*client_fd < 0) {
            perror("Accept failed");
            free(client_fd);
            continue;
        }

        // クライアント処理のスレッドを作成
        if (pthread_create(&thread_id, NULL, handle_client, client_fd) != 0) {
            perror("Thread creation failed");
            close(*client_fd);
            free(client_fd);
        } else {
            pthread_detach(thread_id); // スレッドのリソースを自動解放
        }
    }

    close(server_fd);
    return 0;
}
// クライアントからのリクエストを処理する関数
void *handle_client(void *arg) {
    int client_fd = *(int *)arg; 
    char buffer[1024] = {0};

    // クライアントからのデータを受信
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
    if (bytes_read < 0) {
        perror("Read failed");
        close(client_fd);
        return NULL;
    }
    buffer[bytes_read] = '\0';  // 念のため文字列の終端を追加
    printf("Received request:\n%s\n", buffer);

    // リクエスト内容に基づいて応答を変更
    handle_request(buffer, client_fd);
    // クライアントソケットを閉じる
    close(client_fd);
    return NULL; 
}