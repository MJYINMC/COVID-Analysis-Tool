#include <stdlib.h>
#include <Windows.h>

#include "Data.h"
#include "../libgraphics/genlib.h"
extern BOOL initialized[4];

// 初始化链表
BOOL DataInit(Data ** head, Data ** tail){
    *head = (Data *) malloc(sizeof(Data));
    if(* head == NULL){
		MessageBox(NULL,"内存分配失败!", NULL, MB_ICONERROR);
        return FALSE;
    }else{
        ZeroMemory(*head,sizeof(Data));
        *tail = *head;
        return TRUE;
    }
}
// 添加新节点
Data *  DataAddNewNode(Data * head, Data * tail){
    Data * node = (Data *) malloc(sizeof(Data));
    if(node == NULL){
		MessageBox(NULL,"内存分配失败!", NULL, MB_ICONERROR);
        return 0 ;
    }else{
        ZeroMemory(node,sizeof(Data));
        tail -> next = node;
        head -> items[0]++;
        return node;
    }
}

// 存储数据进入表中(i,j)的位置

BOOL DataStore(Data * head, int value,int i,int j){

    if(i > head -> items [0] || j > head->items[1]){
		MessageBox(NULL,"表格下标越界!", NULL, MB_ICONERROR);
        return FALSE;
    }else{
        for(int k = 0; k < i && head -> next; k++){
            head = head -> next;
        }
        head -> items[j] = value;
    }
    return TRUE;
};

// 获取表中(i,j)位置的数据
char * GetData(Data * head,int i,int j,char * buffer){
    if(i > head -> items [0] || j > head->items[1]){
		MessageBox(NULL,"表格下标越界!", NULL, MB_ICONERROR);
        return NULL;
    }else{
        for(int k = 0; k < i && head -> next; k++){
            head = head -> next;
        }
        if(j==1){
            return head->date;
        }else{
            return itoa(head->items[j],buffer,10);
        }
	}
};
// 获取尾指针
Data * GetTail(Data * head){
    while(head ->next){
        head = head ->next;
    };
    return head;
};
// 摧毁链表
BOOL DataDestroy(Data * head){
    ZeroMemory(initialized,sizeof(initialized));
	Data * p;
    Data * next;
	p = head->next;
	while (p != NULL) {
	    next = p ->next;
		FreeBlock(p);
		p = next;
	}
    ZeroMemory(head,sizeof(Data));
    return TRUE;
};
// 检查输入是否合法
BOOL DataCheck(Data * head){

    while(head = head ->next){
        if(!*(head->date)){
            return FALSE;
        }
    }
    return TRUE;
};