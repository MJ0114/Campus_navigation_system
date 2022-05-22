#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<malloc.h>
#include<time.h>
#include<assert.h> 
#include<iostream>
#include<ctime> 
#include <windows.h>
#include<graphics.h>
#include<cmath>

#define FileName "123.txt"					//日志文件为"123.txt"
#define va_max 3
#define MAX_Buildings 200					//建筑物数目的数组大小为200
#define INF 99999							//规定99999为无限大
#define FALSE 0								//规定false为0
#define TRUE 1								//规定true为1
#define ROADFILE "schoolroad1.txt"//"road111.txt"	//为记录学校道路信息的文件
#define BUILDFILE "schoolbuild1.txt"//"build111.txt"	//为记录学校建筑物信息的页面
#define maxbuilds 220									//建筑物最大数目为220
#define lineThick 6							//实现图形化界面需要
#define textsize 14							//实现图像化页面需要

#define WESTMAX 48							//实现图像化页面需要
#define NORTHMIN 49							//实现图形化页面需要
#define NORTHMAX 84							//实现图像化页面需要
#define SOUTHMIN 85							//实现图像化页面需要
#define SOUTHMAX 123						//实现图像化页面需要

#define speedw 10							//常规速度
#define walk_speed 10						//步行速度
#define bike_speed 30						//自行车速度

struct building;
struct road;
struct graph;

int departure, destination;//r[va_max];

struct building {
	int b_number;//建筑物编号 
	char b_name[21];//建筑真实名称，最多十个汉字 
	char b_names[5][21];// 逻辑位置 
	struct road* first_road; //顶点连的边 
};

struct road {
	bool mark;
	int leftbuild, rightbuild;//该路连接建筑的编号 
	struct road* leftnext, * rightnext;//两端顶点的下一条边 
//	infotype * info;//该边信息指针：拥挤度，长度，可选交通工具（名字） 
//上述信息有可能直接放在这里面。
	int distan;//10n
	float crowd;//0-1
	int tranmark;//交通工具 
};

struct  graph {
	struct building buildings[MAX_Buildings];
	int buildnum, roadnum;
};

struct graph* GP;//全局地图指针 


typedef struct schoolBuild {
	int x, y;
	int num;
	const char* logclLoc;
}Build;

typedef struct schoolRoad {
	int num1, num2;
	int distance;
	double crwDge;
	int bicycle;
}Road;

void shortlongth(int first);//源点到其他所有点的最短路径P及距离D 
void va_shortlongth(int first, int last, int va);
void outroads(int first, int second);
void outroads2(int first, int second);
void getgraphtxt(struct graph* Gp);
int get_longth(int a, int b);
int theNearestBuilding(int tempx, int tempy, int x1, int y1, int x2, int y2);
void short_time(int first);
void tran_short(int first);
int getRoads(int first, int second, int num);

int P[MAX_Buildings][MAX_Buildings];
int D[MAX_Buildings];
int final[MAX_Buildings];
int Roads[MAX_Buildings];
int roundb[20], rd = 150;
int Test = 0, buildBy = 0;
int Buildby;

int search(int now_location) {//搜索now_location范围rd内所有建筑物并存入数组round[]中 
	shortlongth(now_location);
	int rdnum = 0;
	if (now_location <= WESTMAX) {
		for (int i = 0; i <= WESTMAX; i++) {//i是节点编号，D[i]是到now_location的距离 
			if (D[i] <= rd && i != now_location) {
				roundb[rdnum] = i;
				rdnum++;
			}
		}
		return rdnum;
	}
	if (now_location >= NORTHMIN && now_location <= NORTHMAX) {
		for (int i = NORTHMIN; i <= NORTHMAX; i++) {//i是节点编号，D[i]是到now_location的距离 
			if (D[i] <= rd && i != now_location) {
				roundb[rdnum] = i;
				rdnum++;
			}
		}
		return rdnum;
	}

	else {
		for (int i = SOUTHMIN; i <= SOUTHMAX; i++) {//i是节点编号，D[i]是到now_location的距离 
			if (D[i] <= rd && i != now_location) {
				roundb[rdnum] = i;
				rdnum++;
			}
		}
		return rdnum;
	}
}

int getTime(char* out, int fmt)                 // 获取当前系统时间
{
	if (out == NULL)
		return -1;

	time_t t;
	struct tm* tp = 0;
	t = time(NULL);

	localtime_s(tp, &t);
	if (fmt == 0)
		sprintf_s(out, 200, "%2.2d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
	return 0;
}

int writeFile(FILE* fp, const char* str, int bLog)          // 写字符串到文件,bLog表明是否为日志文件
{
	assert(fp != NULL && str != NULL);//
	char curTime[100] = { 0 };
	int ret = -1;
	ret = fprintf(fp, "%s\n", str);

	if (ret >= 0)
	{
		fflush(fp);
		return 0;               // 写文件成功
	}
	else
		return -1;
}

void get_roads(int first, int second) {	//获取当前路径并存到Roads[] 
	int i, j, k = -1, d[MAX_Buildings];
	j = second;
	for (i = 0; i < (*GP).buildnum; i++) {
		if (P[j][i] == TRUE) {
			if (i != first) {
				d[++k] = j;
				j = i;
				i = -1;
			}
			else {
				d[++k] = j;
				d[++k] = i;
				break;
			}
		}
	}
	buildBy = k + 1;//总共经过建筑的个数 
	for (i = 0; k > 0; k--, i++) {
		printf("%d->", d[k]);
		Roads[i] = d[k];
	}
	Roads[i] = d[k];
	printf("%d\n", d[k]);
}

int main() {
	struct graph G;
	GP = &G;
	getgraphtxt(&G);
	FILE* fp;
	int whichPicture;//确定此时加载的是哪幅图片：0.主页；1.西；2.北；3.南
	int navigationInProgress = 0;//标记是否正在导航
	int startPosition = -1, terminalPosition = -1, navigationStrategy = -1;//起始位置，终点位置，导航策略
	int speed;//导航速度
	int changeBreak = 0;//导航中改变
	int nearrstbdg;//左右最近节点
	int queryPoint;//查询点
	int mid;//途径点
	HWND hnd = initgraph(1400, 780, EW_SHOWCONSOLE);
	SetWindowText(hnd, L"校园地图导览系统");
	loadimage(NULL, _T("homepage.jpg"));
	whichPicture = 0;
	setbkmode(TRANSPARENT);//设置文字背景透明

	setlinecolor(RGB(142, 166, 165));//灰色，道路颜色
	setfillcolor(RGB(235, 123, 123));//红色，建筑物节点颜色
	setlinestyle(PS_SOLID | PS_JOIN_BEVEL, lineThick);

	Build build[124];
	Road road[221];
	int count = 0, number = 0;

	SYSTEMTIME time;//当下时间

	{
		//西校区建筑物
		build[count].x = 300, build[count].y = 100, build[count].num = number; build[count].logclLoc = "北门"; count++, number++;//0.北门
		build[count].x = 300, build[count].y = 130, build[count].num = number; build[count].logclLoc = "快递站"; count++, number++;//1.快递站
		build[count].x = 300, build[count].y = 300, build[count].num = number; build[count].logclLoc = "学生活动中心"; count++, number++;//2.学生活动中心
		build[count].x = 200, build[count].y = 300, build[count].num = number; build[count].logclLoc = "操场"; count++, number++;//3.操场
		build[count].x = 200, build[count].y = 240, build[count].num = number; build[count].logclLoc = "篮球场"; count++, number++;//4.篮球场
		build[count].x = 140, build[count].y = 240, build[count].num = number; build[count].logclLoc = "排球场"; count++, number++;//5.排球场
		build[count].x = 140, build[count].y = 360, build[count].num = number; build[count].logclLoc = "网球场"; count++, number++;//6.网球场
		build[count].x = 200, build[count].y = 360, build[count].num = number; build[count].logclLoc = "拓展馆"; count++, number++;//7.拓展馆
		build[count].x = 220, build[count].y = 560, build[count].num = number; build[count].logclLoc = "礼堂"; count++, number++;//8.礼堂
		build[count].x = 300, build[count].y = 560, build[count].num = number; build[count].logclLoc = "观景湖"; count++, number++;//9.观景湖
		build[count].x = 300, build[count].y = 690, build[count].num = number; build[count].logclLoc = "科研楼"; count++, number++;//10.科研楼
		build[count].x = 550, build[count].y = 560, build[count].num = number; build[count].logclLoc = "行政楼"; count++, number++;//11.行政楼
		build[count].x = 480, build[count].y = 560, build[count].num = number; build[count].logclLoc = "励志楼"; count++, number++;//12.励志楼
		build[count].x = 300, build[count].y = 400, build[count].num = number; build[count].logclLoc = "励学楼"; count++, number++;//13.励学楼
		build[count].x = 550, build[count].y = 400, build[count].num = number; build[count].logclLoc = "博学楼"; count++, number++;//14.博学楼
		build[count].x = 470, build[count].y = 300, build[count].num = number; build[count].logclLoc = "启智楼"; count++, number++;//15.启智楼
		build[count].x = 400, build[count].y = 400, build[count].num = number; build[count].logclLoc = "图书馆"; count++, number++;//16.图书馆
		build[count].x = 550, build[count].y = 300, build[count].num = number; build[count].logclLoc = "学生餐厅"; count++, number++;//17.学生餐厅
		build[count].x = 550, build[count].y = 220, build[count].num = number; build[count].logclLoc = "浴室"; count++, number++;//18.浴室
		build[count].x = 630, build[count].y = 220, build[count].num = number; build[count].logclLoc = "超市"; count++, number++;//19.超市
		build[count].x = 670, build[count].y = 340, build[count].num = number; build[count].logclLoc = "洗衣房"; count++, number++;//20.洗衣房
		build[count].x = 630, build[count].y = 300, build[count].num = number; build[count].logclLoc = "男生宿舍A区"; count++, number++;//21.男生宿舍A区
		build[count].x = 630, build[count].y = 380, build[count].num = number; build[count].logclLoc = "女生宿舍A区"; count++, number++;//22.女生宿舍A区
		build[count].x = 680, build[count].y = 690, build[count].num = number; build[count].logclLoc = "计算机学院楼"; count++, number++;//23.计算机学院楼
		build[count].x = 900, build[count].y = 690, build[count].num = number; build[count].logclLoc = "经济管理学院楼"; count++, number++;//24.经济管理学院楼
		build[count].x = 900, build[count].y = 400, build[count].num = number; build[count].logclLoc = "人工智能学院楼"; count++, number++;//25.人工智能学院楼
		build[count].x = 1250, build[count].y = 400, build[count].num = number; build[count].logclLoc = "外卖点"; count++, number++;//26.外卖点
		build[count].x = 1350, build[count].y = 400, build[count].num = number; build[count].logclLoc = "东门"; count++, number++;//27.东门
		build[count].x = 220, build[count].y = 690, build[count].num = number; build[count].logclLoc = "实验楼"; count++, number++;//28.实验楼
		build[count].x = 480, build[count].y = 690, build[count].num = number; build[count].logclLoc = "医务室"; count++, number++;//29.医务室
		build[count].x = 900, build[count].y = 480, build[count].num = number; build[count].logclLoc = "体育馆"; count++, number++;//30.体育馆
		build[count].x = 1170, build[count].y = 480, build[count].num = number; build[count].logclLoc = "男生宿舍B区"; count++, number++;//31.男生宿舍B区
		build[count].x = 900, build[count].y = 550, build[count].num = number; build[count].logclLoc = "咖啡厅"; count++, number++;//32.咖啡厅
		build[count].x = 1170, build[count].y = 550, build[count].num = number; build[count].logclLoc = "纪念碑"; count++, number++;//33.纪念碑
		build[count].x = 900, build[count].y = 620, build[count].num = number; build[count].logclLoc = "多媒体艺术中心"; count++, number++;//34.多媒体艺术中心
		build[count].x = 1170, build[count].y = 620, build[count].num = number; build[count].logclLoc = "女生宿舍B区"; count++, number++;//35.女生宿舍B区
		build[count].x = 700, build[count].y = 400, build[count].num = number; build[count].logclLoc = "美术学院楼"; count++, number++;//36.美术学院楼
		build[count].x = 700, build[count].y = 550, build[count].num = number; build[count].logclLoc = "主楼"; count++, number++;//37.主楼
		build[count].x = 1110, build[count].y = 400, build[count].num = number; build[count].logclLoc = "治安亭"; count++, number++;//38.治安亭
		build[count].x = 1110, build[count].y = 160, build[count].num = number; build[count].logclLoc = "家属1号公寓"; count++, number++;//39.家属1号公寓
		build[count].x = 960, build[count].y = 160, build[count].num = number; build[count].logclLoc = "家属2号公寓"; count++, number++;//40.家属2号公寓
		build[count].x = 960, build[count].y = 340, build[count].num = number; build[count].logclLoc = "博士生公寓楼"; count++, number++;//41.博士生公寓楼
		build[count].x = 850, build[count].y = 160, build[count].num = number; build[count].logclLoc = "家属3号公寓"; count++, number++;//42.家属3号公寓
		build[count].x = 770, build[count].y = 240, build[count].num = number; build[count].logclLoc = "研究生公寓楼"; count++, number++;//43.研究生公寓楼
		build[count].x = 760, build[count].y = 160, build[count].num = number; build[count].logclLoc = "家属4号公寓"; count++, number++;//44.家属4号公寓
		build[count].x = 1110, build[count].y = 340, build[count].num = number; build[count].logclLoc = "科学研究所"; count++, number++;//45.科学研究所
		build[count].x = 1220, build[count].y = 230, build[count].num = number; build[count].logclLoc = "喷泉"; count++, number++;//46.喷泉
		build[count].x = 1220, build[count].y = 120, build[count].num = number; build[count].logclLoc = "附属中学"; count++, number++;//47.附属中学 
		build[count].x = 1220, build[count].y = 340, build[count].num = number; build[count].logclLoc = "特色餐厅"; count++, number++;//48.特色餐厅

		//北校区建筑物
		build[count].x = 700, build[count].y = 740, build[count].num = number; build[count].logclLoc = "北区南门"; count++, number++;//49.北区南门
		build[count].x = 650, build[count].y = 690, build[count].num = number; build[count].logclLoc = "小花园"; count++, number++;//50.小花园
		build[count].x = 700, build[count].y = 640, build[count].num = number; build[count].logclLoc = "北区实验楼"; count++, number++;//51.北区实验楼
		build[count].x = 800, build[count].y = 640, build[count].num = number; build[count].logclLoc = "北区礼堂"; count++, number++;//52.北区礼堂
		build[count].x = 1100, build[count].y = 640, build[count].num = number; build[count].logclLoc = "游泳馆"; count++, number++;//53.游泳馆
		build[count].x = 1100, build[count].y = 490, build[count].num = number; build[count].logclLoc = "羽毛球场"; count++, number++;//54.羽毛球场
		build[count].x = 800, build[count].y = 490, build[count].num = number; build[count].logclLoc = "北区操场"; count++, number++;//55.北区操场
		build[count].x = 750, build[count].y = 490, build[count].num = number; build[count].logclLoc = "北区网球场"; count++, number++;//56.北区网球场
		build[count].x = 700, build[count].y = 440, build[count].num = number; build[count].logclLoc = "教学楼A"; count++, number++;//57.教学楼A
		build[count].x = 700, build[count].y = 540, build[count].num = number; build[count].logclLoc = "教学楼B"; count++, number++;//58.教学楼B
		build[count].x = 650, build[count].y = 640, build[count].num = number; build[count].logclLoc = "男生宿舍C"; count++, number++;//59.男生宿舍C
		build[count].x = 650, build[count].y = 540, build[count].num = number; build[count].logclLoc = "男生宿舍B"; count++, number++;//60.男生宿舍B
		build[count].x = 650, build[count].y = 440, build[count].num = number; build[count].logclLoc = "男生宿舍A"; count++, number++;//61.男生宿舍A
		build[count].x = 650, build[count].y = 340, build[count].num = number; build[count].logclLoc = "北区超市"; count++, number++;//62.北区超市
		build[count].x = 700, build[count].y = 340, build[count].num = number; build[count].logclLoc = "北区主楼"; count++, number++;//63.北区主楼
		build[count].x = 750, build[count].y = 340, build[count].num = number; build[count].logclLoc = "雕塑"; count++, number++;//64.雕塑
		build[count].x = 800, build[count].y = 340, build[count].num = number; build[count].logclLoc = "小超市"; count++, number++;//65.小超市
		build[count].x = 1100, build[count].y = 340, build[count].num = number; build[count].logclLoc = "菜鸟裹裹"; count++, number++;//66.菜鸟裹裹
		build[count].x = 1350, build[count].y = 340, build[count].num = number; build[count].logclLoc = "北区东门"; count++, number++;//67.北区东门
		build[count].x = 1100, build[count].y = 130, build[count].num = number; build[count].logclLoc = "北区排球场"; count++, number++;//68.北区排球场
		build[count].x = 800, build[count].y = 130, build[count].num = number; build[count].logclLoc = "北区足球场"; count++, number++;//69.北区足球场
		build[count].x = 750, build[count].y = 130, build[count].num = number; build[count].logclLoc = "北区篮球场"; count++, number++;//70.北区篮球场
		build[count].x = 700, build[count].y = 130, build[count].num = number; build[count].logclLoc = "行政楼A"; count++, number++;//71.行政楼A
		build[count].x = 1025, build[count].y = 565, build[count].num = number; build[count].logclLoc = "行政楼B"; count++, number++;//72.行政楼B
		build[count].x = 450, build[count].y = 540, build[count].num = number; build[count].logclLoc = "人工湖"; count++, number++;//73.人工湖
		build[count].x = 650, build[count].y = 130, build[count].num = number; build[count].logclLoc = "北区咖啡厅"; count++, number++;//74.北区咖啡厅
		build[count].x = 650, build[count].y = 240, build[count].num = number; build[count].logclLoc = "北区洗衣房"; count++, number++;//75.北区洗衣房
		build[count].x = 550, build[count].y = 130, build[count].num = number; build[count].logclLoc = "新食堂"; count++, number++;//76.新食堂
		build[count].x = 550, build[count].y = 240, build[count].num = number; build[count].logclLoc = "食堂"; count++, number++;//77.食堂
		build[count].x = 440, build[count].y = 240, build[count].num = number; build[count].logclLoc = "西餐厅"; count++, number++;//78.西餐厅
		build[count].x = 550, build[count].y = 340, build[count].num = number; build[count].logclLoc = "水果店"; count++, number++;//79.水果店
		build[count].x = 200, build[count].y = 340, build[count].num = number; build[count].logclLoc = "北区外卖点"; count++, number++;//80.北区外卖点
		build[count].x = 550, build[count].y = 440, build[count].num = number; build[count].logclLoc = "女生宿舍A"; count++, number++;//81.女生宿舍A
		build[count].x = 550, build[count].y = 540, build[count].num = number; build[count].logclLoc = "女生宿舍B"; count++, number++;//82.女生宿舍B
		build[count].x = 550, build[count].y = 640, build[count].num = number; build[count].logclLoc = "女生宿舍C"; count++, number++;//83.女生宿舍C
		build[count].x = 50, build[count].y = 340, build[count].num = number; build[count].logclLoc = "小西门"; count++, number++;//84.小西门

		//南校区建筑物
		build[count].x = 700, build[count].y = 740, build[count].num = number; build[count].logclLoc = "南大门"; count++, number++;//85.南大门
		build[count].x = 700, build[count].y = 700, build[count].num = number; build[count].logclLoc = "南区快递站"; count++, number++;//86.南区快递站
		build[count].x = 700, build[count].y = 620, build[count].num = number; build[count].logclLoc = "南区网球场"; count++, number++;//87.南区网球场
		build[count].x = 1020, build[count].y = 620, build[count].num = number; build[count].logclLoc = "南区操场"; count++, number++;//88.南区操场
		build[count].x = 300, build[count].y = 620, build[count].num = number; build[count].logclLoc = "南区篮球场"; count++, number++;//89.南区篮球场
		build[count].x = 880, build[count].y = 520, build[count].num = number; build[count].logclLoc = "人文学院楼"; count++, number++;//90.人文学院楼
		build[count].x = 760, build[count].y = 520, build[count].num = number; build[count].logclLoc = "南区计算机学院楼"; count++, number++;//91.南区计算机学院楼
		build[count].x = 700, build[count].y = 520, build[count].num = number; build[count].logclLoc = "校训石"; count++, number++;//92.校训石
		build[count].x = 580, build[count].y = 520, build[count].num = number; build[count].logclLoc = "南区雕塑"; count++, number++;//93.南区雕塑
		build[count].x = 460, build[count].y = 520, build[count].num = number; build[count].logclLoc = "经管院大楼"; count++, number++;//94.经管院大楼
		build[count].x = 340, build[count].y = 520, build[count].num = number; build[count].logclLoc = "人工智能学院大楼"; count++, number++;//95.人工智能学院大楼
		build[count].x = 820, build[count].y = 460, build[count].num = number; build[count].logclLoc = "南区小花园"; count++, number++;//96.南区小花园
		build[count].x = 730, build[count].y = 460, build[count].num = number; build[count].logclLoc = "南区咖啡厅"; count++, number++;//97.南区咖啡厅
		build[count].x = 520, build[count].y = 460, build[count].num = number; build[count].logclLoc = "南区喷泉"; count++, number++;//98.南区喷泉
		build[count].x = 400, build[count].y = 460, build[count].num = number; build[count].logclLoc = "花园"; count++, number++;//99.花园
		build[count].x = 880, build[count].y = 400, build[count].num = number; build[count].logclLoc = "立知楼"; count++, number++;//100.立知楼
		build[count].x = 760, build[count].y = 400, build[count].num = number; build[count].logclLoc = "春晖楼"; count++, number++;//101.春晖楼
		build[count].x = 700, build[count].y = 400, build[count].num = number; build[count].logclLoc = "主实验楼"; count++, number++;//102.主实验楼
		build[count].x = 580, build[count].y = 400, build[count].num = number; build[count].logclLoc = "思齐楼"; count++, number++;//103.思齐楼
		build[count].x = 460, build[count].y = 400, build[count].num = number; build[count].logclLoc = "马克思主义学院楼"; count++, number++;//104.马克思主义学院楼
		build[count].x = 340, build[count].y = 400, build[count].num = number; build[count].logclLoc = "南区礼堂"; count++, number++;//105.南区礼堂
		build[count].x = 410, build[count].y = 300, build[count].num = number; build[count].logclLoc = "南区水果店"; count++, number++;//106.南区水果店
		build[count].x = 980, build[count].y = 300, build[count].num = number; build[count].logclLoc = "南区洗衣房"; count++, number++;//107.南区洗衣房
		build[count].x = 810, build[count].y = 300, build[count].num = number; build[count].logclLoc = "江渔堂（食堂）"; count++, number++;//108.江渔堂（食堂）
		build[count].x = 700, build[count].y = 300, build[count].num = number; build[count].logclLoc = "南区超市"; count++, number++;//109.南区超市
		build[count].x = 530, build[count].y = 300, build[count].num = number; build[count].logclLoc = "悯农堂（食堂）"; count++, number++;//110.悯农堂（食堂）
		build[count].x = 1190, build[count].y = 300, build[count].num = number; build[count].logclLoc = "小水果店"; count++, number++;//111.小水果店
		build[count].x = 270, build[count].y = 300, build[count].num = number; build[count].logclLoc = "小超市"; count++, number++;//112.小超市
		build[count].x = 1290, build[count].y = 200, build[count].num = number; build[count].logclLoc = "室内体育馆"; count++, number++;//113.室内体育馆
		build[count].x = 1190, build[count].y = 200, build[count].num = number; build[count].logclLoc = "蓝田馆"; count++, number++;//114.蓝田馆
		build[count].x = 980, build[count].y = 200, build[count].num = number; build[count].logclLoc = "丹阳馆"; count++, number++;//115.丹阳馆
		build[count].x = 810, build[count].y = 200, build[count].num = number; build[count].logclLoc = "青溪馆"; count++, number++;//116.青溪馆
		build[count].x = 700, build[count].y = 200, build[count].num = number; build[count].logclLoc = "邮局"; count++, number++;//117.邮局
		build[count].x = 530, build[count].y = 200, build[count].num = number; build[count].logclLoc = "松竹居"; count++, number++;//118.松竹居
		build[count].x = 410, build[count].y = 200, build[count].num = number; build[count].logclLoc = "齐贤居"; count++, number++;//119.齐贤居
		build[count].x = 270, build[count].y = 200, build[count].num = number; build[count].logclLoc = "偕行居"; count++, number++;//120.偕行居
		build[count].x = 170, build[count].y = 200, build[count].num = number; build[count].logclLoc = "小篮球场"; count++, number++;//121.小篮球场
		build[count].x = 700, build[count].y = 160, build[count].num = number; build[count].logclLoc = "面包房"; count++, number++;//122.面包房
		build[count].x = 700, build[count].y = 100, build[count].num = number; build[count].logclLoc = "南区北门"; count++, number++;//123.南区北门

		count = 0;
		//西校区道路
		road[count].num1 = 0, road[count].num2 = 1; road[count].crwDge = 0.3; road[count].bicycle = 0;     count++;
		road[count].num1 = 1, road[count].num2 = 2; road[count].crwDge = 0.2; road[count].bicycle = 1;     count++;
		road[count].num1 = 2, road[count].num2 = 3; road[count].crwDge = 0.6; road[count].bicycle = 1;     count++;
		road[count].num1 = 3, road[count].num2 = 4; road[count].crwDge = 0.3; road[count].bicycle = 0;     count++;
		road[count].num1 = 3, road[count].num2 = 5; road[count].crwDge = 0.7; road[count].bicycle = 0;     count++;
		road[count].num1 = 3, road[count].num2 = 6; road[count].crwDge = 0.9; road[count].bicycle = 0;     count++;
		road[count].num1 = 3, road[count].num2 = 7; road[count].crwDge = 0.1; road[count].bicycle = 0;     count++;
		road[count].num1 = 5, road[count].num2 = 6; road[count].crwDge = 0.2; road[count].bicycle = 0;     count++;
		road[count].num1 = 2, road[count].num2 = 15; road[count].crwDge = 0.6; road[count].bicycle = 1;    count++;
		road[count].num1 = 2, road[count].num2 = 13; road[count].crwDge = 0.7; road[count].bicycle = 0;    count++;
		road[count].num1 = 13, road[count].num2 = 16; road[count].crwDge = 0.9; road[count].bicycle = 0;   count++;
		road[count].num1 = 9, road[count].num2 = 12; road[count].crwDge = 0.1; road[count].bicycle = 1;    count++;
		road[count].num1 = 9, road[count].num2 = 13; road[count].crwDge = 0.6; road[count].bicycle = 1;    count++;
		road[count].num1 = 8, road[count].num2 = 9; road[count].crwDge = 0.4; road[count].bicycle = 0;     count++;
		road[count].num1 = 11, road[count].num2 = 12; road[count].crwDge = 0.5; road[count].bicycle = 0;   count++;
		road[count].num1 = 15, road[count].num2 = 17; road[count].crwDge = 0.2; road[count].bicycle = 0;   count++;
		road[count].num1 = 14, road[count].num2 = 16; road[count].crwDge = 0.3; road[count].bicycle = 1;   count++;
		road[count].num1 = 9, road[count].num2 = 10; road[count].crwDge = 0.4;  road[count].bicycle = 1;   count++;
		road[count].num1 = 10, road[count].num2 = 28; road[count].crwDge = 0.9; road[count].bicycle = 0;   count++;
		road[count].num1 = 11, road[count].num2 = 14; road[count].crwDge = 0.1; road[count].bicycle = 1;   count++;
		road[count].num1 = 14, road[count].num2 = 17; road[count].crwDge = 0.8; road[count].bicycle = 0;   count++;
		road[count].num1 = 17, road[count].num2 = 18; road[count].crwDge = 0.3; road[count].bicycle = 0;   count++;
		road[count].num1 = 18, road[count].num2 = 19; road[count].crwDge = 0.6; road[count].bicycle = 0;   count++;
		road[count].num1 = 17, road[count].num2 = 19; road[count].crwDge = 0.7; road[count].bicycle = 0;   count++;
		road[count].num1 = 17, road[count].num2 = 21; road[count].crwDge = 0.8; road[count].bicycle = 0;   count++;
		road[count].num1 = 17, road[count].num2 = 22; road[count].crwDge = 0.5; road[count].bicycle = 1;   count++;
		road[count].num1 = 19, road[count].num2 = 21; road[count].crwDge = 0.1; road[count].bicycle = 0;   count++;
		road[count].num1 = 20, road[count].num2 = 21; road[count].crwDge = 0.1; road[count].bicycle = 0;   count++;
		road[count].num1 = 20, road[count].num2 = 22; road[count].crwDge = 0.6; road[count].bicycle = 0;   count++;
		road[count].num1 = 11, road[count].num2 = 23; road[count].crwDge = 0.4; road[count].bicycle = 1;   count++;
		road[count].num1 = 23, road[count].num2 = 24; road[count].crwDge = 0.5; road[count].bicycle = 1;   count++;
		road[count].num1 = 24, road[count].num2 = 34; road[count].crwDge = 1;   road[count].bicycle = 0;   count++;
		road[count].num1 = 14, road[count].num2 = 36; road[count].crwDge = 0.3; road[count].bicycle = 1;   count++;
		road[count].num1 = 25, road[count].num2 = 36; road[count].crwDge = 0.6; road[count].bicycle = 1;   count++;
		road[count].num1 = 25, road[count].num2 = 38; road[count].crwDge = 0.5; road[count].bicycle = 1;   count++;
		road[count].num1 = 26, road[count].num2 = 27; road[count].crwDge = 0.5; road[count].bicycle = 1;   count++;
		road[count].num1 = 8, road[count].num2 = 28; road[count].crwDge = 0.1;  road[count].bicycle = 1;   count++;
		road[count].num1 = 12, road[count].num2 = 29; road[count].crwDge = 0.4; road[count].bicycle = 0;   count++;
		road[count].num1 = 10, road[count].num2 = 29; road[count].crwDge = 0.6; road[count].bicycle = 1;   count++;
		road[count].num1 = 23, road[count].num2 = 29; road[count].crwDge = 0.2; road[count].bicycle = 1;   count++;
		road[count].num1 = 36, road[count].num2 = 37; road[count].crwDge = 0.1; road[count].bicycle = 1;   count++;
		road[count].num1 = 32, road[count].num2 = 37; road[count].crwDge = 0.6; road[count].bicycle = 1;   count++;
		road[count].num1 = 34, road[count].num2 = 35; road[count].crwDge = 0.8; road[count].bicycle = 1;   count++;
		road[count].num1 = 32, road[count].num2 = 33; road[count].crwDge = 0.4; road[count].bicycle = 1;   count++;
		road[count].num1 = 32, road[count].num2 = 34; road[count].crwDge = 0.6; road[count].bicycle = 0;   count++;
		road[count].num1 = 33, road[count].num2 = 35; road[count].crwDge = 0.3; road[count].bicycle = 0;   count++;
		road[count].num1 = 30, road[count].num2 = 32; road[count].crwDge = 0.1; road[count].bicycle = 0;   count++;
		road[count].num1 = 30, road[count].num2 = 31; road[count].crwDge = 0.4; road[count].bicycle = 1;   count++;
		road[count].num1 = 31, road[count].num2 = 33; road[count].crwDge = 0.5; road[count].bicycle = 0;   count++;
		road[count].num1 = 26, road[count].num2 = 38; road[count].crwDge = 0.4; road[count].bicycle = 0;   count++;
		road[count].num1 = 38, road[count].num2 = 45; road[count].crwDge = 0.6; road[count].bicycle = 0;   count++;
		road[count].num1 = 39, road[count].num2 = 45; road[count].crwDge = 0.3; road[count].bicycle = 1;   count++;
		road[count].num1 = 45, road[count].num2 = 46; road[count].crwDge = 0.1; road[count].bicycle = 1;   count++;
		road[count].num1 = 46, road[count].num2 = 47; road[count].crwDge = 0.3; road[count].bicycle = 0;   count++;
		road[count].num1 = 46, road[count].num2 = 48; road[count].crwDge = 0.9; road[count].bicycle = 0;   count++;
		road[count].num1 = 45, road[count].num2 = 47; road[count].crwDge = 0.8; road[count].bicycle = 1;   count++;
		road[count].num1 = 45, road[count].num2 = 48; road[count].crwDge = 0.7; road[count].bicycle = 0;   count++;
		road[count].num1 = 39, road[count].num2 = 40; road[count].crwDge = 0.2; road[count].bicycle = 1;   count++;
		road[count].num1 = 40, road[count].num2 = 41; road[count].crwDge = 0.5; road[count].bicycle = 1;   count++;
		road[count].num1 = 40, road[count].num2 = 42; road[count].crwDge = 0.4; road[count].bicycle = 0;   count++;
		road[count].num1 = 42, road[count].num2 = 43; road[count].crwDge = 0.6; road[count].bicycle = 0;   count++;
		road[count].num1 = 42, road[count].num2 = 44; road[count].crwDge = 0.3; road[count].bicycle = 0;   count++;
		road[count].num1 = 25, road[count].num2 = 30; road[count].crwDge = 0.1; road[count].bicycle = 0;   count++;
		road[count].num1 = 4, road[count].num2 = 5; road[count].crwDge = 0.3;   road[count].bicycle = 0;   count++;
		road[count].num1 = 6, road[count].num2 = 7; road[count].crwDge = 0.2;   road[count].bicycle = 0;   count++;
		road[count].num1 = 8, road[count].num2 = 13; road[count].crwDge = 0.2;  road[count].bicycle = 1;   count++;
		road[count].num1 = 1, road[count].num2 = 15; road[count].crwDge = 0.3;  road[count].bicycle = 1;   count++;
		road[count].num1 = 12, road[count].num2 = 16; road[count].crwDge = 0.4; road[count].bicycle = 1;   count++;
		road[count].num1 = 14, road[count].num2 = 37; road[count].crwDge = 0.6; road[count].bicycle = 1;   count++;
		road[count].num1 = 26, road[count].num2 = 31; road[count].crwDge = 0.5; road[count].bicycle = 0;   count++;
		road[count].num1 = 20, road[count].num2 = 43; road[count].crwDge = 0.1; road[count].bicycle = 0;   count++;
		road[count].num1 = 20, road[count].num2 = 41; road[count].crwDge = 0.5; road[count].bicycle = 1;   count++;
		road[count].num1 = 25, road[count].num2 = 41; road[count].crwDge = 0.4; road[count].bicycle = 0;   count++;
		road[count].num1 = 41, road[count].num2 = 45; road[count].crwDge = 0.7; road[count].bicycle = 0;   count++;
		road[count].num1 = 2, road[count].num2 = 16; road[count].crwDge = 0.6;  road[count].bicycle = 0;   count++;
		road[count].num1 = 20, road[count].num2 = 44; road[count].crwDge = 0.1; road[count].bicycle = 1;   count++;

		//北校区道路
		road[count].num1 = 49, road[count].num2 = 50; road[count].crwDge = 0.1; road[count].bicycle = 0;   count++;
		road[count].num1 = 49, road[count].num2 = 51; road[count].crwDge = 0.3;  road[count].bicycle = 0;  count++;
		road[count].num1 = 49, road[count].num2 = 52; road[count].crwDge = 0.5; road[count].bicycle = 1;   count++;
		road[count].num1 = 50, road[count].num2 = 83; road[count].crwDge = 1;   road[count].bicycle = 1;   count++;
		road[count].num1 = 50, road[count].num2 = 59; road[count].crwDge = 0.9; road[count].bicycle = 0;   count++;
		road[count].num1 = 50, road[count].num2 = 51; road[count].crwDge = 0.4; road[count].bicycle = 0;   count++;
		road[count].num1 = 59, road[count].num2 = 83; road[count].crwDge = 0.4; road[count].bicycle = 0;   count++;
		road[count].num1 = 51, road[count].num2 = 59; road[count].crwDge = 0.6; road[count].bicycle = 0;   count++;
		road[count].num1 = 51, road[count].num2 = 52; road[count].crwDge = 0.7; road[count].bicycle = 0;   count++;
		road[count].num1 = 52, road[count].num2 = 53; road[count].crwDge = 0.8; road[count].bicycle = 1;   count++;
		road[count].num1 = 53, road[count].num2 = 54; road[count].crwDge = 0.6; road[count].bicycle = 1;   count++;
		road[count].num1 = 53, road[count].num2 = 72; road[count].crwDge = 0.8; road[count].bicycle = 0;   count++;
		road[count].num1 = 54, road[count].num2 = 72; road[count].crwDge = 0.3; road[count].bicycle = 0;   count++;
		road[count].num1 = 51, road[count].num2 = 58; road[count].crwDge = 0.2; road[count].bicycle = 0;   count++;
		road[count].num1 = 59, road[count].num2 = 60; road[count].crwDge = 0.1; road[count].bicycle = 0;   count++;
		road[count].num1 = 82, road[count].num2 = 83; road[count].crwDge = 1;   road[count].bicycle = 0;   count++;
		road[count].num1 = 52, road[count].num2 = 58; road[count].crwDge = 0.6; road[count].bicycle = 1;   count++;
		road[count].num1 = 52, road[count].num2 = 55; road[count].crwDge = 0.1; road[count].bicycle = 1;   count++;
		road[count].num1 = 73, road[count].num2 = 83; road[count].crwDge = 0.5; road[count].bicycle = 1;   count++;
		road[count].num1 = 73, road[count].num2 = 82; road[count].crwDge = 0.7; road[count].bicycle = 0;   count++;
		road[count].num1 = 73, road[count].num2 = 81; road[count].crwDge = 0.9; road[count].bicycle = 1;   count++;
		road[count].num1 = 81, road[count].num2 = 82; road[count].crwDge = 0.1; road[count].bicycle = 0;   count++;
		road[count].num1 = 60, road[count].num2 = 82; road[count].crwDge = 0.3; road[count].bicycle = 0;   count++;
		road[count].num1 = 58, road[count].num2 = 60; road[count].crwDge = 0.5; road[count].bicycle = 0;   count++;
		road[count].num1 = 56, road[count].num2 = 58; road[count].crwDge = 0.4; road[count].bicycle = 0;   count++;
		road[count].num1 = 57, road[count].num2 = 58; road[count].crwDge = 0.9; road[count].bicycle = 0;   count++;
		road[count].num1 = 60, road[count].num2 = 61; road[count].crwDge = 0.2; road[count].bicycle = 0;   count++;
		road[count].num1 = 56, road[count].num2 = 57; road[count].crwDge = 0.3; road[count].bicycle = 0;   count++;
		road[count].num1 = 55, road[count].num2 = 56; road[count].crwDge = 0.4; road[count].bicycle = 0;   count++;
		road[count].num1 = 54, road[count].num2 = 55; road[count].crwDge = 0.6; road[count].bicycle = 1;   count++;
		road[count].num1 = 57, road[count].num2 = 61; road[count].crwDge = 0.2; road[count].bicycle = 0;   count++;
		road[count].num1 = 61, road[count].num2 = 81; road[count].crwDge = 0.8; road[count].bicycle = 0;   count++;
		road[count].num1 = 79, road[count].num2 = 81; road[count].crwDge = 0.2; road[count].bicycle = 0;   count++;
		road[count].num1 = 62, road[count].num2 = 81; road[count].crwDge = 0.3;  road[count].bicycle = 1;  count++;
		road[count].num1 = 61, road[count].num2 = 62; road[count].crwDge = 0.4;  road[count].bicycle = 0;  count++;
		road[count].num1 = 57, road[count].num2 = 63; road[count].crwDge = 0.6;  road[count].bicycle = 0;  count++;
		road[count].num1 = 56, road[count].num2 = 64; road[count].crwDge = 0.1;  road[count].bicycle = 1;  count++;
		road[count].num1 = 55, road[count].num2 = 65; road[count].crwDge = 0.1;  road[count].bicycle = 1;  count++;
		road[count].num1 = 54, road[count].num2 = 66; road[count].crwDge = 0.5; road[count].bicycle = 1;   count++;
		road[count].num1 = 80, road[count].num2 = 84; road[count].crwDge = 0.4; road[count].bicycle = 0;   count++;
		road[count].num1 = 79, road[count].num2 = 80; road[count].crwDge = 0.3; road[count].bicycle = 1;   count++;
		road[count].num1 = 62, road[count].num2 = 79; road[count].crwDge = 0.5; road[count].bicycle = 0;   count++;
		road[count].num1 = 62, road[count].num2 = 63; road[count].crwDge = 0.3;  road[count].bicycle = 0;  count++;
		road[count].num1 = 63, road[count].num2 = 64; road[count].crwDge = 0.4;  road[count].bicycle = 0;  count++;
		road[count].num1 = 64, road[count].num2 = 65; road[count].crwDge = 0.7;  road[count].bicycle = 0;  count++;
		road[count].num1 = 65, road[count].num2 = 66; road[count].crwDge = 0.3;  road[count].bicycle = 1;  count++;
		road[count].num1 = 77, road[count].num2 = 79; road[count].crwDge = 0.6;  road[count].bicycle = 0;  count++;
		road[count].num1 = 62, road[count].num2 = 77; road[count].crwDge = 0.8;  road[count].bicycle = 1;  count++;
		road[count].num1 = 62, road[count].num2 = 75; road[count].crwDge = 0.1;  road[count].bicycle = 0;  count++;
		road[count].num1 = 75, road[count].num2 = 77; road[count].crwDge = 0.9; road[count].bicycle = 0;   count++;
		road[count].num1 = 77, road[count].num2 = 78; road[count].crwDge = 1;   road[count].bicycle = 0; count++;
		road[count].num1 = 76, road[count].num2 = 77; road[count].crwDge = 0.6; road[count].bicycle = 0;   count++;
		road[count].num1 = 76, road[count].num2 = 78; road[count].crwDge = 0.4;  road[count].bicycle = 1;  count++;
		road[count].num1 = 74, road[count].num2 = 76; road[count].crwDge = 0.8;  road[count].bicycle = 0;  count++;
		road[count].num1 = 71, road[count].num2 = 74; road[count].crwDge = 0.3;  road[count].bicycle = 0;  count++;
		road[count].num1 = 70, road[count].num2 = 71; road[count].crwDge = 0.4;  road[count].bicycle = 0;  count++;
		road[count].num1 = 69, road[count].num2 = 70; road[count].crwDge = 0.6;  road[count].bicycle = 0;  count++;
		road[count].num1 = 68, road[count].num2 = 69; road[count].crwDge = 0.7;  road[count].bicycle = 1; count++;
		road[count].num1 = 74, road[count].num2 = 75; road[count].crwDge = 0.1; road[count].bicycle = 0;   count++;
		road[count].num1 = 63, road[count].num2 = 71; road[count].crwDge = 0.3;  road[count].bicycle = 1;  count++;
		road[count].num1 = 64, road[count].num2 = 70; road[count].crwDge = 0.6;  road[count].bicycle = 1;  count++;
		road[count].num1 = 65, road[count].num2 = 69; road[count].crwDge = 0.8;  road[count].bicycle = 1;  count++;
		road[count].num1 = 66, road[count].num2 = 67; road[count].crwDge = 0.7;  road[count].bicycle = 1;  count++;
		road[count].num1 = 66, road[count].num2 = 68; road[count].crwDge = 0.8;  road[count].bicycle = 1;  count++;

		//南校区道路
		road[count].num1 = 85, road[count].num2 = 86; road[count].crwDge = 0.6;  road[count].bicycle = 0;    count++;
		road[count].num1 = 86, road[count].num2 = 87; road[count].crwDge = 0.4;  road[count].bicycle = 0;    count++;
		road[count].num1 = 87, road[count].num2 = 89; road[count].crwDge = 0.7;  road[count].bicycle = 1;    count++;
		road[count].num1 = 87, road[count].num2 = 88; road[count].crwDge = 0.2;  road[count].bicycle = 1;    count++;
		road[count].num1 = 87, road[count].num2 = 92; road[count].crwDge = 0.3; road[count].bicycle = 0;     count++;
		road[count].num1 = 91, road[count].num2 = 92; road[count].crwDge = 0.9; road[count].bicycle = 0;     count++;
		road[count].num1 = 90, road[count].num2 = 91; road[count].crwDge = 0.4; road[count].bicycle = 0;     count++;
		road[count].num1 = 92, road[count].num2 = 93; road[count].crwDge = 1;   road[count].bicycle = 1;     count++;
		road[count].num1 = 93, road[count].num2 = 94; road[count].crwDge = 0.8;  road[count].bicycle = 1;    count++;
		road[count].num1 = 94, road[count].num2 = 95; road[count].crwDge = 0.2;  road[count].bicycle = 1;    count++;
		road[count].num1 = 95, road[count].num2 = 99; road[count].crwDge = 0.6;  road[count].bicycle = 0;    count++;
		road[count].num1 = 95, road[count].num2 = 105; road[count].crwDge = 0.3; road[count].bicycle = 1;    count++;
		road[count].num1 = 94, road[count].num2 = 99; road[count].crwDge = 0.8;  road[count].bicycle = 0;    count++;
		road[count].num1 = 94, road[count].num2 = 104; road[count].crwDge = 0.7; road[count].bicycle = 1;    count++;
		road[count].num1 = 99, road[count].num2 = 105; road[count].crwDge = 0.3; road[count].bicycle = 0;    count++;
		road[count].num1 = 99, road[count].num2 = 104; road[count].crwDge = 0.4;  road[count].bicycle = 0;   count++;
		road[count].num1 = 104, road[count].num2 = 105; road[count].crwDge = 0.2;  road[count].bicycle = 1;  count++;
		road[count].num1 = 94, road[count].num2 = 98; road[count].crwDge = 1;  road[count].bicycle = 0;      count++;
		road[count].num1 = 98, road[count].num2 = 104; road[count].crwDge = 0.9; road[count].bicycle = 0;    count++;
		road[count].num1 = 93, road[count].num2 = 98; road[count].crwDge = 0.4;  road[count].bicycle = 0;    count++;
		road[count].num1 = 98, road[count].num2 = 103; road[count].crwDge = 0.3; road[count].bicycle = 0;    count++;
		road[count].num1 = 103, road[count].num2 = 104; road[count].crwDge = 0.3; road[count].bicycle = 1;   count++;
		road[count].num1 = 93, road[count].num2 = 103; road[count].crwDge = 0.6;  road[count].bicycle = 1;   count++;
		road[count].num1 = 92, road[count].num2 = 103; road[count].crwDge = 0.8;  road[count].bicycle = 1;   count++;
		road[count].num1 = 92, road[count].num2 = 102; road[count].crwDge = 0.1;  road[count].bicycle = 1;   count++;
		road[count].num1 = 102, road[count].num2 = 103; road[count].crwDge = 0.6; road[count].bicycle = 1;   count++;
		road[count].num1 = 92, road[count].num2 = 97; road[count].crwDge = 0.2;  road[count].bicycle = 0;    count++;
		road[count].num1 = 97, road[count].num2 = 102; road[count].crwDge = 0.3; road[count].bicycle = 0;    count++;
		road[count].num1 = 91, road[count].num2 = 97; road[count].crwDge = 0.7;  road[count].bicycle = 0;    count++;
		road[count].num1 = 97, road[count].num2 = 101; road[count].crwDge = 0.6;  road[count].bicycle = 0;   count++;
		road[count].num1 = 101, road[count].num2 = 102; road[count].crwDge = 0.4; road[count].bicycle = 0;   count++;
		road[count].num1 = 91, road[count].num2 = 101; road[count].crwDge = 0.9;  road[count].bicycle = 0;   count++;
		road[count].num1 = 91, road[count].num2 = 96; road[count].crwDge = 0.7;  road[count].bicycle = 0;    count++;
		road[count].num1 = 96, road[count].num2 = 101; road[count].crwDge = 0.7; road[count].bicycle = 0;    count++;
		road[count].num1 = 90, road[count].num2 = 96; road[count].crwDge = 0.2;  road[count].bicycle = 0;    count++;
		road[count].num1 = 96, road[count].num2 = 100; road[count].crwDge = 0.4;  road[count].bicycle = 0;   count++;
		road[count].num1 = 100, road[count].num2 = 101; road[count].crwDge = 0.3; road[count].bicycle = 1;   count++;
		road[count].num1 = 90, road[count].num2 = 100; road[count].crwDge = 0.7;  road[count].bicycle = 1;   count++;
		road[count].num1 = 102, road[count].num2 = 109; road[count].crwDge = 0.9;  road[count].bicycle = 1;  count++;
		road[count].num1 = 109, road[count].num2 = 110; road[count].crwDge = 1;  road[count].bicycle = 1;    count++;
		road[count].num1 = 108, road[count].num2 = 109; road[count].crwDge = 0.4; road[count].bicycle = 0;   count++;
		road[count].num1 = 109, road[count].num2 = 117; road[count].crwDge = 0.6; road[count].bicycle = 0;   count++;
		road[count].num1 = 106, road[count].num2 = 110; road[count].crwDge = 0.7; road[count].bicycle = 0;   count++;
		road[count].num1 = 106, road[count].num2 = 112; road[count].crwDge = 0.3; road[count].bicycle = 1;   count++;
		road[count].num1 = 107, road[count].num2 = 108; road[count].crwDge = 0.1; road[count].bicycle = 1;   count++;
		road[count].num1 = 107, road[count].num2 = 111; road[count].crwDge = 0.1; road[count].bicycle = 1;   count++;
		road[count].num1 = 112, road[count].num2 = 121; road[count].crwDge = 0.9; road[count].bicycle = 1;   count++;
		road[count].num1 = 112, road[count].num2 = 120; road[count].crwDge = 0.7; road[count].bicycle = 0;   count++;
		road[count].num1 = 106, road[count].num2 = 119; road[count].crwDge = 0.6;  road[count].bicycle = 0;  count++;
		road[count].num1 = 110, road[count].num2 = 118; road[count].crwDge = 0.7;  road[count].bicycle = 0;  count++;
		road[count].num1 = 108, road[count].num2 = 116; road[count].crwDge = 0.8;  road[count].bicycle = 0;  count++;
		road[count].num1 = 107, road[count].num2 = 115; road[count].crwDge = 0.2;  road[count].bicycle = 0;  count++;
		road[count].num1 = 111, road[count].num2 = 114; road[count].crwDge = 0.1;  road[count].bicycle = 0;  count++;
		road[count].num1 = 111, road[count].num2 = 113; road[count].crwDge = 0.4;  road[count].bicycle = 1;  count++;
		road[count].num1 = 120, road[count].num2 = 121; road[count].crwDge = 0.1;  road[count].bicycle = 0;  count++;
		road[count].num1 = 119, road[count].num2 = 120; road[count].crwDge = 0.6;  road[count].bicycle = 1;  count++;
		road[count].num1 = 118, road[count].num2 = 119; road[count].crwDge = 0.5;  road[count].bicycle = 0;  count++;
		road[count].num1 = 117, road[count].num2 = 118; road[count].crwDge = 0.2;  road[count].bicycle = 1;  count++;
		road[count].num1 = 116, road[count].num2 = 117; road[count].crwDge = 0.8;  road[count].bicycle = 0;  count++;
		road[count].num1 = 115, road[count].num2 = 116; road[count].crwDge = 0.4;  road[count].bicycle = 1;  count++;
		road[count].num1 = 114, road[count].num2 = 115; road[count].crwDge = 0.5;  road[count].bicycle = 1;  count++;
		road[count].num1 = 113, road[count].num2 = 114; road[count].crwDge = 0.6;  road[count].bicycle = 0;  count++;
		road[count].num1 = 117, road[count].num2 = 122; road[count].crwDge = 1;   road[count].bicycle = 0;   count++;
		road[count].num1 = 122, road[count].num2 = 123; road[count].crwDge = 0.1;  road[count].bicycle = 0;  count++;
		road[count].num1 = 0, road[count].num2 = 49, road[count].distance = 10000, road[count].crwDge = 1; road[count].bicycle = 1; count++;//205
		road[count].num1 = 0, road[count].num2 = 67, road[count].distance = 11000, road[count].crwDge = 0.9; road[count].bicycle = 1; count++;
		road[count].num1 = 0, road[count].num2 = 84, road[count].distance = 12000, road[count].crwDge = 1; road[count].bicycle = 1; count++;
		road[count].num1 = 0, road[count].num2 = 85, road[count].distance = 13000, road[count].crwDge = 0.8; road[count].bicycle = 1; count++;
		road[count].num1 = 0, road[count].num2 = 123, road[count].distance = 9000, road[count].crwDge = 0.9; road[count].bicycle = 1; count++;
		road[count].num1 = 27, road[count].num2 = 49, road[count].distance = 14000, road[count].crwDge = 1; road[count].bicycle = 1; count++;
		road[count].num1 = 27, road[count].num2 = 67, road[count].distance = 8000, road[count].crwDge = 0.9; road[count].bicycle = 1; count++;
		road[count].num1 = 27, road[count].num2 = 84, road[count].distance = 15000, road[count].crwDge = 1; road[count].bicycle = 1; count++;
		road[count].num1 = 27, road[count].num2 = 85, road[count].distance = 11000, road[count].crwDge = 1; road[count].bicycle = 1; count++;
		road[count].num1 = 27, road[count].num2 = 123, road[count].distance = 16000, road[count].crwDge = 0.7; road[count].bicycle = 1; count++;
		road[count].num1 = 49, road[count].num2 = 85, road[count].distance = 15500, road[count].crwDge = 0.8; road[count].bicycle = 1; count++;
		road[count].num1 = 49, road[count].num2 = 123, road[count].distance = 10000, road[count].crwDge = 0.9; road[count].bicycle = 1; count++;
		road[count].num1 = 67, road[count].num2 = 85, road[count].distance = 8500, road[count].crwDge = 1; road[count].bicycle = 1; count++;
		road[count].num1 = 67, road[count].num2 = 123, road[count].distance = 9500, road[count].crwDge = 0.6; road[count].bicycle = 1; count++;
		road[count].num1 = 84, road[count].num2 = 85, road[count].distance = 11500, road[count].crwDge = 0.8; road[count].bicycle = 1; count++;
		road[count].num1 = 84, road[count].num2 = 123, road[count].distance = 12500, road[count].crwDge = 0.9; road[count].bicycle = 1;//220
	}

	//道路距离计算
	int roaddis;
	int d1, d2, d1x, d1y, d2x, d2y;
	int absx, absy;
	for (int i = 0; i < 204; i++) {
		d1 = road[i].num1, d2 = road[i].num2;
		d1x = build[d1].x, d1y = build[d1].y, d2x = build[d2].x, d2y = build[d2].y;
		if (d2y > d1y)
			absy = d2y - d1y;
		else
			absy = d1y - d2y;
		if (d2x > d1x)
			absx = d2x - d1x;
		else
			absx = d1x - d2x;
		roaddis = (int)sqrt(absx * absx + absy * absy);
		road[i].distance = roaddis;
	}

	int isBreak = 0;

	while (1) {
		if (MouseHit()) {
			MOUSEMSG msg = GetMouseMsg();
			if (msg.uMsg == WM_LBUTTONDOWN) {
				switch (whichPicture) {
				case 0://此时为主页面
					if (msg.x > 420 && msg.x < 510 && msg.y > 200 && msg.y < 230 && startPosition == -1) {//起始位置西校区
						loadimage(NULL, _T("west.jpg"));
						while (1) {
							if (MouseHit()) {
								MOUSEMSG msgstart = GetMouseMsg();
								for (int i = 0; i < NORTHMIN; i++) {
									if (msgstart.x >= build[i].x - lineThick && msgstart.x <= build[i].x + lineThick && msgstart.y >= build[i].y - lineThick && msgstart.y <= build[i].y + lineThick && msgstart.mkLButton) {
										startPosition = build[i].num;
										departure = startPosition;
										isBreak = 1;
										printf("起始位置为:%d——%s\n", departure, build[departure].logclLoc);
										if (isBreak) break;
									}
								}
								if (isBreak) {
									isBreak = 0;
									break;
								}
							}
						}
						cleardevice();
						loadimage(NULL, _T("homepage.jpg"));
					}

					if (msg.x > 550 && msg.x < 640 && msg.y > 200 && msg.y < 230 && startPosition == -1) {//起始位置北校区 
						loadimage(NULL, _T("north.jpg"));
						while (1) {
							if (MouseHit()) {
								MOUSEMSG msgstart = GetMouseMsg();
								for (int i = NORTHMIN; i < SOUTHMIN; i++) {
									if (msgstart.x >= build[i].x - lineThick && msgstart.x <= build[i].x + lineThick && msgstart.y >= build[i].y - lineThick && msgstart.y <= build[i].y + lineThick && msgstart.mkLButton) {
										startPosition = build[i].num;
										departure = startPosition;
										isBreak = 1;
										printf("起始位置为:%d——%s\n", departure, build[departure].logclLoc);
										if (isBreak) break;
									}
								}
								if (isBreak) {
									isBreak = 0;
									break;
								}
							}
						}
						cleardevice();
						loadimage(NULL, _T("homepage.jpg"));
					}

					if (msg.x > 680 && msg.x < 770 && msg.y > 200 && msg.y < 230 && startPosition == -1) {//起始位置南校区
						loadimage(NULL, _T("south.jpg"));
						while (1) {
							if (MouseHit()) {
								MOUSEMSG msgstart = GetMouseMsg();
								for (int i = SOUTHMIN; i < SOUTHMAX + 1; i++) {
									if (msgstart.x >= build[i].x - lineThick && msgstart.x <= build[i].x + lineThick && msgstart.y >= build[i].y - lineThick && msgstart.y <= build[i].y + lineThick && msgstart.mkLButton) {
										startPosition = build[i].num;
										departure = startPosition;
										isBreak = 1;
										printf("起始位置为:%d——%s\n", departure, build[departure].logclLoc);
										if (isBreak) break;
									}
								}
								if (isBreak) {
									isBreak = 0;
									break;
								}
							}
						}
						cleardevice();
						loadimage(NULL, _T("homepage.jpg"));
					}

					if (msg.x > 420 && msg.x < 510 && msg.y > 300 && msg.y < 330 && terminalPosition == -1) {//终点位置西校区
						loadimage(NULL, _T("west.jpg"));
						while (1) {
							if (MouseHit()) {
								MOUSEMSG msgterminal = GetMouseMsg();
								for (int i = 0; i < NORTHMIN; i++) {
									if (msgterminal.x >= build[i].x - lineThick && msgterminal.x <= build[i].x + lineThick && msgterminal.y >= build[i].y - lineThick && msgterminal.y <= build[i].y + lineThick && msgterminal.mkLButton) {
										terminalPosition = build[i].num;
										destination = terminalPosition;
										isBreak = 1;
										printf("终点位置为:%d——%s\n", destination, build[destination].logclLoc);
										if (isBreak) break;
									}
								}
								if (isBreak) {
									isBreak = 0;
									break;
								}
							}
						}
						cleardevice();
						loadimage(NULL, _T("homepage.jpg"));
					}

					if (msg.x > 550 && msg.x < 640 && msg.y > 300 && msg.y < 330 && terminalPosition == -1) {//终点位置北校区
						loadimage(NULL, _T("north.jpg"));
						while (1) {
							if (MouseHit()) {
								MOUSEMSG msgterminal = GetMouseMsg();
								for (int i = NORTHMIN; i < SOUTHMIN; i++) {
									if (msgterminal.x >= build[i].x - lineThick && msgterminal.x <= build[i].x + lineThick && msgterminal.y >= build[i].y - lineThick && msgterminal.y <= build[i].y + lineThick && msgterminal.mkLButton) {
										terminalPosition = build[i].num;
										destination = terminalPosition;
										isBreak = 1;
										printf("终点位置为:%d——%s\n", destination, build[destination].logclLoc);
										if (isBreak) break;
									}
								}
								if (isBreak) {
									isBreak = 0;
									break;
								}
							}
						}
						cleardevice();
						loadimage(NULL, _T("homepage.jpg"));
					}

					if (msg.x > 680 && msg.x < 770 && msg.y > 300 && msg.y < 330 && terminalPosition == -1) {//终点位置南校区
						loadimage(NULL, _T("south.jpg"));
						while (1) {
							if (MouseHit()) {
								MOUSEMSG msgterminal = GetMouseMsg();
								for (int i = SOUTHMIN; i < SOUTHMAX + 1; i++) {
									if (msgterminal.x >= build[i].x - lineThick && msgterminal.x <= build[i].x + lineThick && msgterminal.y >= build[i].y - lineThick && msgterminal.y <= build[i].y + lineThick && msgterminal.mkLButton) {
										terminalPosition = build[i].num;
										destination = terminalPosition;
										isBreak = 1;
										printf("终点位置为:%d——%s\n", destination, build[destination].logclLoc);
										if (isBreak) break;
									}
								}
								if (isBreak) {
									isBreak = 0;
									break;
								}
							}
						}
						cleardevice();
						loadimage(NULL, _T("homepage.jpg"));
					}

					if (msg.x > 100 && msg.x < 460 && msg.y > 400 && msg.y < 430 && navigationStrategy == -1) {//导航策略
						wchar_t s[5];
						InputBox(s, 5, L"请输入导航策略编号", L"导航策略选择", L"1", 250, 0, false);
						// 将用户输入转换为数字
						navigationStrategy = _wtoi(s);
						while (navigationStrategy <= 0 || navigationStrategy >= 4) {
							printf("\n导航策略编号输入有误，请重新输入！\n");
							InputBox(s, 5, L"请输入导航策略编号", L"导航策略选择", L"1", 250, 0, false);
							navigationStrategy = _wtoi(s);
						}

						if (navigationStrategy == 3) {//途经点
							printf("\n请点击地图以选择途经点\n");
							isBreak = 0;
							while (1) {
								if (MouseHit()) {
									MOUSEMSG msgmid = GetMouseMsg();
									if (msgmid.x >= 350 && msgmid.x <= 500 && msgmid.y >= 680 && msgmid.y <= 710 && msgmid.mkLButton) {//西
										cleardevice();
										loadimage(NULL, _T("west.jpg"));
										while (1) {
											if (MouseHit()) {
												MOUSEMSG msgmidwest = GetMouseMsg();
												for (int i = 0; i < NORTHMIN; i++) {
													if (msgmidwest.x >= build[i].x - lineThick && msgmidwest.x <= build[i].x + lineThick && msgmidwest.y >= build[i].y - lineThick && msgmidwest.y <= build[i].y + lineThick && msgmidwest.mkLButton) {
														mid = build[i].num;
														isBreak = 1;
														printf("\n途经点编号为:%d\n", mid);
														if (isBreak) break;
													}
												}
												if (isBreak) break;
											}
										}
										cleardevice();
										loadimage(NULL, _T("homepage.jpg"));
									}
									if (isBreak) break;
									if (msgmid.x >= 600 && msgmid.x <= 750 && msgmid.y >= 680 && msgmid.y <= 710 && msgmid.mkLButton) {//北
										cleardevice();
										loadimage(NULL, _T("north.jpg"));
										while (1) {
											if (MouseHit()) {
												MOUSEMSG midnorth = GetMouseMsg();
												for (int i = NORTHMIN; i < SOUTHMIN; i++) {
													if (midnorth.x >= build[i].x - lineThick && midnorth.x <= build[i].x + lineThick && midnorth.y >= build[i].y - lineThick && midnorth.y <= build[i].y + lineThick && midnorth.mkLButton) {
														mid = build[i].num;
														isBreak = 1;
														printf("\n途经点编号为:%d\n", mid);
														if (isBreak) break;
													}
												}
												if (isBreak) break;
											}
										}
										cleardevice();
										loadimage(NULL, _T("homepage.jpg"));
									}
									if (isBreak) break;
									if (msgmid.x >= 850 && msgmid.x <= 1000 && msgmid.y >= 680 && msgmid.y <= 710 && msgmid.mkLButton) {//南
										cleardevice();
										loadimage(NULL, _T("south.jpg"));
										while (1) {
											if (MouseHit()) {
												MOUSEMSG midsouth = GetMouseMsg();
												for (int i = SOUTHMIN; i < SOUTHMAX + 1; i++) {
													if (midsouth.x >= build[i].x - lineThick && midsouth.x <= build[i].x + lineThick && midsouth.y >= build[i].y - lineThick && midsouth.y <= build[i].y + lineThick && midsouth.mkLButton) {
														mid = build[i].num;
														isBreak = 1;
														printf("\n途经点编号为:%d\n", mid);
														if (isBreak) break;
													}
												}
												if (isBreak) break;
											}
										}
										cleardevice();
										loadimage(NULL, _T("homepage.jpg"));
									}
									if (isBreak) break;
								}
							}
						}
					}

					if (msg.x > 350 && msg.x < 500 && msg.y > 680 && msg.y < 710) {//西校区地图查看
						cleardevice();
						loadimage(NULL, _T("west.jpg"));
						setfillcolor(RGB(162, 209, 106));
						settextcolor(RGB(19, 21, 19));
						settextstyle(30, 0, _T("仿宋"));
						solidrectangle(1280, 700, 1340, 730);
						outtextxy(1280, 700, L"返回");
						while (1) {
							if (MouseHit()) {
								MOUSEMSG msgreturn = GetMouseMsg();
								if (msgreturn.x >= 1280 && msgreturn.x <= 1340 && msgreturn.y >= 700 && msgreturn.y <= 730 && msgreturn.mkLButton) {
									cleardevice();
									loadimage(NULL, _T("homepage.jpg"));
									break;
								}
							}
						}
					}

					if (msg.x > 600 && msg.x < 750 && msg.y > 680 && msg.y < 710) {//北校区地图查看
						cleardevice();
						loadimage(NULL, _T("north.jpg"));
						setfillcolor(RGB(162, 209, 106));
						settextcolor(RGB(19, 21, 19));
						settextstyle(30, 0, _T("仿宋"));
						solidrectangle(1280, 700, 1340, 730);
						outtextxy(1280, 700, L"返回");
						while (1) {
							if (MouseHit()) {
								MOUSEMSG msgreturn = GetMouseMsg();
								if (msgreturn.x >= 1280 && msgreturn.x <= 1340 && msgreturn.y >= 700 && msgreturn.y <= 730 && msgreturn.mkLButton) {
									cleardevice();
									loadimage(NULL, _T("homepage.jpg"));
									break;
								}
							}
						}
					}

					if (msg.x > 850 && msg.x < 1000 && msg.y > 680 && msg.y < 710) {//南校区地图查看
						cleardevice();
						loadimage(NULL, _T("south.jpg"));
						setfillcolor(RGB(162, 209, 106));
						settextcolor(RGB(19, 21, 19));
						settextstyle(30, 0, _T("仿宋"));
						solidrectangle(1280, 700, 1340, 730);
						outtextxy(1280, 700, L"返回");
						while (1) {
							if (MouseHit()) {
								MOUSEMSG msgreturn = GetMouseMsg();
								if (msgreturn.x >= 1280 && msgreturn.x <= 1340 && msgreturn.y >= 700 && msgreturn.y <= 730 && msgreturn.mkLButton) {
									cleardevice();
									loadimage(NULL, _T("homepage.jpg"));
									break;
								}
							}
						}
					}

					if (msg.mkLButton && msg.x > 640 && msg.x < 720 && msg.y>590 && msg.y < 620) {//重置按钮
						startPosition = -1;
						terminalPosition = -1;
						navigationStrategy = -1;
						printf("\n已重置\n");
					}

					if (msg.mkLButton && startPosition == -1 && msg.x > 100 && msg.x < 400 && msg.y>200 && msg.y < 230) {//逻辑起始位置
						loadimage(NULL, _T("logicposition.jpg"));
						setfillcolor(RGB(162, 209, 106));
						settextcolor(RGB(19, 21, 19));
						settextstyle(30, 0, _T("仿宋"));
						solidrectangle(1280, 700, 1340, 730);
						outtextxy(1280, 700, L"输入");
						while (1) {
							if (MouseHit()) {
								MOUSEMSG msgreturn = GetMouseMsg();
								if (msgreturn.x >= 1280 && msgreturn.x <= 1340 && msgreturn.y >= 700 && msgreturn.y <= 730 && msgreturn.mkLButton) {
									wchar_t s[5];
									InputBox(s, 5, L"请输入逻辑起始位置编号", L"起始位置选择", L"1", 250, 0, false);
									// 将用户输入转换为数字
									startPosition = _wtoi(s);
									while (startPosition < 0 || startPosition >= 124) {
										printf("\n逻辑起始位置输入有误，请重新输入!\n");
										InputBox(s, 5, L"请输入逻辑起始位置编号", L"起始位置选择", L"1", 250, 0, false);
										startPosition = _wtoi(s);
									}
									departure = startPosition;
									printf("起始位置为:%d——%s\n", departure, build[departure].logclLoc);
									cleardevice();
									loadimage(NULL, _T("homepage.jpg"));
									break;
								}
							}
						}
					}

					if (msg.mkLButton && terminalPosition == -1 && msg.x > 100 && msg.x < 400 && msg.y>300 && msg.y < 330) {//逻辑终点位置
						loadimage(NULL, _T("logicposition.jpg"));
						setfillcolor(RGB(162, 209, 106));
						settextcolor(RGB(19, 21, 19));
						settextstyle(30, 0, _T("仿宋"));
						solidrectangle(1280, 700, 1340, 730);
						outtextxy(1280, 700, L"输入");
						while (1) {
							if (MouseHit()) {
								MOUSEMSG msgreturn = GetMouseMsg();
								if (msgreturn.x >= 1280 && msgreturn.x <= 1340 && msgreturn.y >= 700 && msgreturn.y <= 730 && msgreturn.mkLButton) {
									wchar_t t[5];
									InputBox(t, 5, L"请输入逻辑终点位置编号", L"终点位置选择", L"1", 250, 0, false);
									// 将用户输入转换为数字
									terminalPosition = _wtoi(t);
									while (terminalPosition < 0 || terminalPosition >= 124) {
										printf("\n逻辑终点位置输入有误，请重新输入!\n");
										InputBox(t, 5, L"请输入逻辑终点位置编号", L"终点位置选择", L"1", 250, 0, false);
										terminalPosition = _wtoi(t);
									}
									destination = terminalPosition;
									printf("终点位置为:%d——%s\n", destination, build[destination].logclLoc);
									cleardevice();
									loadimage(NULL, _T("homepage.jpg"));
									break;

								}
							}
						}
					}

					if (msg.x > 350 && msg.x < 500 && msg.y > 590 && msg.y < 620 && msg.mkLButton && startPosition == -1 && terminalPosition != -1 && navigationStrategy != -1) {
						printf("起始位置尚未确定,请输入\n");
					}

					if (msg.x > 350 && msg.x < 500 && msg.y > 590 && msg.y < 620 && msg.mkLButton && startPosition != -1 && terminalPosition == -1 && navigationStrategy != -1) {
						printf("终点位置尚未确定,请输入\n");
					}

					if (msg.x > 350 && msg.x < 500 && msg.y > 590 && msg.y < 620 && msg.mkLButton && startPosition != -1 && terminalPosition != -1 && navigationStrategy == -1) {
						printf("导航策略尚未确定,请输入\n");
					}

					if (msg.x > 350 && msg.x < 500 && msg.y > 590 && msg.y < 620 && msg.mkLButton && startPosition == -1 && terminalPosition == -1 && navigationStrategy != -1) {
						printf("起始位置及终点位置尚未确定,请输入\n");
					}

					if (msg.x > 350 && msg.x < 500 && msg.y > 590 && msg.y < 620 && msg.mkLButton && startPosition == -1 && terminalPosition != -1 && navigationStrategy == -1) {
						printf("起始位置及导航策略尚未确定,请输入\n");
					}

					if (msg.x > 350 && msg.x < 500 && msg.y > 590 && msg.y < 620 && msg.mkLButton && startPosition != -1 && terminalPosition == -1 && navigationStrategy == -1) {
						printf("终点位置及导航策略尚未确定,请输入\n");
					}

					if (msg.x > 350 && msg.x < 500 && msg.y > 590 && msg.y < 620 && msg.mkLButton && startPosition == -1 && terminalPosition == -1 && navigationStrategy == -1) {
						printf("起始位置、终点位置及导航策略尚未确定,请输入\n");
					}

					if (msg.x > 350 && msg.x < 500 && msg.y > 590 && msg.y < 620 && msg.mkLButton && startPosition != -1 && terminalPosition != -1 && navigationStrategy != -1) {//开始导航
						//测试用
						printf("\n起始：%d 终止：%d 策略：%d\n", departure, destination, navigationStrategy);
						switch (navigationStrategy) {
						case 1:
							shortlongth(departure);
							get_roads(departure, destination);
							printf("结点总数为 ：%d\n", buildBy);
							break;

						case 2:
							short_time(departure);
							get_roads(departure, destination);
							break;

						case 3:
							va_shortlongth(departure, destination, mid);
							printf("结点总数为 ：%d\n", Buildby);
							break;

						case 4:
							tran_short(departure);
							get_roads(departure, destination);
							break;

						default:
							printf("Error!\n");
							break;
						}

						changeBreak = 0;
						navigationInProgress = 1;//标记正在导航
						int i, k;//k为斜率
						int x1, x2, y1, y2;
						int tempx, tempy;
						int data1, data2;
						int count;

						int largest = 0;

						if (navigationStrategy == 3)
							largest = Buildby;
						else
							largest = buildBy;

						//写入日志文件
						int err;
						if ((err = fopen_s(&fp, FileName, "a")) != NULL) {
							printf("\nopen file error");
							getchar();
							exit(1);
						}
						int sroundminute;
						char plogic[20];
						char things[100];
						GetLocalTime(&time);

						BeginBatchDraw();

						for (i = 0; i < largest - 1; i++) {
							if (Roads[i] < 49 && Roads[i + 1] > 48 && Roads[i + 1] < 85) {//西北
								srand((unsigned)road[Roads[i], Roads[i + 1]].distance);
								sroundminute = rand() % 60;
								time.wHour++;
								time.wMinute += sroundminute;
								{
									if (time.wSecond >= 60) {
										time.wSecond = 0;
										time.wMinute++;
									}
									if (time.wMinute >= 60) {
										time.wMinute = 0;
										time.wHour++;
									}
									if (time.wHour >= 24) {
										time.wHour = 0;
										time.wDay++;
									}
									if (time.wMonth % 2 == 0) {
										if (time.wDay >= 30) {
											time.wDay = 0;
											time.wMonth++;
										}
									}
									else {
										if (time.wDay >= 31) {
											time.wDay = 0;
											time.wMonth++;
										}
									}
									if (time.wMonth >= 12) {
										time.wMonth = 0;
										time.wYear++;
									}
								}
								strcpy_s(plogic, 20, build[Roads[i + 1]].logclLoc);
								sprintf_s(things, 50, "%2d年%2d月%2d日: %2d:%2d到达：%s\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, plogic);
								printf("%s \n", things);
								writeFile(fp, things, 1);
								cleardevice();
								loadimage(NULL, _T("north.jpg"));
								continue;
							}
							if (Roads[i] < 49 && Roads[i + 1] > 84) {//西南
								srand((unsigned)road[Roads[i], Roads[i + 1]].distance);
								sroundminute = rand() % 60;
								time.wHour++;
								time.wMinute += sroundminute;
								{
									if (time.wSecond >= 60) {
										time.wSecond = 0;
										time.wMinute++;
									}
									if (time.wMinute >= 60) {
										time.wMinute = 0;
										time.wHour++;
									}
									if (time.wHour >= 24) {
										time.wHour = 0;
										time.wDay++;
									}
									if (time.wMonth % 2 == 0) {
										if (time.wDay >= 30) {
											time.wDay = 0;
											time.wMonth++;
										}
									}
									else {
										if (time.wDay >= 31) {
											time.wDay = 0;
											time.wMonth++;
										}
									}
									if (time.wMonth >= 12) {
										time.wMonth = 0;
										time.wYear++;
									}
								}
								strcpy_s(plogic, 20, build[Roads[i + 1]].logclLoc);
								sprintf_s(things, 50, "%2d年%2d月%2d日: %2d:%2d到达：%s\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, plogic);
								printf("%s \n", things);
								writeFile(fp, things, 1);
								cleardevice();
								loadimage(NULL, _T("south.jpg"));
								continue;
							}
							if (Roads[i] > 48 && Roads[i] < 85 && Roads[i + 1] < 49) {//北西
								srand((unsigned)road[Roads[i], Roads[i + 1]].distance);
								sroundminute = rand() % 60;
								time.wHour++;
								time.wMinute += sroundminute;
								{
									if (time.wSecond >= 60) {
										time.wSecond = 0;
										time.wMinute++;
									}
									if (time.wMinute >= 60) {
										time.wMinute = 0;
										time.wHour++;
									}
									if (time.wHour >= 24) {
										time.wHour = 0;
										time.wDay++;
									}
									if (time.wMonth % 2 == 0) {
										if (time.wDay >= 30) {
											time.wDay = 0;
											time.wMonth++;
										}
									}
									else {
										if (time.wDay >= 31) {
											time.wDay = 0;
											time.wMonth++;
										}
									}
									if (time.wMonth >= 12) {
										time.wMonth = 0;
										time.wYear++;
									}
								}
								strcpy_s(plogic, 20, build[Roads[i + 1]].logclLoc);
								sprintf_s(things, 50, "%2d年%2d月%2d日: %2d:%2d到达：%s\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, plogic);
								printf("%s \n", things);
								writeFile(fp, things, 1);
								cleardevice();
								loadimage(NULL, _T("west.jpg"));
								continue;
							}
							if (Roads[i] > 48 && Roads[i] < 85 && Roads[i + 1]>84) {//北南
								srand((unsigned)road[Roads[i], Roads[i + 1]].distance);
								sroundminute = rand() % 60;
								time.wHour++;
								time.wMinute += sroundminute;
								{
									if (time.wSecond >= 60) {
										time.wSecond = 0;
										time.wMinute++;
									}
									if (time.wMinute >= 60) {
										time.wMinute = 0;
										time.wHour++;
									}
									if (time.wHour >= 24) {
										time.wHour = 0;
										time.wDay++;
									}
									if (time.wMonth % 2 == 0) {
										if (time.wDay >= 30) {
											time.wDay = 0;
											time.wMonth++;
										}
									}
									else {
										if (time.wDay >= 31) {
											time.wDay = 0;
											time.wMonth++;
										}
									}
									if (time.wMonth >= 12) {
										time.wMonth = 0;
										time.wYear++;
									}
								}
								strcpy_s(plogic, 20, build[Roads[i + 1]].logclLoc);
								sprintf_s(things, 50, "%2d年%2d月%2d日: %2d:%2d到达：%s\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, plogic);
								printf("%s \n", things);
								writeFile(fp, things, 1);
								cleardevice();
								loadimage(NULL, _T("south.jpg"));
								continue;
							}
							if ((Roads[i] > 84) && Roads[i + 1] < 49) {//南西
								srand((unsigned)road[Roads[i], Roads[i + 1]].distance);
								sroundminute = rand() % 60;
								time.wHour++;
								time.wMinute += sroundminute;
								{
									if (time.wSecond >= 60) {
										time.wSecond = 0;
										time.wMinute++;
									}
									if (time.wMinute >= 60) {
										time.wMinute = 0;
										time.wHour++;
									}
									if (time.wHour >= 24) {
										time.wHour = 0;
										time.wDay++;
									}
									if (time.wMonth % 2 == 0) {
										if (time.wDay >= 30) {
											time.wDay = 0;
											time.wMonth++;
										}
									}
									else {
										if (time.wDay >= 31) {
											time.wDay = 0;
											time.wMonth++;
										}
									}
									if (time.wMonth >= 12) {
										time.wMonth = 0;
										time.wYear++;
									}
								}
								strcpy_s(plogic, 20, build[Roads[i + 1]].logclLoc);
								sprintf_s(things, 50, "%2d年%2d月%2d日: %2d:%2d到达：%s\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, plogic);
								printf("%s \n", things);
								writeFile(fp, things, 1);
								cleardevice();
								loadimage(NULL, _T("west.jpg"));
								continue;
							}
							if (Roads[i] > 84 && Roads[i + 1] < 85 && Roads[i + 1] > 48) {//南北
								srand((unsigned)road[Roads[i], Roads[i + 1]].distance);
								sroundminute = rand() % 60;
								time.wHour++;
								time.wMinute += sroundminute;
								{
									if (time.wSecond >= 60) {
										time.wSecond = 0;
										time.wMinute++;
									}
									if (time.wMinute >= 60) {
										time.wMinute = 0;
										time.wHour++;
									}
									if (time.wHour >= 24) {
										time.wHour = 0;
										time.wDay++;
									}
									if (time.wMonth % 2 == 0) {
										if (time.wDay >= 30) {
											time.wDay = 0;
											time.wMonth++;
										}
									}
									else {
										if (time.wDay >= 31) {
											time.wDay = 0;
											time.wMonth++;
										}
									}
									if (time.wMonth >= 12) {
										time.wMonth = 0;
										time.wYear++;
									}
								}
								strcpy_s(plogic, 20, build[Roads[i + 1]].logclLoc);
								sprintf_s(things, 50, "%2d年%2d月%2d日: %2d:%2d到达：%s\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, plogic);
								printf("%s \n", things);
								writeFile(fp, things, 1);
								cleardevice();
								loadimage(NULL, _T("north.jpg"));
								continue;
							}
							//接下来就在同一幅图中画图
							if (Roads[i] < 49) {
								loadimage(NULL, _T("west.jpg"));
								whichPicture = 1;
							}
							else if (Roads[i] > 48 && Roads[i] < 85) {
								loadimage(NULL, _T("north.jpg"));
								whichPicture = 2;
							}
							else {
								loadimage(NULL, _T("south.jpg"));
								whichPicture = 3;
							}
							data1 = Roads[i], data2 = Roads[i + 1];
							x1 = build[data1].x, y1 = build[data1].y;
							x2 = build[data2].x, y2 = build[data2].y;
							k = 1;
							if (x1 != x2)
								k = (y2 - y1) / (x2 - x1);
							k = abs(k);
							for (int j = 0; j < 204; j++) {//速度计算
								if (data1 > data2) {
									if (road[j].num1 == data2 && road[j].num2 == data1) {
										if (road[j].bicycle)
											speed = road[j].crwDge * 20;
										else
											speed = road[j].crwDge * 20 + 5;

									}
								}
								else if (data1 < data2) {
									if (road[j].num1 == data1 && road[j].num2 == data2) {
										if (road[j].bicycle)
											speed = road[j].crwDge * 20;
										else
											speed = road[j].crwDge * 20 + 5;
									}
								}
							}
							int flag = 0;
							while (x1 != x2 || y1 != y2) {
								time.wSecond++;
								if (time.wSecond >= 60) {
									time.wSecond = 0;
									time.wMinute++;
								}
								if (time.wMinute >= 60) {
									time.wMinute = 0;
									time.wHour++;
								}
								if (time.wHour >= 24) {
									time.wHour = 0;
									time.wDay++;
								}
								if (time.wMonth % 2 == 0) {
									if (time.wDay >= 30) {
										time.wDay = 0;
										time.wMonth++;
									}
								}
								else {
									if (time.wDay >= 31) {
										time.wDay = 0;
										time.wMonth++;
									}
								}
								if (time.wMonth >= 12) {
									time.wMonth = 0;
									time.wYear++;
								}
								setfillcolor(RGB(162, 209, 106));
								settextcolor(RGB(19, 21, 19));
								settextstyle(30, 0, _T("仿宋"));
								solidrectangle(1280, 700, 1340, 730);
								outtextxy(1280, 700, L"更改");
								solidrectangle(1180, 700, 1240, 730);
								outtextxy(1180, 700, L"查询");
								if (MouseHit()) {
									MOUSEMSG msgchange = GetMouseMsg();
									if (msgchange.x >= 1280 && msgchange.x <= 1340 && msgchange.y >= 700 && msgchange.y <= 730 && msgchange.mkLButton) {//更改
										EndBatchDraw();
										whichPicture = 0;
										navigationInProgress = 0;
										nearrstbdg = theNearestBuilding(x1, y1, build[data1].x, build[data1].y, build[data2].x, build[data2].y);
										if (nearrstbdg == 1)	startPosition = Roads[i];
										else startPosition = Roads[i + 1];
										departure = startPosition;
										terminalPosition = -1, navigationStrategy = -1;
										cleardevice();
										loadimage(NULL, _T("homepage.jpg"));
										changeBreak = 1;
										sprintf_s(things, 50, "%2d年%2d月%2d日: %2d:%2d 提出了“更改”请求\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
										writeFile(fp, things, 1);
										break;
									}
									if (msgchange.x >= 1180 && msgchange.x <= 1240 && msgchange.y >= 700 && msgchange.y <= 730 && msgchange.mkLButton) {//查询
										int searchnum;
										//nearrstbdg = theNearestBuilding(x1, y1, build[data1].x, build[data1].y, build[data2].x, build[data2].y);
										//if (nearrstbdg == 1)	queryPoint = Roads[i];
										//else queryPoint = Roads[i + 1];
										queryPoint = Roads[i + 1];
										//调用查找算法
										printf("\n待查询节点为：%s", build[queryPoint].logclLoc);
										searchnum = search(queryPoint);
										setfillcolor(YELLOW);//改变颜色
										printf("\n周围建筑总数为：%d\n", searchnum);
										TCHAR dds[5];
										for (int i = 0; i < searchnum; i++) {
											printf("到%s的距离是 %d \n", build[roundb[i]].logclLoc, D[roundb[i]]);
											_stprintf_s(dds, 5, _T("%d"), D[roundb[i]]);
											settextstyle(25, 0, _T("楷书"));
											settextcolor(BLACK);
											outtextxy(build[roundb[i]].x, build[roundb[i]].y, dds);
										}

										sprintf_s(things, 50, "%2d年%2d月%2d日: %2d:%2d 提出了“查询”请求\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
										writeFile(fp, things, 1);
										setfillcolor(RGB(235, 123, 123));//红色，建筑物节点颜色
										system("pause");
									}
								}

								if (flag) {//从第二个像素点开始绘制上一个圆，使其与道路颜色完全相同，实现行进动画效果
									setlinecolor(RGB(142, 166, 165));//灰色，道路颜色
									FlushBatchDraw();
									Sleep(speed);
									//此处增加一个分支，分为建筑物节点和普通道路
									int paintbuild = 0, temp;
									if (whichPicture == 1) {
										for (temp = 0; temp < 49; temp++) {//判断该点是否为建筑物节点
											if ((tempx == build[temp].x) && (tempy == build[temp].y)) {
												paintbuild = 1; break;
											}
										}
										if (paintbuild) {
											setfillcolor(RGB(235, 123, 123));//红色，建筑物节点颜色
											solidcircle(tempx, tempy, lineThick);
											paintbuild = 0;
										}
										else {
											setfillcolor(RGB(142, 166, 165));//灰色，道路颜色
											solidcircle(tempx, tempy, lineThick / 2);//线宽度的一半为圆的半径 ，所以用lineThick/2.
										}
									}
									else if (whichPicture == 2) {
										for (temp = 49; temp < 85; temp++) {//判断该点是否为建筑物节点
											if ((tempx == build[temp].x) && (tempy == build[temp].y)) {
												paintbuild = 1; break;
											}
										}
										if (paintbuild) {
											setfillcolor(RGB(235, 123, 123));//红色，建筑物节点颜色
											solidcircle(tempx, tempy, lineThick);
											paintbuild = 0;
										}
										else {
											setfillcolor(RGB(142, 166, 165));//灰色，道路颜色
											solidcircle(tempx, tempy, lineThick / 2);//线宽度的一半为圆的半径 ，所以用lineThick/2.
										}
									}
									else if (whichPicture == 3) {
										for (temp = 85; temp < 124; temp++) {//判断该点是否为建筑物节点
											if ((tempx == build[temp].x) && (tempy == build[temp].y)) {
												paintbuild = 1; break;
											}
										}
										if (paintbuild) {
											setfillcolor(RGB(235, 123, 123));//红色，建筑物节点颜色
											solidcircle(tempx, tempy, lineThick);
											paintbuild = 0;
										}
										else {
											setfillcolor(RGB(142, 166, 165));//灰色，道路颜色
											solidcircle(tempx, tempy, lineThick / 2);//线宽度的一半为圆的半径 ，所以用lineThick/2.
										}
									}
								}
								setfillcolor(RGB(235, 123, 123));//红色，建筑物节点颜色
								//此处修补被破坏的建筑物节点
								if (whichPicture == 1) {
									for (count = 0; count < 49; count++)
										solidcircle(build[count].x, build[count].y, lineThick);
								}
								else if (whichPicture == 2) {
									for (count = 49; count < 85; count++)
										solidcircle(build[count].x, build[count].y, lineThick);
								}
								else if (whichPicture == 3) {
									for (count = 85; count < 124; count++)
										solidcircle(build[count].x, build[count].y, lineThick);
								}

								setfillcolor(RED);
								Sleep(speed);
								solidcircle(x1, y1, lineThick / 2);
								tempx = x1, tempy = y1;
								if (x1 < x2) x1++;
								else if (x1 > x2) x1--;
								if (y1 < y2) y1 += k;
								else if (y1 > y2) y1 -= k;
								flag = 1;
							}
							if (changeBreak) break;

							strcpy_s(plogic, 20, build[Roads[i + 1]].logclLoc);
							sprintf_s(things, 50, "%2d年%2d月%2d日: %2d:%2d到达：%s\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, plogic);
							printf("%s \n", things);
							writeFile(fp, things, 1);//写日志，这个时间就是到达下一站时间 
						}

						EndBatchDraw();

						fclose(fp);

						if (navigationInProgress) {
							printf("\n本次导航结束，感谢您的使用！\n");
							setfillcolor(RGB(162, 209, 106));
							settextcolor(RGB(19, 21, 19));
							settextstyle(30, 0, _T("仿宋"));
							solidrectangle(1280, 700, 1340, 730);
							outtextxy(1280, 700, L"返回");
							while (1) {
								if (MouseHit()) {
									MOUSEMSG msgreturn = GetMouseMsg();
									if (msgreturn.x >= 1280 && msgreturn.x <= 1340 && msgreturn.y >= 700 && msgreturn.y <= 730 && msgreturn.mkLButton) {
										whichPicture = 0;
										navigationInProgress = 0;
										startPosition = -1, terminalPosition = -1, navigationStrategy = -1;
										cleardevice();
										loadimage(NULL, _T("homepage.jpg"));
										break;
									}
								}
							}
						}
					}
				default:
					break;
				}
			}
		}
	}

	getchar();
	closegraph();

}

void getgraphtxt(struct graph* Gp) {
	int j = 0;
	int err;
	struct road* p = NULL;
	FILE* fpp;
	int i = 0, k = 0, max1;
	if ((err = fopen_s(&fpp, BUILDFILE, "r")) != 0) {
		printf("Open the filebuild failure...\n");
		exit(0);
	}
	fscanf_s(fpp, "%d", &max1);
	char s[maxbuilds][4][1000], ch;
	int flag = 0;
	while ((ch = fgetc(fpp)) != EOF) {
		//按字符方式读取文件，每读一个字符判断一下是否为汉字编码(<0)，
		//若是则存入字符数组，不是则跳过读取下一个字符，直到文件结束。  	
		if (ch < 0)     s[k][j][i++] = ch;
		if (ch == ';') { s[k][j][i] = '\0'; j++; i = 0; }
		if (flag) {
			if (ch == '\n') { s[k][j][i] = '\0'; k++; j = 0; i = 0; }
		}
		flag = 1;
	}
	fclose(fpp);
	(*Gp).buildnum = max1;
	for (i = 0; i < ((*Gp).buildnum); i++) {
		(*Gp).buildings[i].b_number = i;
		strcpy_s((*Gp).buildings[i].b_name, s[i][0]);
		(*Gp).buildings[i].first_road = NULL;
	}
	int max;
	FILE* fp;
	fopen_s(&fp, ROADFILE, "r");
	if (NULL == fp) {
		printf("Failed to open the fileroad!\n");
		exit(0);
	}
	fscanf_s(fp, "%d", &max);
	(*Gp).roadnum = max;
	int a[maxbuilds], b[maxbuilds], c[maxbuilds], g[maxbuilds];
	float f[maxbuilds];
	for (int i = 0; i < max; i++) {
		fscanf_s(fp, "%d%d%d%f%d", &a[i], &b[i], &c[i], &f[i], &g[i]);
	}//从fp所指文件中读取一个整数保存到变量a[]...中
	fclose(fp);
	for (j = 0; j < (*Gp).roadnum; j++) {
		p = (struct road*)malloc(sizeof(struct road));
		p->mark = 0;//未被访问 
		p->leftbuild = a[j];
		p->rightbuild = b[j];
		//		p->info=NULL;
		p->distan = c[j];
		p->crowd = f[j];
		p->tranmark = g[j];
		p->leftnext = (*Gp).buildings[a[j]].first_road;
		(*Gp).buildings[a[j]].first_road = p;
		p->rightnext = (*Gp).buildings[b[j]].first_road;
		(*Gp).buildings[b[j]].first_road = p;
	}
}

int get_longth(int a, int b) {
	struct road* p = (*GP).buildings[a].first_road;
	while (p != NULL) {
		if (p->rightbuild == a) {
			if (p->leftbuild == b)
				return p->distan;
			p = p->rightnext;
		}
		else if (p->leftbuild == a) {
			if (p->rightbuild == b)
				return p->distan;
			p = p->leftnext;
		}
	}
	return INF;
}

void shortlongth(int first) {
	int i, j, w, v, min;
	for (i = 0; i < (*GP).buildnum; i++) {
		D[i] = get_longth(first, i);
		final[i] = FALSE;
		for (w = 0; w < (*GP).buildnum; w++) {
			P[i][w] = FALSE;
		}
		if (D[i] < INF) P[i][first] = TRUE;
	}//初始化 
	D[first] = 0;
	final[first] = TRUE;//final 表示这已经找到最短路径了的节点 
	for (j = 1; j < (*GP).buildnum; j++) {
		min = INF;
		for (w = 0; w < (*GP).buildnum; w++) {
			if (!final[w])
				if (D[w] < min) {//未被标记过（确定final）的 
					v = w;
					min = D[w];
				}
		}
		final[v] = TRUE;//找到first最近的未被确定的建筑 v
		for (w = 0; w < (*GP).buildnum; w++) {
			if (!final[w])
				if (min + get_longth(v, w) < D[w]) {//min是first到v的距离  
					D[w] = min + get_longth(v, w);//判断加入v后w是否更短,更新 
					P[w][v] = TRUE;
					P[w][first] = FALSE;//D改过w就不是连接first的最短路径途径点 
				}
		}
	}

}

int getRoads(int first, int second, int num) {	//获取当前路径并存到Roads[] 
	int i, j, k = -1, d[MAX_Buildings];
	j = second;
	for (i = 0; i < (*GP).buildnum; i++) {
		if (P[j][i] == TRUE) {
			if (i != first) {
				d[++k] = j;
				j = i;
				i = -1;
			}
			else {
				d[++k] = j;
				d[++k] = i;
				break;
			}
		}
	}
	Buildby = k + num + 1;//总共经过建筑的个数 
	if (num) {
		for (i = num - 1; k > 0; k--, i++) {
			Roads[i] = d[k];
		}
		Buildby -= 1;
	}
	else {
		for (i = num; k > 0; k--, i++) {
			Roads[i] = d[k];
		}
	}
	Roads[i] = d[k];
	return Buildby;
}

void va_shortlongth(int first, int last, int va) {
	int vaflag = 0, numm;
	shortlongth(first);
	numm = getRoads(first, va, vaflag);
	vaflag += numm;
	shortlongth(va);
	numm = getRoads(va, last, vaflag);
	vaflag += numm;
}

int get_time(int a, int b)
{
	int spendTime, crowd10;
	struct road* p = (*GP).buildings[a].first_road;
	while (p != NULL) {
		if (p->rightbuild == a)
		{
			if (p->leftbuild == b)
			{
				crowd10 = (int)(10 * p->crowd);
				spendTime = 10 * p->distan / speedw * crowd10;
				return spendTime;
			}
			p = p->rightnext;
		}
		else if (p->leftbuild == a)
		{
			if (p->rightbuild == b)
			{
				crowd10 = (int)(10 * p->crowd);
				spendTime = 10 * p->distan / speedw * crowd10;
				return spendTime;
			}
			p = p->leftnext;
		}
	}
	return INF;
}

int get_bike_time(int a, int b)
{
	int spendTime, crowd10;
	struct road* p = (*GP).buildings[a].first_road;
	while (p != NULL)//&&
	{
		if (p->rightbuild == a)
		{
			if (p->leftbuild == b)
			{
				if (p->tranmark == 1) {
					crowd10 = (int)(10 * p->crowd);
					spendTime = 10 * p->distan / bike_speed * crowd10;
					return spendTime;
				}
				else if (p->tranmark == 0) {
					crowd10 = (int)(10 * p->crowd);
					spendTime = 10 * p->distan / walk_speed * crowd10;
					return spendTime;
				}
			}
			p = p->rightnext;
		}
		else if (p->leftbuild == a)
		{
			if (p->rightbuild == b)
			{
				if (p->tranmark == 1) {
					crowd10 = (int)(10 * p->crowd);
					spendTime = 10 * p->distan / bike_speed * crowd10;
					return spendTime;
				}
				else if (p->tranmark == 0) {
					crowd10 = (int)(10 * p->crowd);
					spendTime = 10 * p->distan / walk_speed * crowd10;
					return spendTime;
				}
			}
			p = p->leftnext;
		}
	}
	return INF;
}

void short_time(int first) {
	int i, j, w, v, min;
	for (i = 0; i < (*GP).buildnum; i++) {
		D[i] = get_time(first, i);
		final[i] = FALSE;
		for (w = 0; w < (*GP).buildnum; w++) {
			P[i][w] = FALSE;
		}
		if (D[i] < INF) P[i][first] = TRUE;
	}//初始化
	D[first] = 0;
	final[first] = TRUE;//final 表示这已经找到最短路径了的节点
	for (j = 1; j < (*GP).buildnum; j++) {
		min = INF;
		for (w = 0; w < (*GP).buildnum; w++) {
			if (!final[w])
				if (D[w] < min) {//未被标记过（确定final）的
					v = w;
					min = D[w];
				}
		}
		final[v] = TRUE;//找到first最近的未被确定的建筑 v
		for (w = 0; w < (*GP).buildnum; w++) {
			if (!final[w])
				if (min + get_time(v, w) < D[w]) {//min是first到v的距离
					D[w] = min + get_time(v, w);//判断加入v后w是否更短,更新
					P[w][v] = TRUE;
					P[w][first] = FALSE;//D改过w就不是连接first的最短路径途径点
				}
		}
	}

}

void tran_short(int first) {
	int i, j, w, v, min;
	for (i = 0; i < (*GP).buildnum; i++) {
		D[i] = get_bike_time(first, i);
		final[i] = FALSE;
		for (w = 0; w < (*GP).buildnum; w++) {
			P[i][w] = FALSE;
		}
		if (D[i] < INF) P[i][first] = TRUE;
	}//初始化
	D[first] = 0;
	final[first] = TRUE;//final 表示这已经找到最短路径了的节点
	for (j = 1; j < (*GP).buildnum; j++) {
		min = INF;
		for (w = 0; w < (*GP).buildnum; w++) {
			if (!final[w])
				if (D[w] < min) {//未被标记过（确定final）的
					v = w;
					min = D[w];
				}
		}
		final[v] = TRUE;//找到first最近的未被确定的建筑 v
		for (w = 0; w < (*GP).buildnum; w++) {
			if (!final[w])
				if (min + get_bike_time(v, w) < D[w]) {//min是first到v的距离
					D[w] = min + get_bike_time(v, w);//判断加入v后w是否更短,更新
					P[w][v] = TRUE;
					P[w][first] = FALSE;//D改过w就不是连接first的最短路径途径点
				}
		}
	}

}

void outroads(int first, int second) {
	int i, j, k = -1, d[maxbuilds];
	j = second;
	printf("最短距离为：%d\n", D[second]);
	printf("途径各建筑物编号为：");
	for (i = 0; i < (*GP).buildnum; i++) {
		if (P[j][i] == TRUE) {
			if (i != first) {
				d[++k] = j;
				j = i;
				i = -1;
			}
			else {
				d[++k] = j;
				d[++k] = i;
				break;
			}
		}
	}
	for (; k > 0; k--)//按顺序输出路径编号 
		printf("%d->", d[k]);
	printf("%d\n", d[k]);
}

void outroads2(int first, int second) {
	int i, j, k = -1, d[maxbuilds];
	j = second;
	printf("最短距离为：%d\n", D[second]);
	printf("途径各建筑物为：");
	for (i = 0; i < (*GP).buildnum; i++) {
		if (P[j][i] == TRUE) {
			if (i != first) {
				d[++k] = j;
				j = i;
				i = -1;
			}
			else {
				d[++k] = j;
				d[++k] = i;
				break;
			}
		}
	}
	for (; k > 0; k--) {//按顺序输出路径编号 
		printf("%d", d[k]);
		printf("%s", (*GP).buildings[d[k]].b_name);
		printf("->");
	}
	printf("%d", d[k]);
	printf("%s", (*GP).buildings[d[k]].b_name);
	printf("\n");
}

int theNearestBuilding(int tempx, int tempy, int x1, int y1, int x2, int y2) {
	double dis1, dis2;
	int absx1, absy1, absx2, absy2;
	if (tempx > x1)
		absx1 = tempx - x1;
	else
		absx1 = x1 - tempx;
	if (tempy > y1)
		absy1 = tempy - y1;
	else
		absy1 = y1 - tempy;
	dis1 = sqrt(absx1 * absx1 + absy1 * absy1);
	if (tempx > x2)
		absx2 = tempx - x2;
	else
		absx2 = x2 - tempx;
	if (tempy > y2)
		absy2 = tempy - y2;
	else
		absy2 = y2 - tempy;
	dis2 = sqrt(absx2 * absx2 + absy2 * absy2);
	return dis1 > dis2 ? 1 : 2;
}
