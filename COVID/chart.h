#ifndef _Chart_H_
#define _Chart_H_
extern int ifdraw[4][MAX_ITEMS];/*main���Ĵ����飬chart���ݴ����黭ͼ 
								ifdraw��ά����ʾ���ڻ����ǵڼ��ű�
								ifdraw[n][0]����ñ�����ͣ�2Ϊ����ͼ��1Ϊ����ͼ 
								��ifdraw[n][1]��ifdraw[n][MAX_ITEMS]���Ա�ʾ��Ӧ���е����ݻ�������
								1��ʾ����0��ʾ���� 
								�����������в����ͼ 
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
