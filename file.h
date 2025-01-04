#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sockets.h"
typedef struct Filepaths {
    char *file;
    struct Filepaths *pathlist;
} Filepaths ;
typedef struct Breadcrumb {
    char *label;        // 表示するラベル
    struct Breadcrumb *next; // 次のパンくずリストへのポインタ
} Breadcrumb;
 Filepaths file_structure[] = {
    {"cmd", (Filepaths[]){{"save", NULL}, {"exe", NULL}, {"com", NULL}, {NULL, NULL}}},
    {"root", (Filepaths[]){{NULL, NULL}}},
    {NULL,NULL}
};
Filepaths root={
    "",
    file_structure
};
void free_filepaths(Filepaths *node) {
    if (node == NULL) {
        return; // ベースケース: ノードがNULLの場合は何もしない
    }

    // サブディレクトリやファイルのリストを再帰的に解放
    free_filepaths(node->pathlist);

    // 現在のノードのfileを解放
    free(node->file);

    // 現在のノード自体を解放
    free(node);
}
Breadcrumb* create_breadcrumb(char *label) {
    Breadcrumb *new_breadcrumb = (Breadcrumb *)malloc(sizeof(Breadcrumb));
    if (new_breadcrumb == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    new_breadcrumb->label = strdup(label); // ラベル文字列をコピー
    if (new_breadcrumb->label == NULL) {
        perror("Failed to allocate memory for label");
        free(new_breadcrumb);
        exit(EXIT_FAILURE);
    }
    new_breadcrumb->next = NULL;
    return new_breadcrumb;
}
void print_breadcrumbs(Breadcrumb *head) {
    Breadcrumb *current = head;
    printf("\n||");
    while (current != NULL) {
        printf("%s", current->label);
        if (current->next != NULL) {
            printf(" > "); // 次の項目があれば区切りを追加
        }
        current = current->next;
    }
    printf("\n"); // 改行
}
void free_breadcrumb(Breadcrumb *head) {
    Breadcrumb *current = head;
    while (current != NULL) {
        Breadcrumb *next = current->next;
        free(current->label);
        free(current);
        current = next;
    }
}

Breadcrumb *GenerateBreadcrumb(const char *str){
    const char *start=str;
    const char *searchstr;
    char liststr[128]={'\0'};
    char filename[32]={'\0'};
    int n;
    int flg=0;
    Filepaths *oya_filepoint=&root;
    Filepaths *ko_filepoint;
    Breadcrumb *now=NULL;
    Breadcrumb *ret=NULL;
    while(*str!='/'){str++;};
    str++;
    searchstr=str;
    while (*searchstr!=' ')
    {
        n=0;
        while (*searchstr!='/'){
            filename[n]=*searchstr;
            searchstr++;
            n++;
            if(*searchstr==' '){
                flg=1;
                break;
            }
        }
        searchstr++;
        filename[n]='\0';
        ko_filepoint=oya_filepoint->pathlist;
        n=0;
        while (ko_filepoint[n].file!=NULL){
            if(strcmp(ko_filepoint[n].file,filename)==0){
                if(now==NULL){
                    now=create_breadcrumb(ko_filepoint[n].file);
                    ret=now;
                }
                else{
                    now->next=create_breadcrumb(ko_filepoint[n].file);
                    now=now->next;
                }
                oya_filepoint=ko_filepoint;
            }
            n++;
        }
        if(flg)break;
    }
    return ret;


}