#include <stdlib.h>
#include <Windows.h>

#include "Data.h"
#include "../libgraphics/genlib.h"
extern BOOL initialized[4];

// ��ʼ������
BOOL DataInit(Data ** head, Data ** tail){
    *head = (Data *) malloc(sizeof(Data));
    if(* head == NULL){
		MessageBox(NULL,"�ڴ����ʧ��!", NULL, MB_ICONERROR);
        return FALSE;
    }else{
        ZeroMemory(*head,sizeof(Data));
        *tail = *head;
        return TRUE;
    }
}
// ����½ڵ�
Data *  DataAddNewNode(Data * head, Data * tail){
    Data * node = (Data *) malloc(sizeof(Data));
    if(node == NULL){
		MessageBox(NULL,"�ڴ����ʧ��!", NULL, MB_ICONERROR);
        return 0 ;
    }else{
        ZeroMemory(node,sizeof(Data));
        tail -> next = node;
        head -> items[0]++;
        return node;
    }
}

// �洢���ݽ������(i,j)��λ��

BOOL DataStore(Data * head, int value,int i,int j){

    if(i > head -> items [0] || j > head->items[1]){
		MessageBox(NULL,"����±�Խ��!", NULL, MB_ICONERROR);
        return FALSE;
    }else{
        for(int k = 0; k < i && head -> next; k++){
            head = head -> next;
        }
        head -> items[j] = value;
    }
    return TRUE;
};

// ��ȡ����(i,j)λ�õ�����
char * GetData(Data * head,int i,int j,char * buffer){
    if(i > head -> items [0] || j > head->items[1]){
		MessageBox(NULL,"����±�Խ��!", NULL, MB_ICONERROR);
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
// ��ȡβָ��
Data * GetTail(Data * head){
    while(head ->next){
        head = head ->next;
    };
    return head;
};
// �ݻ�����
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
// ��������Ƿ�Ϸ�
BOOL DataCheck(Data * head){

    while(head = head ->next){
        if(!*(head->date)){
            return FALSE;
        }
    }
    return TRUE;
};