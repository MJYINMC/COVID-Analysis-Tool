#ifndef _Chart_H_
#define _Chart_H_
extern int ifdraw[4][MAX_ITEMS];/*main更改此数组，chart根据此数组画图 
								ifdraw的维数表示正在画的是第几张表
								ifdraw[n][0]代表该表的类型，2为折线图，1为条形图 
								从ifdraw[n][1]到ifdraw[n][MAX_ITEMS]各自表示对应的列的数据画不画，
								1表示画，0表示不画 
								最多可以有六列参与绘图 
*/ 

BOOL DrawChart(double x,double y,double w,double h,Data * head,char (*Table)[50],int dim,int interval);
BOOL DrawCoordinate(double x,double y,double w,double h,int days,int ddt,double xdivd,Data * head);
BOOL DrawDaycoordinate(double x,double y,double w,int days,double xdivd,Data * head);
BOOL DrawVerticalcoordinate(double x,double y,double w,double h,int ddt);
int Getmaxdata(Data * head);
BOOL DrawLegend(double x,double y,double w,double h,char (*Table)[50]);
BOOL Drawlinech(double x,double y,double w,double h,int days,int ddt,double xdivd,Data * head);
BOOL Drawbarch(double x,double y,double w,double h,int days,int ddt,double xdivd,Data*head);
int InBox(double x,double y,double w,double xdivd,int days);
BOOL PrintLinechartData(double x,double y,double w,double h,int days,int ddt,double xdivd,Data * head,int j);
BOOL PrintBarhartData(double x,double y,double w,double h,int days,int ddt,double xdivd,Data*head,int j);
BOOL Drawpiechart(double x,double y,double w,double h,Data*head,int cnty,int days); 
BOOL DrawPie(double x,double y,double r,int total,int cnty);

#endif
