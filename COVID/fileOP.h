#ifndef _fileOP_H_
#define _fileOP_H_

extern char  filename[260];
extern BOOL openfile;

BOOL NewFile(Data * head);
BOOL GetFileName(); // 
BOOL ReadFromFile(Data * head, char (* table)[50]); // 从文件读取信息组织进链表中
BOOL Save(Data * head, char (* table)[50]); // 存入文件
BOOL SaveAs(Data * head, char (* table)[50]); // 另存为
BOOL OpenDoc(); // 打开帮助文档
BOOL SaveBMP(); // 保存为bmp图片

#endif
