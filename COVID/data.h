#ifndef _Data_H_
#define _Data_H_

#define MAX_ITEMS 50
#define MAX_DAYS 100
typedef struct data {
	char date [15];// 以字符串形式存储日期

	int items[MAX_ITEMS];// 数据 items[0]表示此数据为预测还是真实的 
	// items[0] = 0 真实
	// items[0] = 1 预测

	struct data * next;
}Data;

BOOL DataInit(Data ** head, Data ** tail);// 仅被调用一次，用于初始化指针

BOOL DataStore(Data * head, int value,int i,int j);// 将value放入数据表(除开表头行)中(i,j)的位置

BOOL DataDestroy(Data * head);

BOOL DataCheck(Data * head);

Data * DataAddNewNode(Data * head, Data * tail);

Data * GetTail(Data * head); // 返回尾指针

char * GetData(Data * head,int i,int j,char * buffer);


#endif
