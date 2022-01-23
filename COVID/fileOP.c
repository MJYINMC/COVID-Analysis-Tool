#include <Windows.h>
#include <WinUser.h>
#include <MMSystem.h>
#include <stdio.h>
#include <string.h>

#include "data.h"
#include "main.h"

#define MAX_LENGTH 2000 // ÿ����󳤶�

char filename[MAX_PATH];

BOOL openfile;
static char bmpfilepath[MAX_PATH];
static char buffer[MAX_LENGTH];// ���ļ�ʹ�õĻ�����(1.����CSV�ļ� 2.�������ȡ����)
static FILE * fp;
static OPENFILENAME ofn;
static BOOL initialized;
static HWND hwnd;
static char * s;

BOOL SaveBitmapToFile(HBITMAP hBitmap,LPSTR lpFileName){ 
    HDC            hDC; //�豸������
    int            iBits;//��ǰ��ʾ�ֱ�����ÿ��������ռ�ֽ���
    WORD           wBitCount;//λͼ��ÿ��������ռ�ֽ���    
    DWORD          dwPaletteSize=0;//�����ɫ���С
    DWORD          dwBmBitsSize;//λͼ�������ֽڴ�С
    DWORD          dwDIBSize;// λͼ�ļ���С
    DWORD          dwWritten;//д���ļ��ֽ���
    BITMAP         Bitmap;//λͼ�ṹ
    BITMAPFILEHEADER   bmfHdr;   //λͼ���Խṹ   
    BITMAPINFOHEADER   bi;       //λͼ�ļ�ͷ�ṹ
    LPBITMAPINFOHEADER lpbi;     //λͼ��Ϣͷ�ṹ     ָ��λͼ��Ϣͷ�ṹ
    HANDLE          fh;//�����ļ����
    HANDLE            hDib;//�����ڴ���
    HANDLE            hPal;//�����ڴ���
    HANDLE          hOldPal=NULL;//��ɫ����  
     
    //����λͼ�ļ�ÿ��������ռ�ֽ���   
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
     
    //�����ɫ���С   
    if (wBitCount <= 8)       
        dwPaletteSize = (1 << wBitCount) *sizeof(RGBQUAD);

    //����λͼ��Ϣͷ�ṹ   
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
     
    //Ϊλͼ���ݷ����ڴ�   
    hDib  = GlobalAlloc(GHND,dwBmBitsSize+dwPaletteSize+sizeof(BITMAPINFOHEADER));   
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
    if (lpbi==NULL)
    {
        return FALSE;
    }
    *lpbi = bi;   
    // �����ɫ��
    hPal = GetStockObject(DEFAULT_PALETTE);   
    if (hPal)
    {       
        hDC  = GetDC(NULL);       
        hOldPal = SelectPalette(hDC, (HPALETTE)hPal, FALSE);       
        RealizePalette(hDC);       
    }   
    // ��ȡ�õ�ɫ�����µ�����ֵ   
    GetDIBits(hDC, hBitmap, 0, (UINT) Bitmap.bmHeight,       
        (LPSTR)lpbi + sizeof(BITMAPINFOHEADER)+dwPaletteSize,
        (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);   
    //�ָ���ɫ��      
    if (hOldPal)       
    {       
        SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);       
        RealizePalette(hDC);       
        ReleaseDC(NULL, hDC);       
    }   
    //����λͼ�ļ�       
    fh = CreateFile(lpFileName, GENERIC_WRITE,        
        0, NULL, CREATE_ALWAYS,       
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);   
 
    if (fh == INVALID_HANDLE_VALUE)       
        return FALSE;
     
    // ����λͼ�ļ�ͷ   
    bmfHdr.bfType = 0x4D42;  // "BM"   
    dwDIBSize    = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)+ dwPaletteSize + dwBmBitsSize;     
    bmfHdr.bfSize = dwDIBSize;   
    bmfHdr.bfReserved1 = 0;   
    bmfHdr.bfReserved2 = 0;   
    bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER)+ dwPaletteSize;
     
    // д��λͼ�ļ�ͷ   
    WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
     
    // д��λͼ�ļ���������   
    WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
     
    //���      
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
    SetCurrentTask(Concat("��ȡ ",filename));
    Data * tail = head; 
    DataDestroy(head);
    fp = fopen(filename,"r");
    if(fp == NULL){
        TaskFinished();
        return FALSE;
    }else{
        if(head == NULL){
		    MessageBox(NULL,"�ڴ��ʼ������!�����´򿪳���", NULL, MB_ICONERROR);
            TaskFinished();
            return FALSE;
        }else{
            char * slice;
            int i = 1, j;
            fgets(buffer,MAX_LENGTH,fp);
            for (j = 0, slice = strtok(buffer, ","); slice && *slice; j++,slice = strtok(NULL, ",\n")){
                strcpy(table[j],slice);
            } //�ַ�����ֲ���
            (head->items)[1] = j;// ��¼��ͷ����,��Ϊ���ݱ������

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
    hwnd = FindWindowA(NULL ,"�������");
    initialized = TRUE;
}

BOOL NewFile(Data * head){
    SetCurrentTask("�½��ļ�");
    openfile=FALSE;
    init();
    DataDestroy(head);
    head -> items[1] = 1;
    TaskFinished();
    return TRUE;
}

BOOL GetFileName(){
    SetCurrentTask("��ȡ�ļ�·��");
    if(!initialized){
        init();
	}

    ofn.lpstrFile = filename;
    ofn.lpstrTitle = "ѡ�����Ӣ��";
    ofn.lpstrFilter = "CSV File(*.csv)\0*.csv;\0\0";
    ofn.lpstrInitialDir = "./";
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    mciSendString("play bgm.mp3", NULL, 0, 0);
    if(GetOpenFileName(&ofn) == NULL){
        mciSendString("pause bgm.mp3",NULL,0,0);
		MessageBox(NULL,"��ѡ����Ч�ļ�", NULL, MB_ICONERROR);
        TaskFinished();  
        return FALSE;
    }else{
        mciSendString("pause bgm.mp3",NULL,0,0);
        TaskFinished();
        return TRUE;
    }
}
BOOL Save(Data * head, char (* table)[50]){
    SetCurrentTask("����");
    
    if(openfile)fp = fopen(filename,"w");
        if(!fp){
            MessageBox(NULL, "�ļ�д��ʧ��!�����Ƿ���������ռ��!", NULL, MB_ICONERROR);
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
    SetCurrentTask("���Ϊ");
    if(!initialized){
		MessageBox(NULL, "���ȴ򿪻��½�һ���ļ�", NULL, MB_ICONERROR);
        TaskFinished();
        return FALSE;
    } 
    ofn.lpstrFile = filename;
	ofn.lpstrTitle = "���浽";// ʹ��ϵͳĬ�ϱ������ռ��� 
	ofn.lpstrDefExt = "csv";// Ĭ��׷�ӵ���չ�� 
    ofn.lpstrFilter = "CSV File(*.csv)\0*.csv;\0\0";
    ofn.Flags =  OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;//Ŀ¼������ڣ������ļ�ǰ��������

	if (GetSaveFileName(&ofn)){
        fp = fopen(filename,"w");
        if(!fp){
            MessageBox(NULL, "�ļ�д��ʧ��!�����Ƿ���������ռ��!", NULL, MB_ICONERROR);
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
		MessageBox(NULL,"��ѡ����ЧĿ¼", NULL, MB_ICONERROR);
        TaskFinished();
        return FALSE;
	}
}

BOOL OpenDoc(){
    SetCurrentTask("�򿪰����ĵ�");
    if(ShellExecute(0, "open", "help.pdf", NULL ,NULL, SW_SHOWNORMAL) < 32){
    // ����ֵС��32ʱ��ִ��ʧ��
		MessageBox(NULL,"���һ��\nhelp.pdf�Ƿ����ִ���ļ�λ��ͬһĿ¼�£�\n�����ļ�·�����Ƿ��пո�\n�����ˣ�\n�����еĻ�,�ڽ�������򿪵���վ�и���ShellExecute�ķ���ֵ������ʲô����ſ", "�ǲ�������������⣿", MB_ICONERROR);
        ShellExecute(0, "open", "https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecutea", NULL,NULL, SW_SHOWNORMAL);
        TaskFinished();
        return FALSE;
    }else{
        TaskFinished();
        return TRUE;
    }
}

BOOL SaveBMP(){
    SetCurrentTask("��洰��Ϊͼ��");
    if(!initialized){
		MessageBox(NULL, "���ȴ򿪻��½�һ���ļ�", NULL, MB_ICONERROR);
        TaskFinished();
        return FALSE;
    }
    
    HBITMAP bmp = GetCaptureBmp();

    if(!bmp){
        MessageBox(NULL, "��ȡ����ʧ��!", NULL, MB_ICONERROR);
        TaskFinished();
        return FALSE;
    }

    ofn.lpstrFile = bmpfilepath;
    ofn.Flags =  OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;//Ŀ¼������ڣ������ļ�ǰ��������
	ofn.lpstrTitle = "���浽";// ʹ��ϵͳĬ�ϱ������ռ�
    ofn.lpstrFilter = "BMP File(*.bmp)\0*.bmp;\0\0";
	ofn.lpstrDefExt = "bmp";//Ĭ��׷�ӵ���չ��

    if(GetSaveFileName(&ofn)){
        if(SaveBitmapToFile(bmp,bmpfilepath)){
            TaskFinished();
            return TRUE;
        }else{
            MessageBox(NULL, "ͼ�񱣴�ʧ��!�����Ƿ���������ռ��!", NULL, MB_ICONERROR);
            return FALSE;
        }
    }else{
        TaskFinished();
		MessageBox(NULL, "����ѡ����ЧĿ¼", NULL, MB_ICONERROR);
        return FALSE;
    }
}