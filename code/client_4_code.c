#include <stdio.h>
#include <windows.h>

int main() {
    // "Hello, World!" を表示
    printf("Hello, World!\n");

    // 3秒間スリープ（3000ミリ秒）
    Sleep(3000); // 3000ミリ秒 = 3秒

    // キー入力を待つ
    printf("Press any key to continue...\n");
    getchar();  // ユーザーの入力を待つ

    return 0;
}


