#include <Windows.h>
#include <WinUser.h>
#include <MMSystem.h>
#include <stdlib.h>
#include <stdio.h>
#include "../libgraphics/graphics.h"
#include "../libgraphics/extgraph.h"
#include "../simpleGUI/imgui.h"

#include "main.h"
#include "data.h"
#include "fileOP.h"
#include "predict.h"
#include "chart.h" 

#define MAX_LENGTH 12
#define LISTEN_FILE_CHANGE 1 // ��ʱ�����
/*
���������ȣ����ļ���buffer�������� �������ȡ���ݽ��л���
����int �������2147483647 
��С����Ϊ12 ��Ϊ���������Ϊ 2 1 4 7 4 8 3 6 4 7  user_input_char  /0
��ʱ������ȷ����������Ϣ  
*/

// ��Ҫ�ṩ�������ļ�ʹ�ã�ͷ�ļ�����Ҫ����Ӧ����extern��ȥ
static HWND hwnd;
static HDC osdc;
static HICON icon,bgm,open,save,saveas,help,close;
// ���ļ���ʹ�õ�ȫ�ֱ���

static int Mode; 
static double xdpi;
static double ydpi;
static BOOL Capturing;
static BOOL predicted;
static BOOL BGM_PLAYING = 1;
static Data * head;
static Data * tail;
static char table[MAX_ITEMS][50];
static char buffer[MAX_LENGTH];
static int interval[4] = {1,1,1,1};
static char * task;
static BOOL task_status;
static BOOL processing;
static int timer_interval = 100;    // ���ʱ�� (in ms)
static double winwidth, winheight;   // ���ڳߴ�
static double fH ;//����߶�
static int subplots;
static int filelen;

double Mx,My;

// ��������غ���
void SetCurrentTask(char * taskname){
    task = taskname;
    processing = TRUE;
}
void TaskFinished(){
    processing = FALSE;
};

void SetFileLen(int len){
    filelen = len;
}

void ClearTable(){
    ZeroMemory(table,50*MAX_ITEMS);
}
void CharEventProcess(char ch)
{
	uiGetChar(ch); // GUI�ַ�����
	display(); //ˢ����ʾ
}

// �û��ļ����¼���Ӧ����
void KeyboardEventProcess(int key, int event)
{
	uiGetKeyboard(key,event); // GUI��ȡ����
	display(); // ˢ����ʾ
}

// �û�������¼���Ӧ����
void MouseEventProcess(int x, int y, int button, int event)
{
	uiGetMouse(x,y,button,event); //GUI��ȡ���
    static int zone;
 	Mx = ScaleXInches(x);/*pixels --> inches*/
 	My = ScaleYInches(y);/*pixels --> inches*/

    switch (subplots)
    {
    case 1:
        zone = -1;
        if(My <= (winheight-1)/5 + 3.5*fH && My >= (winheight-1)/5- fH)
            zone = 0;
        break;
    case 2:
        zone = -1;
        if(Mx <= winwidth/2 && My <= (winheight-1)/5+ 3.5*fH && My >= (winheight-1)/5- 2*fH)
            zone = 0;
        if(Mx >= winwidth/2 && My <= (winheight-1)/5+ 3.5*fH && My >= (winheight-1)/5- 2*fH)
            zone = 1;
        break;
    case 3:
        zone = -1;
        if(Mx <= winwidth/2 && My <= 3*(winheight-1)/5+ 3.5*fH && My >= 3*(winheight-1)/5- 2*fH)
            zone = 0;
        if(Mx >= winwidth/2 && My <= 3*(winheight-1)/5+ 3.5*fH && My >= 3*(winheight-1)/5- 2*fH)
            zone = 1;
        if(My <= (winheight-1)/10 + 3.5*fH && My >= (winheight-1)/10- 2*fH)
            zone = 2;
        break;
    case 4:
        zone = -1;
        if(Mx <= winwidth/2 && My <= 3*(winheight-1)/5+ 3.5*fH && My >= 3*(winheight-1)/5- 2*fH)
            zone = 0;
        if(Mx >= winwidth/2 && My <= 3*(winheight-1)/5+ 3.5*fH && My >= 3*(winheight-1)/5- 2*fH)
            zone = 1;
        if(Mx <= winwidth/2 && My <= (winheight-1)/10 + 3.5*fH && My >= (winheight-1)/10- 2*fH)
            zone = 2;
        if(Mx >= winwidth/2 && My <= (winheight-1)/10 + 3.5*fH && My >= (winheight-1)/10- 2*fH)
            zone = 3;
        break;
    default:
        zone = -1;
        break;
    }
 	if(event == ROLL_UP && zone != -1){
        interval[zone] = interval[zone] + 1 <=5 ? interval[zone]+1 : 5;
    }else if(event == ROLL_DOWN && zone != -1){
        interval[zone] = interval[zone] - 1 >= 1 ? interval[zone]-1 :1;
    }
 	
	display(); // ˢ����ʾ
}
// �û��Ķ�ʱ���¼���Ӧ����
void TimerEventProcess(int timerID)
{
    static FILE * tmp;
    if(*filename){
        tmp = fopen(filename,"r");
        if(tmp){
            fseek(tmp,0,SEEK_END);
            if(ftell(tmp) != filelen){
                ReadFromFile(head, table);
                tail = GetTail(head);
                fclose(tmp);
            }
        }
    }
    display();
}

void Main(){ 
    SetWindowTitle("�������");
    SetWindowSize(16,9);
    InitGraphics();

    // InitConsole(); //��ʼ������̨�����ڵ���
    SetFont("Times");// ��������
    SetPointSize(18);// �����ֺ�
    mciSendString("play Lifeline.mp3 repeat",NULL,0,0);
    // ������ʼ��
    hwnd = FindWindowA(NULL ,"�������");
    osdc = GetOsdc();
    HINSTANCE instance = GetModuleHandle(NULL);
    icon = LoadImage(instance, "A", IMAGE_ICON, 0, 0,LR_DEFAULTCOLOR);
    bgm = LoadImage(instance, "bgm", IMAGE_ICON, 0, 0,LR_DEFAULTCOLOR);
    open = LoadImage(instance, "open", IMAGE_ICON, 0, 0,LR_DEFAULTCOLOR);
    save = LoadImage(instance, "save", IMAGE_ICON, 0, 0,LR_DEFAULTCOLOR);
    saveas = LoadImage(instance, "saveas", IMAGE_ICON, 0, 0,LR_DEFAULTCOLOR);
    help = LoadImage(instance, "help", IMAGE_ICON, 0, 0,LR_DEFAULTCOLOR);
    close = LoadImage(instance, "close", IMAGE_ICON, 0, 0,LR_DEFAULTCOLOR);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
    fH = GetFontHeight();
    winwidth = GetWindowWidth();
    winheight = GetWindowHeight();
    xdpi = GetXResolution();
	ydpi = GetYResolution();
    DataInit(&head, &tail);
    Mode = -1;
    registerCharEvent(CharEventProcess);        // �ַ�
    registerKeyboardEvent(KeyboardEventProcess);// ����
    registerMouseEvent(MouseEventProcess);      // ���
	registerTimerEvent(TimerEventProcess);      // ��ʱ��
    startTimer(LISTEN_FILE_CHANGE, timer_interval);// ��ʱ������

}

void DrawMenu(){
	static char * menuListFile[] = {"�ļ�",  
		"�� | Ctrl-O", // ��ݼ��������[Ctrl-X]��ʽ�������ַ����Ľ�β
		"�½� | Ctrl-N", // TODO:�½��ļ���ֱ����ת���༭�ձ��ҳ��
        "���� | Ctrl-S",
        "���Ϊ | Ctrl-A",
        "���Ϊͼ�� | Ctrl-P",
		"�˳� | Ctrl-E"};
	static char * menuListEdit[] = {"�༭",  
		"�� �����±�",
        "�� ����༭ģʽ", 
        "�� �������",
        "�� �������",
        "�� �˳��༭ģʽ"};
	static char * menuListDraw[] = {"��ͼ",  
		"�� �����ͼģʽ", 
        "�� �����ͼ(subplot)",
        "�� ɾ����ͼ",
        "�� �˳���ͼģʽ"};
    static char * menuListPredict[] = {"Ԥ��",  
		"�� ����Ԥ��ģʽ", 
        "�� �˳�Ԥ��ģʽ",
        };
	static char * menuListHelp[] = {"����",  
		"����ָ�� | Ctrl-H",
		"���ڱ����",
		};
    int selection;
	// File �˵�
	double x = 0; //fH/8;
	double y = winheight;
	double h = fH*1.5; // �ؼ��߶�
	double w = TextStringWidth(menuListFile[0])*2; // �ؼ����
    double wlist  =TextStringWidth(menuListFile[1])*1.8;

    drawMenuBar(x,y-h,winwidth,h);
	selection = menuList(GenUIID(0), x, y-h, w, wlist, h, menuListFile, sizeof(menuListFile)/sizeof(menuListFile[0]));
	if( selection == 1 ) {
        mciSendString("pause Lifeline.mp3",NULL,0,0);
        task_status = GetFileName();
        if(BGM_PLAYING) mciSendString("play Lifeline.mp3 repeat",NULL,0,0);
        if(task_status){
        	ClearTable();
            task_status = ReadFromFile(head,table);
    	}
        tail  = GetTail(head);
        predicted = tail ->items[0];
        ZeroMemory(ifdraw,sizeof(ifdraw));
        subplots = 0;
        Mode = 3;
    }else if(selection == 2){
        predicted = FALSE;
        ClearTable();
        task_status = NewFile(head);
        ZeroMemory(ifdraw,sizeof(ifdraw));
        subplots = 0;
        tail = GetTail(head);
    }else if(selection == 3){
        if(!*filename){
            task_status == SaveAs(head,table);
        }else{
            task_status == Save(head,table);
        }
    }else if(selection == 4){
        task_status = SaveAs(head,table);
    }else if(selection == 5){
        Capturing = TRUE; // ����Ϊ���ڽ���״̬ 
        display();// ˢ����ʾ ��ʱ�������״̬���������� 
        UpdateDisplay();// �����Ƶ�ͼ��ˢ�µ�hdc 
        task_status = SaveBMP();// ����ͼƬ 
        Capturing = FALSE;
    }else if( selection == 6){
		exit(-1); // �˳������򣬷���ֵΪ-1 
    }

 	selection = menuList(GenUIID(0), x+w, y-h, w, wlist, h, menuListEdit, sizeof(menuListEdit)/sizeof(menuListEdit[0]));
    if(selection == 1){
		SetCurrentTask("�����±�");
        predicted = FALSE;
        ClearTable();
        DataDestroy(head);
        tail = GetTail(head);
        head -> items[1] = 1;
        Mode = 0;
        TaskFinished();
        ZeroMemory(ifdraw,sizeof(ifdraw));
        subplots = 0;
        task_status = 1;
    }else if(selection == 2){
        if(Mode != 0){
            SetCurrentTask("�л����༭ģʽ");
            Mode =  0;
            TaskFinished();
            task_status = 1;
        }else{
            MessageBox(NULL,"�Ѵ��ڱ༭ģʽ��","��ʾ",MB_OK);
        }
    }else if(selection == 3 ){
        if(Mode == 0 ){
            SetCurrentTask("�������");
            tail = DataAddNewNode(head,tail);
            TaskFinished();
            task_status = 1;
        }else{
            MessageBox(NULL,"δ���ڱ༭ģʽ��","����",MB_ICONERROR);
        }
    }else if(selection == 4 ){
        if(Mode == 0){
            SetCurrentTask("�������");
            head -> items[1]++;
            TaskFinished();
            task_status = 1;
        }else{
            MessageBox(NULL,"δ���ڱ༭ģʽ��","����",MB_ICONERROR);
        }
    }else if(selection == 5){
        SetCurrentTask("�˳��༭ģʽ");
        Mode = 3;
        TaskFinished();
        task_status = 1;  
    }
    
 	selection = menuList(GenUIID(0), x+2*w, y-h, w, wlist, h, menuListDraw, sizeof(menuListDraw)/sizeof(menuListDraw[0])); // ��ͼ�˵�
    if(selection == 1){
        SetCurrentTask("�����ͼģʽ");
        Mode = 1;
        TaskFinished();
        task_status = 1;       
    }else if(selection == 2){
        SetCurrentTask("�����ͼ");
       if(subplots != 0 && ifdraw[subplots-1][0] == 0){
            MessageBox(NULL,"���������һ��ͼ������","����",MB_ICONERROR);
            TaskFinished();
            task_status = 0;        
        }else if(subplots < 4 && Mode == 1){
            subplots++;
            TaskFinished();
            task_status = 1; 
        }else {
            MessageBox(NULL,"δ���ڻ�ͼģʽ������ͼ�������ࣨ���֧�ֻ���������ͼ��","����",MB_ICONERROR);
            TaskFinished();
            task_status = 0;    
        }
       
    }else if(selection == 3){
        SetCurrentTask("ɾ����ͼ");
        if(Mode == 1){
            subplots = (subplots-1 >= 0) ? (subplots-1):0;
            ifdraw[subplots][0] = 0;
            TaskFinished();
            task_status = 1;   
        }else{
            MessageBox(NULL,"δ���ڻ�ͼģʽ","����",MB_ICONERROR);
            TaskFinished();
            task_status = 0;      
        }
    }else if(selection == 4){
        SetCurrentTask("�˳���ͼģʽ");
        Mode = 3;
        TaskFinished();
        task_status = 1; 
    }
    
 	selection = menuList(GenUIID(0), x+3*w, y-h, w, wlist, h, menuListPredict, sizeof(menuListPredict)/sizeof(menuListPredict[0]));
    if(selection == 1){
        SetCurrentTask("����Ԥ��ģʽ");
        Mode = 2;
        TaskFinished();
        task_status = 1;       
    }else if(selection == 2){
        SetCurrentTask("�˳�Ԥ��ģʽ");
        Mode = 3;
        TaskFinished();
        task_status = 1;        
    }
    selection = menuList(GenUIID(0), x+4*w, y-h, w, wlist, h, menuListHelp, sizeof(menuListHelp)/sizeof(menuListHelp[0]));
    if(selection == 1){
        task_status = OpenDoc();
    }else if(selection == 2){
    	MessageBox(NULL,"2020 ZJU C��̴���ҵ","All Rights Reversed.",MB_OK);
	}
}

void DrawStatusBar(){
    drawMenuBar(0,0,winwidth,fH*1.1);
    if(task){
        drawLabel(0,0.05,task);
        if(!processing){
            SetPenColor("Red");
            drawLabel(TextStringWidth(task),0.05,task_status?" �ɹ�":" ʧ��");
        }else{
            SetPenColor("Red");
            drawLabel(TextStringWidth(task),0.05," ���ڴ�����");
        }
    }else{
        drawLabel(0,0.05,"�ף��㻹δ�����κβ�����");
    }

    if(*filename && openfile){
        static char * fileinformation;
        fileinformation = Concat("��ǰ�ļ���",filename);
        SetPenColor("Blue");
        drawLabel(winwidth - TextStringWidth(fileinformation),0.05,fileinformation);
    }
};

BOOL DrawSheet(){
    static char * s;
    if(head -> items[1]){
        int rows = head -> items[0];
        int columns = head ->items[1];
        double dx = (winwidth - 1) / columns;
        double dy = (winheight - 1) / (rows + 1);
        for(int i = 0;i < columns;i++){
            textbox(GenUIID(i),0.5 + i*dx,winheight - dy - 2*fH,dx-0.02,dy-0.02,table[i],50);
        }
        for(int i = 1; i <= rows;i++){
            for(int j = 1; j <= columns;j++){
                if(s = GetData(head,i,j,buffer)){
                    if(textbox(GenUIID(i*columns+j),0.5 + (j-1)*dx,winheight - (i+1)*dy - 2*fH,dx-0.02,dy-0.02,s,sizeof(buffer))){
                        if(*s == '#'){
                            predicted = TRUE;
                        }
                        if(j >= 1 && atoi(buffer) >= 0){
                            DataStore(head,atoi(buffer),i,j);
                        }else{
                            MessageBox(NULL,"�������ͷ�Χ","����",MB_ICONWARNING);
                        }
                    };
                }else{
                    return FALSE; 
                }
            }
        }
        return TRUE;
    }else{
        SetPenColor("Red");
        SetPointSize(70);// �����ֺ�
        drawLabel(winwidth/2 - TextStringWidth("���½�����ļ�")/2,winheight/2,"���½�����ļ�");
        SetPointSize(18);// �����ֺ�
        return FALSE;
    }
}

void DrawToolBar(){
    static double width;
    width = TextStringWidth("       ");
    DrawIconEx(osdc,(winwidth - 6*width)*xdpi+2,1,bgm,30,30,0,0,DI_NORMAL);
    DrawIconEx(osdc,(winwidth - 5*width)*xdpi+2,0,open,30,30,0,0,DI_NORMAL);
    DrawIconEx(osdc,(winwidth - 4*width)*xdpi+2,1,save,30,30,0,0,DI_NORMAL);
    DrawIconEx(osdc,(winwidth - 3*width)*xdpi+2,3,saveas,29,29,0,0,DI_NORMAL);
    DrawIconEx(osdc,(winwidth - 2*width)*xdpi+2,1,help,30,30,0,0,DI_NORMAL);
    DrawIconEx(osdc,(winwidth - 1*width)*xdpi+2,1,close,30,30,0,0,DI_NORMAL);

    if(button(GenUIID(0),winwidth - 6*width, winheight- 1.5*fH,width, 1.5*fH,"")){
        if(BGM_PLAYING){
            mciSendString("pause Lifeline.mp3",NULL,0,0);
            BGM_PLAYING = 0;
        }else{
            mciSendString("play Lifeline.mp3 repeat",NULL,0,0);
            BGM_PLAYING = 1;
        }
    }

    if(button(GenUIID(0),winwidth - 5*width, winheight- 1.5*fH,width, 1.5*fH,"")){
        predicted = FALSE;
        mciSendString("pause Lifeline.mp3",NULL,0,0);
        task_status = GetFileName();
        if(BGM_PLAYING) mciSendString("play Lifeline.mp3 repeat",NULL,0,0);
        if(task_status){
        	ClearTable();
            task_status = ReadFromFile(head,table);
    	}
        tail  = GetTail(head);
        subplots = 0;
        Mode = 3;
    }
    if(button(GenUIID(0),winwidth - 4*width, winheight- 1.5*fH,width, 1.5*fH,"")){
        if(!*filename){
            task_status = SaveAs(head,table);
        }else{
            task_status = Save(head,table);
        }
    }
    if(button(GenUIID(0),winwidth - 3*width, winheight- 1.5*fH,width, 1.5*fH,"")){
        task_status = SaveAs(head,table);

    }    
    if(button(GenUIID(0),winwidth - 2*width, winheight- 1.5*fH,width, 1.5*fH,"")){
        task_status = OpenDoc();
    }
    if(button(GenUIID(0),winwidth - 1*width, winheight- 1.5*fH,width, 1.5*fH,"")){
        exit(-1);
    }
}

BOOL DrawUnchangeSheet(double width){
    static char * s;
    if(head -> items[1]){
        int rows = head -> items[0];
        int columns = head ->items[1];
        double dx = (width - 1) / columns;
        double dy = (winheight - 1) / (rows + 1);
        for(int i = 0;i < columns;i++){
        	SetPenColor("Black");
            drawBox(0.5 + i*dx,winheight - dy - 2*fH,dx,dy,0,table[i],0,"Black");
        }
        for(int i = 1; i <= rows;i++){
            for(int j = 1; j <= columns;j++){
                
                if(s = GetData(head,i,j,buffer)){
                    drawBox(0.5 + (j-1)*dx,winheight - (i+1)*dy - 2*fH,dx,dy,0,s,0,"Black");    
                }else{
                    return FALSE; 
                }
                
            }
        }
        return TRUE;
    }else{
        SetPenColor("Red");
        SetPointSize(70);// �����ֺ�
        drawLabel(width/3,winheight/2,"���½�����ļ�");
        SetPointSize(18);// �����ֺ�
        return FALSE;
    }
}

BOOL DrawGraph(){
    static char * type [3] = {"����ͼ","����ͼ","��״ͼ"};
    if(!subplots){
            SetPenColor("Red");
            SetPointSize(70);// �����ֺ�
            drawLabel(winwidth/2-TextStringWidth("�������ͼ")/2,winheight/2,"�������ͼ");
            SetPointSize(18);// �����ֺ�  
            return FALSE;      
    }

    if(ifdraw[subplots-1][0] == 0){
        SetPenColor("Red");
        static int choice = 0;
        SetPointSize(50);
        drawLabel(winwidth/2-TextStringWidth("�������õ�N��ͼ")/2,winheight-4*fH,Concat(Concat("�������õ�",itoa(subplots,buffer,10)),"��ͼ"));
        SetPenColor("Black");
        SetPointSize(18);
        drawLabel(0.5,winheight-8*fH,"ѡ��ͼ������:");
        if(button(GenUIID(0),0.5+TextStringWidth("ѡ��ͼ������:  "),winheight-8.5*fH,TextStringWidth("����ͼ"),fH*2,type[choice%3])){
            choice++;
        }
        SetPenColor("Black");
        if(choice%3 != 2){
            drawLabel(0.5,winheight-12*fH,"ѡ��Ҫ���Ƶ���:");
            double dx = (winwidth - 1) / (head->items[1]-1);
            for(int i = 0; i < head->items[1]-1 ; i++){
                SetPenColor("Black");
                drawBox(0.5 + i*dx,winheight-20*fH,dx,4*fH,0,table[i+1],0,"Black");
                if(button(GenUIID(i),0.5 + i*dx, winheight - 24 *fH,dx,4*fH,ifdraw[subplots-1][i+1]?"����":"������")){
                    ifdraw[subplots-1][i+1] = ifdraw[subplots-1][i+1] ? 0:1;
                }
            }
        }
        if(button(GenUIID(0),winwidth/2,1,2*TextStringWidth("ȷ��"),2*fH,"ȷ��")) ifdraw[subplots-1][0] = choice%3+1;
        return FALSE;
    }
    switch(subplots){
        case 1: DrawChart(-1,1,winwidth,winheight-2,head,table,0,interval[0]);break;
        case 2: SetPenColor("Black");
                SetPointSize(50);
                drawLabel(0,winheight-0.8,"��");
                drawLabel(winwidth/2+0.5,winheight-0.8,"��");
                SetPointSize(18);
                DrawChart(0,1,winwidth/2,winheight-2,head,table,0,interval[0]);
                DrawChart(winwidth/2,1,winwidth/2-0.5,winheight-2,head,table,1,interval[1]);break;
        case 3: SetPenColor("Black");
                SetPointSize(50);
                drawLabel(0,winheight-0.8,"��");
                drawLabel(winwidth/2,winheight-0.8,"��");
                drawLabel(0,winheight/2-0.5,"��");
                SetPointSize(18);
                DrawChart(0,winheight/2+0.5,winwidth/2-0.5,winheight/2-1,head,table,0,interval[0]);
                DrawChart(winwidth/2,winheight/2+0.5,winwidth/2-0.5,winheight/2-1,head,table,1,interval[1]);
                DrawChart(0,0.75,winwidth-1,winheight/2-1,head,table,2,interval[2]);
                break;
        case 4:
                SetPenColor("Black");
                SetPointSize(50);
                drawLabel(0,winheight-0.8,"��");
                drawLabel(winwidth/2-0.25,winheight-0.8,"��");
                drawLabel(0,winheight/2-0.5,"��");
                drawLabel(winwidth/2-0.25,winheight/2-0.5,"��");
                SetPointSize(18);
                DrawChart(0,winheight/2+0.5,winwidth/2-0.5,winheight/2-1,head,table,0,interval[0]);
                DrawChart(winwidth/2,winheight/2+0.5,winwidth/2-0.5,winheight/2-1,head,table,1,interval[1]);
                DrawChart(0,0.75,winwidth/2-0.5,winheight/2-1,head,table,2,interval[2]);
                DrawChart(winwidth/2,0.75,winwidth/2-0.5,winheight/2-1,head,table,3,interval[3]);
                break;
    }
    return TRUE;
}

BOOL DrawPredict(){
    static double X[MAX_DAYS]; //���֧��100�������Ԥ��
    static double Y[MAX_ITEMS][MAX_DAYS];
    static double coefs[MAX_ITEMS][MAX_ORDER+1];
    static char * s;
    static char max_order[2] = {MAX_ORDER+'0',0};
    static char order[2] = "2";
    static char interval[3] = "2";
    static char times[3] = "5";
    static int intervalD,timesD,orderD,x,y;
    DrawUnchangeSheet(winwidth*3/4);

    SetPenColor("Black");
    drawLabel(winwidth*3/4,winheight-4*fH,Concat(Concat("������Ͻ���(���Ϊ",max_order),"��)��"));
    textbox(GenUIID(0),winwidth*3/4,winheight- 8*fH,winwidth*1/5,2*fH,order,sizeof(order));

    SetPenColor("Black");
    drawLabel(winwidth*3/4,winheight-12*fH,"����Ԥ������������");
    textbox(GenUIID(0),winwidth*3/4,winheight- 16*fH,winwidth*1/5,2*fH,interval,sizeof(interval));

    SetPenColor("Black");
    drawLabel(winwidth*3/4,winheight-20*fH,"������ҪԤ��Ĵ�����");
    textbox(GenUIID(0),winwidth*3/4,winheight- 24*fH,winwidth*1/5,2*fH,times,sizeof(times));


    if(button(GenUIID(0),winwidth*3/4,winheight - 28*fH,winwidth*1/5, 2*fH,"Ԥ��")){
        SetCurrentTask("Ԥ��");
        if(!DataCheck(head)){
            MessageBox(NULL,"��������δ��д�ĵ�λ��","����",MB_ICONERROR);    
            task_status = 0;
            TaskFinished();
        }else if(predicted){
            MessageBox(NULL,"��������ϴ�Ԥ�������","����",MB_ICONERROR); 
            task_status = 0;
            TaskFinished();
        }else if(!atoi(order) || atoi(order) > MAX_ORDER || atoi(order) >= head->items[0] || !atoi(interval) || !atoi(times)){
            MessageBox(NULL,"�����趨���������������Ƿ��趨����(���ɳ��������Ŀǰ���ݱ��������Ҳ��ɳ���������)���Ƿ��в���Ϊ��!","����",MB_ICONERROR); 
            task_status = 0;
            TaskFinished();            
        }else{
            static int flag;
            flag = 1;
            setX(head->next,X);
            setY(head->next,Y,head->items[1]);

            for(int i = 0;i < head->items[1]-1;i++){
                if(!polyfit(X,Y[i],head->items[0],atoi(order),coefs[i])){
                    MessageBox(NULL,"���ʧ��","����",MB_ICONERROR);
                    flag = 0;
                    task_status = 0;
                    TaskFinished();
                };
            }
            
            if(flag){
                timesD = atoi(times);
                intervalD = atoi(interval);
                orderD = atoi(order);
                x = X[head->items[0]-1];
                for(int i=0;i < timesD;i++){
                    x += intervalD;
                    s = format_date(tail->date,intervalD);
                    tail = DataAddNewNode(head,tail);
                    tail->items[0] = 1;
                    strcpy(tail->date,s);
                    for(int j=0;j < head->items[1] -1;j++){
                        tail -> items[j+2] = polyval(coefs[j],orderD,x) > 0 ? polyval(coefs[j],orderD,x) : 0;
                    }
                }
                predicted = TRUE;
                display();
                task_status = 1;
                TaskFinished();
            }
        }
    }
    if(button(GenUIID(0),winwidth*3/4,winheight - 32*fH,winwidth*1/5, 2*fH,"ȥ��Ԥ�ⲿ�ֵ�����")){
        SetCurrentTask("ȥ��Ԥ�ⲿ�ֵ�����");
        static int count;
        static Data * p;
        p = head;
        count = 0;
        if(!p->next){
            task_status = 1;
            TaskFinished();
            return TRUE;
        }
        while ( p = p->next)
        {
            if( p->next == NULL || p->next->items[0] == 1){
                break;
            }
        }
        if(p->next == NULL){
            predicted = FALSE;
            task_status = 1;
            TaskFinished();
        }else{
            tail = p;
            
            while (p = p->next)
            {
                FreeBlock(p);
                count++;
            }

            head->items[0] -= count;
            tail -> next = NULL;
            predicted = FALSE;
            display();
            task_status = 1;
            TaskFinished();
        }
    }
    return TRUE;
};

void display(){
    DisplayClear(); // �������� 
    switch (Mode){
        case 0:
            DrawSheet();
            break;
        case 1:
            DrawGraph();
            break;
        case 2:
            DrawPredict();
            break;
        case 3:
        	DrawUnchangeSheet(winwidth);
        	break;
        default:
            SetPenColor("Red");
            SetPointSize(70);// �����ֺ�
            drawLabel(winwidth/2-TextStringWidth("��ӭʹ�ñ������")/2,winheight/2,"��ӭʹ�ñ������");
            SetPointSize(18);// �����ֺ�
            break;
    }
    if(!Capturing){
        DrawMenu();
        DrawStatusBar();
        DrawToolBar();
    }
}
