#ifndef _Data_H_
#define _Data_H_

#define MAX_ITEMS 50
#define MAX_DAYS 100
typedef struct data {
	char date [15];// ���ַ�����ʽ�洢����

	int items[MAX_ITEMS];// ���� items[0]��ʾ������ΪԤ�⻹����ʵ�� 
	// items[0] = 0 ��ʵ
	// items[0] = 1 Ԥ��

	struct data * next;
}Data;

BOOL DataInit(Data ** head, Data ** tail);// ��������һ�Σ����ڳ�ʼ��ָ��

BOOL DataStore(Data * head, int value,int i,int j);// ��value�������ݱ�(������ͷ��)��(i,j)��λ��

BOOL DataDestroy(Data * head);

BOOL DataCheck(Data * head);

Data * DataAddNewNode(Data * head, Data * tail);

Data * GetTail(Data * head); // ����βָ��

char * GetData(Data * head,int i,int j,char * buffer);


#endif
