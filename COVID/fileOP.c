#include <Windows.h>
#include <WinUser.h>
#include <MMSystem.h>
#include <stdio.h>
#include <string.h>

#include "data.h"
#include "main.h"

#define MAX_LENGTH 2000 // 每行最大长度

char filename[MAX_PATH];

BOOL openfile;
static char bmpfilepath[MAX_PATH];
static char buffer[MAX_LENGTH];// 本文件使用的缓冲区(1.读入CSV文件 2.从链表获取数据)
static FILE * fp;
static OPENFILENAME ofn;
static BOOL initialized;
static HWND hwnd;
static char * s;

BOOL SaveBitmapToFile(HBITMAP hBitmap,LPSTR lpFileName){ 
    HDC            hDC; //设备描述表
    int            iBits;//当前显示分辨率下每个像素所占字节数
    WORD           wBitCount;//位图中每个像素所占字节数    
    DWORD          dwPaletteSize=0;//定义调色板大小
    DWORD          dwBmBitsSize;//位图中像素字节大小
    DWORD          dwDIBSize;// 位图文件大小
    DWORD          dwWritten;//写入文件字节数
    BITMAP         Bitmap;//位图结构
    BITMAPFILEHEADER   bmfHdr;   //位图属性结构   
    BITMAPINFOHEADER   bi;       //位图文件头结构
    LPBITMAPINFOHEADER lpbi;     //位图信息头结构     指向位图信息头结构
    HANDLE          fh;//定义文件句柄
    HANDLE            hDib;//分配内存句柄
    HANDLE            hPal;//分配内存句柄
    HANDLE          hOldPal=NULL;//调色板句柄  
     
    //计算位图文件每个像素所占字节数   
    hDC = CreateDC("DISPLAY",NULL,NULL,NULL);   
    iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);   
    DeleteDC(hDC);
     
    if (iBits <= 1)       
        wBitCount = 1;   
    else if (iBits <= 4)       
        wBitCount = 4;   
    else if (iBits <= 8)       
        wBitCount = 8;   
    else if (iBits <= 24)
        wBitCount = 24;
    else if (iBits<=32)
        wBitCount = 24;
     
    //计算调色板大小   
    if (wBitCount <= 8)       
        dwPaletteSize = (1 << wBitCount) *sizeof(RGBQUAD);

    //设置位图信息头结构   
    GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);   
    bi.biSize            = sizeof(BITMAPINFOHEADER);   
    bi.biWidth           = Bitmap.bmWidth;   
    bi.biHeight          = Bitmap.bmHeight;   
    bi.biPlanes          = 1;   
    bi.biBitCount        = wBitCount;       
    bi.biCompression     = BI_RGB;   
    bi.biSizeImage       = 0;   
    bi.biXPelsPerMeter   = 0;   
    bi.biYPelsPerMeter   = 0;   
    bi.biClrUsed         = 0;   
    bi.biClrImportant    = 0;   
    dwBmBitsSize = ((Bitmap.bmWidth *wBitCount+31)/32)* 4*Bitmap.bmHeight ;
     
    //为位图内容分配内存   
    hDib  = GlobalAlloc(GHND,dwBmBitsSize+dwPaletteSize+sizeof(BITMAPINFOHEADER));   
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
    if (lpbi==NULL)
    {
        return FALSE;
    }
    *lpbi = bi;   
    // 处理调色板
    hPal = GetStockObject(DEFAULT_PALETTE);   
    if (hPal)
    {       
        hDC  = GetDC(NULL);       
        hOldPal = SelectPalette(hDC, (HPALETTE)hPal, FALSE);       
        RealizePalette(hDC);       
    }   
    // 获取该调色板下新的像素值   
    GetDIBits(hDC, hBitmap, 0, (UINT) Bitmap.bmHeight,       
        (LPSTR)lpbi + sizeof(BITMAPINFOHEADER)+dwPaletteSize,
        (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);   
    //恢复调色板      
    if (hOldPal)       
    {       
        SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);       
        RealizePalette(hDC);       
        ReleaseDC(NULL, hDC);       
    }   
    //创建位图文件       
    fh = CreateFile(lpFileName, GENERIC_WRITE,        
        0, NULL, CREATE_ALWAYS,       
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);   
 
    if (fh == INVALID_HANDLE_VALUE)       
        return FALSE;
     
    // 设置位图文件头   
    bmfHdr.bfType = 0x4D42;  // "BM"   
    dwDIBSize    = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)+ dwPaletteSize + dwBmBitsSize;     
    bmfHdr.bfSize = dwDIBSize;   
    bmfHdr.bfReserved1 = 0;   
    bmfHdr.bfReserved2 = 0;   
    bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER)+ dwPaletteSize;
     
    // 写入位图文件头   
    WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
     
    // 写入位图文件其余内容   
    WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
     
    //清除      
    GlobalUnlock(hDib);   
    GlobalFree(hDib);   
    CloseHandle(fh);
     
    return TRUE;
}

HBITMAP GetCaptureBmp(){
    HDC     hDC;   
    HDC     MemDC;   
    BYTE*   Data;   
    HBITMAP   hBmp;   
    BITMAPINFO   bi;
    memset(&bi,   0,   sizeof(bi));  
	RECT rect;
	GetClientRect(hwnd, &rect);
    bi.bmiHeader.biSize   =   sizeof(BITMAPINFO);
    bi.bmiHeader.biWidth   =  rect.right - rect.left ;
    bi.bmiHeader.biHeight   = rect.bottom - rect.top ; 
    bi.bmiHeader.biPlanes   =   1;   
    bi.bmiHeader.biBitCount   =   24;   
     
    hDC   =   GetDC(hwnd);   
    MemDC   =   CreateCompatibleDC(hDC);   
    hBmp   =   CreateDIBSection(MemDC,   &bi, DIB_RGB_COLORS,   (void**)&Data,   NULL,   0);   
    SelectObject(MemDC,   hBmp);   
    BitBlt(MemDC,   0,   0,   bi.bmiHeader.biWidth,   bi.bmiHeader.biHeight,hDC,   0 ,   0,   SRCCOPY);   
    ReleaseDC(NULL,   hDC);     
    DeleteDC(MemDC);   
    return   hBmp;  
}

BOOL ReadFromFile(Data * head, char (* table)[50]){
    SetCurrentTask(Concat("读取 ",filename));
    Data * tail = head; 
    DataDestroy(head);
    fp = fopen(filename,"r");
    if(fp == NULL){
        TaskFinished();
        return FALSE;
    }else{
        if(head == NULL){
		    MessageBox(NULL,"内存初始化有误!请重新打开程序！", NULL, MB_ICONERROR);
            TaskFinished();
            return FALSE;
        }else{
            char * slice;
            int i = 1, j;
            fgets(buffer,MAX_LENGTH,fp);
            for (j = 0, slice = strtok(buffer, ","); slice && *slice; j++,slice = strtok(NULL, ",\n")){
                strcpy(table[j],slice);
            } //字符串拆分操作
            (head->items)[1] = j;// 记录表头个数,即为数据表的列数

            while(fgets(buffer,MAX_LENGTH,fp)){
                if(tail = DataAddNewNode(head, tail)){
                    int flag = 0;
                    for (j = 1, slice = strtok(buffer, ","); slice && *slice; j++,slice = strtok(NULL, ",\n")){
                        if(flag == 0){
                            strcpy(tail->date,slice);
                            if(*slice == '#')
                                tail->items[0] = 1;
                            flag = 1;
                        }else{
                            tail ->items[j] = atoi(slice);
                        } 
                    }
                    i++;
                }else{
                    TaskFinished();
                    return FALSE;
                }
            }  
        }
    }
    openfile=TRUE;
    SetFileLen(ftell(fp));
    fclose(fp);
    TaskFinished();
    return TRUE;
}

void init(){
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = 0;
    ofn.lpstrDefExt = 0;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.nFilterIndex = 0;
    hwnd = FindWindowA(NULL ,"疫情分析");
    initialized = TRUE;
}

BOOL NewFile(Data * head){
    SetCurrentTask("新建文件");
    openfile=FALSE;
    init();
    DataDestroy(head);
    head -> items[1] = 1;
    TaskFinished();
    return TRUE;
}

BOOL GetFileName(){
    SetCurrentTask("获取文件路径");
    if(!initialized){
        init();
	}

    ofn.lpstrFile = filename;
    ofn.lpstrTitle = "选择你的英雄";
    ofn.lpstrFilter = "CSV File(*.csv)\0*.csv;\0\0";
    ofn.lpstrInitialDir = "./";
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    mciSendString("play bgm.mp3", NULL, 0, 0);
    if(GetOpenFileName(&ofn) == NULL){
        mciSendString("pause bgm.mp3",NULL,0,0);
		MessageBox(NULL,"请选择有效文件", NULL, MB_ICONERROR);
        TaskFinished();  
        return FALSE;
    }else{
        mciSendString("pause bgm.mp3",NULL,0,0);
        TaskFinished();
        return TRUE;
    }
}
BOOL Save(Data * head, char (* table)[50]){
    SetCurrentTask("保存");
    
    if(openfile)fp = fopen(filename,"w");
        if(!fp){
            MessageBox(NULL, "文件写入失败!请检查是否被其他程序占用!", NULL, MB_ICONERROR);
            TaskFinished();
            return FALSE;            
        }
        
        int rows = head->items[0];
        int columns = head->items[1];

        for(int i = 0;i < columns; i++){ 
            fprintf(fp,i!=columns-1 ?"%s,":"%s\n",table[i]);
        }
        
        for(int i = 1;i <= rows; i++){
            for(int j = 1;j <= columns;j++){
                if(s = GetData(head,i,j,buffer)){
                    fprintf(fp,j!=columns?"%s,":"%s\n",s);
                }else{
            		fclose(fp);
                    TaskFinished();
                    return FALSE;
                }
            }
        }
        fclose(fp);
        TaskFinished();
        return TRUE;
 
}

BOOL SaveAs(Data * head, char (* table)[50]){
    SetCurrentTask("另存为");
    if(!initialized){
		MessageBox(NULL, "请先打开或新建一个文件", NULL, MB_ICONERROR);
        TaskFinished();
        return FALSE;
    } 
    ofn.lpstrFile = filename;
	ofn.lpstrTitle = "保存到";// 使用系统默认标题留空即可 
	ofn.lpstrDefExt = "csv";// 默认追加的扩展名 
    ofn.lpstrFilter = "CSV File(*.csv)\0*.csv;\0\0";
    ofn.Flags =  OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;//目录必须存在，覆盖文件前发出警告

	if (GetSaveFileName(&ofn)){
        fp = fopen(filename,"w");
        if(!fp){
            MessageBox(NULL, "文件写入失败!请检查是否被其他程序占用!", NULL, MB_ICONERROR);
            TaskFinished();
            return FALSE;            
        }
        
        int rows = head->items[0];
        int columns = head->items[1];

        for(int i = 0;i < columns; i++){ 
            fprintf(fp,i!=columns-1 ?"%s,":"%s\n",table[i]);
        }
        
        for(int i = 1;i <= rows; i++){
            for(int j = 1;j <= columns;j++){
                if(s = GetData(head,i,j,buffer)){
                    fprintf(fp,j!=columns?"%s,":"%s\n",s);
                }else{
            		fclose(fp);
                    TaskFinished();
                    return FALSE;
                }
            }
        }
        fclose(fp);
        TaskFinished();
        return TRUE;
	}else{
		MessageBox(NULL,"请选择有效目录", NULL, MB_ICONERROR);
        TaskFinished();
        return FALSE;
	}
}

BOOL OpenDoc(){
    SetCurrentTask("打开帮助文档");
    if(ShellExecute(0, "open", "help.pdf", NULL ,NULL, SW_SHOWNORMAL) < 32){
    // 返回值小于32时，执行失败
		MessageBox(NULL,"检查一下\nhelp.pdf是否与可执行文件位于同一目录下？\n您的文件路径中是否含有空格？\n求求了！\n还不行的话,在接下来会打开的网站中根据ShellExecute的返回值看下是什么问题趴", "是不是哪里出了问题？", MB_ICONERROR);
        ShellExecute(0, "open", "https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecutea", NULL,NULL, SW_SHOWNORMAL);
        TaskFinished();
        return FALSE;
    }else{
        TaskFinished();
        return TRUE;
    }
}

BOOL SaveBMP(){
    SetCurrentTask("另存窗口为图像");
    if(!initialized){
		MessageBox(NULL, "请先打开或新建一个文件", NULL, MB_ICONERROR);
        TaskFinished();
        return FALSE;
    }
    
    HBITMAP bmp = GetCaptureBmp();

    if(!bmp){
        MessageBox(NULL, "读取窗口失败!", NULL, MB_ICONERROR);
        TaskFinished();
        return FALSE;
    }

    ofn.lpstrFile = bmpfilepath;
    ofn.Flags =  OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;//目录必须存在，覆盖文件前发出警告
	ofn.lpstrTitle = "保存到";// 使用系统默认标题留空即
    ofn.lpstrFilter = "BMP File(*.bmp)\0*.bmp;\0\0";
	ofn.lpstrDefExt = "bmp";//默认追加的扩展名

    if(GetSaveFileName(&ofn)){
        if(SaveBitmapToFile(bmp,bmpfilepath)){
            TaskFinished();
            return TRUE;
        }else{
            MessageBox(NULL, "图像保存失败!请检查是否被其他程序占用!", NULL, MB_ICONERROR);
            return FALSE;
        }
    }else{
        TaskFinished();
		MessageBox(NULL, "请先选择有效目录", NULL, MB_ICONERROR);
        return FALSE;
    }
}