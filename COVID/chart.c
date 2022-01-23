#include <Windows.h>
#include <WinUser.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../libgraphics/graphics.h"
#include "../libgraphics/strlib.h"
#include "../libgraphics/extgraph.h"
#include "../libgraphics/random.h"
#include "../simpleGUI/imgui.h"

#include "data.h"
#include "main.h"
#include "fileOP.h"
#include "chart.h"
#include "math.h"

#define Y_CODI 8//纵轴刻度个数 
#define MAX_LENGTH 50//最大天数 
#define PI 3.1415926
BOOL initialized[4];
int ifdraw[4][MAX_ITEMS];//判断哪些列需要画的数组，已经extern出去 

static int nowday[4]={1,1,1,1};

static char buffer[MAX_LENGTH];// 缓存空间 

static double winwidth;
static double winheight;
static double xresolution;
static double yresolution;
static double Fh;//字体高度 
static int number;//需要绘制数据的列数 
static int Dim;//ifdraw的维数 
static int daymod;//间隔天数 
static Data*now[4];
static light[4][7]={0};

static char *linecolor[4][MAX_ITEMS];//颜色名称数组 


/*BOOL DrawChart(double x,double y,double w,double h,Data * head,char (*table)[50],int dim,int interval)的功能 
根据数据随机生成颜色（如果多余六列只生成六个）
调用获取最大值函数Getmaxdata(Data * head)，从而算出最大刻度。
调用画坐标轴函数DrawVerticalcoordinate(double x,double y,double w,double h,int ddt)画坐标轴和写坐标轴的文字。
调用画图例函数DrawLegend(double x,double y,double w,double h,char (*table)[50])画图例。
调用画折线Drawlinech(double x,double y,double w,double h,int days,int ddt,double xdivd,Data * head)或者画条形Drawbarch(double x,double y,double w,double h,int days,int ddt,double xdivd,Data*head)函数画图。
根据鼠标位置判断调用显示数据函数PrintLinechartData(double x,double y,double w,double h,int days,int ddt,double xdivd,Data * head,int j)或者PrintBarhartData(double x,double y,double w,double h,int days,int ddt,double xdivd,Data*head,int j)。
*/ 
BOOL DrawChart(double x,double y,double w,double h,Data * head,char (*table)[50],int dim,int interval)
{
	daymod = interval;
	Dim = dim;
	if(!initialized[Dim]){
		now[Dim] = head->next;
		initialized[Dim]=TRUE;
	}
	int chartmod = ifdraw[Dim][0];
	winwidth = GetWindowWidth();
	winheight = GetWindowHeight();
	xresolution = GetXResolution();
	yresolution = GetYResolution();

	Fh=GetFontHeight();
	
	int max,days,cmax,ddt,t,i=1,j,u;
	double cx,cy,chartw,charth;
	cx=x+w/10.0;
	cy=y+h/5.0;
	chartw=w*0.9;
	charth=h*0.8;
	double xdivd;
	
	number=0;
	for(t=1;t<MAX_ITEMS;t++){
		if(ifdraw[Dim][t]==TRUE)number++;
	}
	if(number>6)number=6;
	
	if(chartmod==3){
		number=head ->items[0];
		for(int i=1;i<=number;i++)ifdraw[Dim][i]=1;
	}
	
	Randomize();
	for(u=1;u<=number;u++){//随机生成颜色 
		if(linecolor[Dim][u]==NULL){
			double r = RandomReal(0, 1);
			double g = RandomReal(0, 1);
			double b = RandomReal(0, 1);
			linecolor[Dim][u] = RealToString(r+g+b);
			DefineColor(linecolor[Dim][u],r,g,b);
		}
	}
	days= ceil(1.0*head ->items[0]/interval);//确定要画的天数 
	
	max=Getmaxdata(head);//获得最大数据 
	xdivd=1.0*chartw/((double)days+1.0);
	
	if(max<100){
		i = max/10;
		if(i==0)cmax = 16;
		else cmax = (i+2)*10;
	}else{
		for(i=1;max/100>0;){
			max/=10;
			i*=10;
		}
		cmax=(max+1)*i;
	}
	
	ddt=cmax/Y_CODI;
	
	DrawLegend(cx,y+0.5*Fh,chartw,h*0.2-3.0*Fh,table);//画图例
	
	if(chartmod<=2){//画折线图或者条形图 
		DrawCoordinate(cx,cy,chartw,charth,days,ddt,xdivd,head);//画坐标轴 

		
		if(chartmod==1)Drawbarch(cx,cy,chartw,charth,days,ddt,xdivd,head); 
		if(chartmod==2)Drawlinech(cx,cy,chartw,charth,days,ddt,xdivd,head);
		ZeroMemory(light,sizeof(light));
		
		j = InBox(cx,cy-3.0*Fh,chartw,xdivd,days);
		if(j!=0) {//如果光标在相应位置，显示数据 
			if(chartmod==1)PrintBarhartData(cx,cy,chartw,charth,days,ddt,xdivd,head,j);
			if(chartmod==2)PrintLinechartData(cx,cy,chartw,charth,days,ddt,xdivd,head,j);
		}
	}else{//画饼状图 
		Drawpiechart(x,cy,w,charth,head,head ->items[1]-1,head ->items[0]); 
	}
	return TRUE;
}

/* BOOL DrawCoordinate(double x,double y,double w,double h,int days,int ddt,double xdivd,Data * head)的功能： 
调用画横轴函数DrawDaycoordinate(double x,double y,double w,int days,double xdivd,Data * head)写日期；
画基准线DrawLine(double x,double y);
调用画纵轴函数DrawVerticalcoordinate(double x,double y,double w,double h,int ddt)写刻度。
*/ 
BOOL DrawCoordinate(double x,double y,double w,double h,int days,int ddt,double xdivd,Data * head)
{
	SetPenColor("black");
	MovePen(x,y);
	DrawLine(w,0);
	DrawDaycoordinate(x,y-3.0*Fh,w,days,xdivd,head);
	
	SetPenColor("black");
	MovePen(x,y);
	DrawLine(0,h);

	DrawVerticalcoordinate(x,y,w,h,ddt);
	return TRUE;
}

/* BOOL DrawDaycoordinate(double x,double y,double w,int days,double xdivd,Data * head)的功能：
竖着写出横轴的文字。
*/ 
BOOL DrawDaycoordinate(double x,double y,double w,int days,double xdivd,Data * head)
{
	string date = NULL;
	HFONT Font = CreateFont(
    0/*高度*/, 0/*宽度*/, 900/*不用管*/, 0/*不用管*/, 400 /*一般这个值设为400*/,
    FALSE/*不带斜体*/, FALSE/*不带下划线*/, FALSE/*不带删除线*/,
    DEFAULT_CHARSET, //这里我们使用默认字符集，还有其他以 _CHARSET 结尾的常量可用
    OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, //这行参数不用管
    DEFAULT_QUALITY, //默认输出质量
    FF_DONTCARE, //不指定字体族*/
    "Times"); //字体名
    static RECT rect;
	HDC osdc = GetOsdc();
    SelectObject(osdc,Font);
	rect.top=(int)((winheight-y-4.0*Fh)*yresolution);
	for(int t=1,k=1;k<=days;t+=daymod,k++){
		date=GetData(head,t,1,buffer);
		if(*date=='#'){ 
			
			rect.bottom=(int)((winheight-y+4.0*Fh)*yresolution);
			rect.left=(int)((1.0*k*xdivd+x-0.8*Fh)*xresolution);
	    	rect.right=(int)((1.0*k*xdivd+x+1.3*Fh)*xresolution);

	    	DrawText(osdc,date,-1,&rect,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
	    	
		}else{
			rect.bottom=(int)((winheight-y+3.0*Fh)*yresolution);
			rect.left=(int)((1.0*k*xdivd+x-0.6*Fh)*xresolution);
	    	rect.right=(int)((1.0*k*xdivd+x+1.1*Fh)*xresolution);
			DrawText(osdc,date+1,-1,&rect,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
		}
	}

	SetFont("Times");
	return TRUE;
}

/*BOOL DrawVerticalcoordinate(double x,double y,double w,double h,int ddt)的功能：
写出纵轴的刻度。
*/ 
BOOL DrawVerticalcoordinate(double x,double y,double w,double h,int ddt)
{
	double sw;
	int t=0;
	char * wf = NULL;
	
	for(t=0;t<=Y_CODI;t++){
		MovePen(x,y+(h/(double)Y_CODI)*t);
		SetPenColor("Gray");
		DrawLine(w,0);
		if(wf != NULL) free(wf);
		wf=(string)malloc(sizeof(char)*10);
		sprintf(wf, "%d", t*ddt);
		sw=TextStringWidth(wf);

		SetPenColor("black");
		drawLabel(x-sw,y+(h/(double)Y_CODI)*t-Fh/4.0,wf);
	}
	
	return TRUE;
}

/*int Getmaxdata(Data * head)获取要使用数据中的最大数据并返回*/ 
int Getmaxdata(Data * head)
{
	int max = Y_CODI,m;
	int rows = head -> items[0];
    int columns = head ->items[1];
	for(int k = 0; k < rows && head -> next; k++){
		head = head -> next;
		if(k%daymod==0){
            for(int j = 2,u=1; j <= columns&&u<=number;j++){
            	if(ifdraw[Dim][j-1]){
	            	m = head->items[j];
	            
	                if(m>max){
	                    max=m;
	                }
                }
            }
        }
    }
	return max;
}

/*画图例，使用linecolor中的颜色画一小段粗线条*/ 
BOOL DrawLegend(double x,double y,double w,double h,char (*table)[50])
{
	int n = 1,t;
	int trow = (number-1)/3+1;
	
	double distance;
	distance = h/((int)((number-1)/3)+1.0);
	
	for(t=1;t<MAX_ITEMS&&n<=number;t++){
		if(ifdraw[Dim][t]){
			SetPenColor(linecolor[Dim][n]); 
			
			int row = (n-1) / 3;
			
			SetPenSize(10);//画线 
			if(n%3==1){
				MovePen(x,y+h-(row+1)*0.7*Fh);
				DrawLine(w/9.0,0);
				if(Mx>x&&Mx<x+w/8.0+2*Fh&&My>y+h-(row+1)*0.9*Fh&&My<y+h-(row+1)*0.9*Fh+Fh)light[Dim][n]=1;
			}else if(n%3==2){
				MovePen(x+w/3.0,y+h-(row+1)*0.7*Fh);
				DrawLine(w/9.0,0);
				if(Mx>x+w/3.0&&Mx<x+w/3.0+w/8.0+2*Fh&&My>y+h-(row+1)*0.9*Fh&&My<y+h-(row+1)*0.9*Fh+Fh)light[Dim][n]=1;
			}else{
				MovePen(x+w/1.5,y+h-(row+1)*0.7*Fh);
				DrawLine(w/9.0,0);
				if(Mx>x+w/1.5&&Mx<x+w/1.5+w/8.0+2*Fh&&My>y+h-(row+1)*0.9*Fh&&My<y+h-(row+1)*0.9*Fh+Fh)light[Dim][n]=1;
			}
			
			SetPenSize(1);//写字 
			SetPenColor("Black");
			if(n%3==1){
				drawLabel(x+w/8.0,y+h-(row+1)*0.9*Fh,table[t]);
			}else if(n%3==2){
				drawLabel(x+w/3.0+w/8.0,y+h-(row+1)*0.9*Fh,table[t]);
			}else{
				drawLabel(x+w/1.5+w/8.0,y+h-(row+1)*0.9*Fh,table[t]);
			}
			
			n++;
		}
	}
	return TRUE;
}

/*画折线，完成画折线图*/ 
BOOL Drawlinech(double x,double y,double w,double h,int days,int ddt,double xdivd,Data * head)
{
	double xy[8][2]={0.0};//保存上次数据的数组 
	int m,n,columns=head ->items[1],k = 1,u=1,q;
	for(q = 0;head -> next&&k<=days; q++){
		head = head -> next;
        if(q%daymod==0||(head -> next==NULL&&k==days)){
		    for(int p = 2,u=1; p <= columns&&u<=number;p++){
		    	if(ifdraw[Dim][p-1]){
	            	m = head->items[p];
					double x1=k*xdivd+x;
					double y1=(m/(ddt*(double)Y_CODI))*h+y;
					SetPenColor(linecolor[Dim][u]);
					MovePen(x1+0.02,y1);
	                DrawArc(0.02,0,360);
	                SetPenColor("Black"); 
	                if(xy[p-1][0]==0.0){//保存第一次数据 
	                	xy[p-1][0]=x1; 
	                	xy[p-1][1]=y1;
					}else{
						double x2=xy[p-1][0];//获取上次数据 
	                	double y2=xy[p-1][1];
	                	xy[p-1][0]=x1;//存储本次数据 
	                	xy[p-1][1]=y1;
	                	SetPenColor(linecolor[Dim][u]); 
	                
	                	MovePen(x2,y2);
	                	if(light[Dim][u]==1)SetPenSize(3);
	                	DrawLine(x1-x2,y1-y2);//画线连接 
	                	SetPenColor("Black"); 
	                	SetPenSize(1);
	                	u++;	
					}
		        }
		    }
		    k++;
		}
    }
    
	return TRUE;
}

/*画填充矩形完成画条形图*/ 
BOOL Drawbarch(double x,double y,double w,double h,int days,int ddt,double xdivd,Data*head)
{
	double bw = (xdivd-w/100.0)/number;//条形宽度 
	int m,columns=head ->items[1],k = 1,u=1,q;
	for(q = 0;head -> next&&k<=days; q++){
		head = head -> next;
        if(q%daymod==0||(head -> next==NULL&&k==days)){
		
		    for(int p = 2,u=1; p <= columns&&u<=number;p++){
		    	if(ifdraw[Dim][p-1]){
	            	m = head->items[p];
					double x1=k*xdivd-(xdivd-w/100.0)/2+(u-1.0)*bw+x;
					double y1=(m/(ddt*(double)Y_CODI))*h+y;
					
					SetPenColor(linecolor[Dim][u]); 
					
	                drawRectangle(x1, y, bw, y1-y, 1);
	                
	                SetPenColor("Black"); 
	                u++;
		        }
		    }
		    k++;
		}
    }
	return TRUE;
}

/*判断鼠标光标位置*/ 
int InBox(double x,double y,double w,double xdivd,int days)
{
	double mx=Mx;
	double my=My;
	int t=0,i;
	if(mx>x&&mx<x+w&&my>y&&my<y+3.0*Fh){
		for(i=1;i<=days;i++){
			double ux=i*xdivd+x;
			if(mx<ux+0.5*Fh&&mx>ux-0.5*Fh){
				t=i;
				break;
			}
		}
	}
	return t;
}

/*动态显示折线图数据*/ 
BOOL PrintLinechartData(double x,double y,double w,double h,int days,int ddt,double xdivd,Data * head,int j)
{
	string sd=NULL;
	int m,columns=head ->items[1],u=1,q;
	q = (j-1)*daymod+1;
	for(int k = 0; k < q && head -> next; k++){
		head = head -> next;
        }
    for(int p = 2; p <= columns&&u<=number;p++){
    	if(ifdraw[Dim][p-1]){
            	m = head->items[p];
            	if(sd!=NULL)free(sd);
				sd=(string)malloc(sizeof(char)*10);
				sprintf(sd, "%d", m);
				double dx=j*xdivd+x;
				double dy=(m/(ddt*(double)Y_CODI))*h+y;
				SetPenColor(linecolor[Dim][u]); 
                drawLabel(dx,dy+0.1*Fh,sd);
                SetPenColor("Black"); 
                u++;
            }
        }
	return TRUE;
}

/*动态显示条形图数据*/ 
BOOL PrintBarhartData(double x,double y,double w,double h,int days,int ddt,double xdivd,Data*head,int j)
{
	double bw = (xdivd-w/100.0)/number;
	string sd=NULL;
	int m,columns=head ->items[1],u=1,q;
	q = (j-1)*daymod+1;
	for(int k = 0; k < q && head -> next; k++){
		head = head -> next;
        }
    for(int p = 2; p <= columns&&u<=number;p++){
    	if(ifdraw[Dim][p-1]){
            	m = head->items[p];
            	if(sd!=NULL)free(sd);
				sd=(string)malloc(sizeof(char)*10);
				sprintf(sd, "%d", m);
				double g = TextStringWidth(sd)/2.0;
				double dx=j*xdivd-(xdivd-w/100.0)/2+(u-0.5)*bw+x-g;
				double dy=(m/(ddt*(double)Y_CODI))*h+y;
				SetPenColor(linecolor[Dim][u]); 
                drawLabel(dx,dy+0.1*Fh,sd);
                SetPenColor("Black"); 
                u++;
            }
        }
	return TRUE; 
}

BOOL Drawpiechart(double x,double y,double w,double h,Data*head,int cnty,int days)
{
	if(button(GenUIID(Dim),x+0.8*w,y+0.8*h-2*Fh,2*TextStringWidth("上一天"),2*Fh,"上一天")){
		if(nowday[Dim]==1)MessageBox(NULL,"已是第一天","警告",MB_ICONWARNING);
		else{
			nowday[Dim]--;
			now[Dim]=head;
			for(int i=0;i<nowday[Dim];i++){
				now[Dim]=now[Dim]->next;
			}
		}
	}
	if(button(GenUIID(Dim),x+0.8*w,y+0.2*h,2*TextStringWidth("下一天"),2*Fh,"下一天")){
		if(nowday[Dim]==days)MessageBox(NULL,"已是最后一天","警告",MB_ICONWARNING);
		else{
			nowday[Dim]++;
			now[Dim]=now[Dim]->next;
		}
	}
	
	
	string title = Concat(now[Dim]->date,"疫情数据饼状图");
	SetPenColor("black");
	drawLabel(x+0.4*w-TextStringWidth(title)/2.0,y-1.5*Fh,title);
	
	
	int total = 0;
	for(int j=1;j<=cnty;j++){
		total+=now[Dim]->items[j+1];
	}
	
	double r = 0.5*h;
	if(0.4*w<r)r=0.4*w;
	MovePen(x+0.4*w+r,y+0.5*h);
	SetPenColor("black");
	DrawArc(r, 0, 360);
	DrawPie(x+0.4*w,y+0.5*h,r,total,cnty);
	return TRUE;
}

BOOL DrawPie(double x,double y,double r,int total,int cnty)
{
	double nowang = 0;
	
	for(int i = 1;i<=cnty;i++){
		SetPenColor(linecolor[Dim][i]);
		StartFilledRegion(0.5);
		
		double adang = 1.0*(now[Dim]->items[i+1])/total*360;
		MovePen(x,y);
		DrawLine(r*cos(nowang*PI/180),r*sin(nowang*PI/180));
		DrawArc(r, nowang, adang);

		nowang+=adang;
		EndFilledRegion();
	}
	SetPenColor("black");
	
	for(int i = 1;i<=cnty;i++){
		
		double adang = 1.0*(now[Dim]->items[i+1])/total*360;
		double prs = ((int)(1.0*(now[Dim]->items[i+1])/total*10000))/100.0;
		string sd = Concat(RealToString(prs),"%");
		drawLabel(x+r*cos((nowang+adang/2.0)*PI/180)/2.0,y+r*sin((nowang+adang/2.0)*PI/180)/2.0,sd);

		nowang+=adang;
	}
	
	return TRUE;
}
