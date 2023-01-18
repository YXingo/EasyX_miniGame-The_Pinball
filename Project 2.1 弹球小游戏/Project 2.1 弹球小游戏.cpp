#include <iostream>
#include <vector>
#include <easyx.h>
#include <conio.h>
using namespace std;

#define BLOCK_WIDTH 40

/* 类的声明 */
class Block;		//方块类
class Ball;			//小球类
class Guard;		//挡板类
class Level_Data;	//关卡数据类

/* 函数的声明 */
Level_Data setLevel_1_Data();	//编辑第一关关卡数据
Level_Data setLevel_2_Data();	//编辑第二关关卡数据
Level_Data setLevel_3_Data();	//编辑第三关关卡数据
void printGrid();	//绘制网格线
bool isGameOver(Ball ball);				//判断游戏是否结束
bool isOnAngle(Ball ball, Block block);	//判断小球撞到了方块的四个角还是四条边
void gameInitialization(Ball& ball, Guard& guard, Level_Data level_data);	//游戏内容初始化


/********************************** 类的定义分割线 **********************************/

class Block	//方块类 目前设定为：白色方块1分 黄色方块3分 红色方块5分
{
public:
	int m_Length = BLOCK_WIDTH;
	int m_X; int m_Y;
	COLORREF m_Color;

	Block(int x, int y, COLORREF color)
	{
		m_X = x; m_Y = y;
		m_Color = color;
	}

	void printBlock()
	{
		setlinecolor(BLACK);	//方块边框默认为黑色 便于识别
		setfillcolor(m_Color);	//方块填充色为m_Color 便于制作不同种类 不同得分的方块
		fillrectangle(m_X * BLOCK_WIDTH, m_Y * BLOCK_WIDTH, (m_X + 1) * BLOCK_WIDTH, (m_Y + 1) * BLOCK_WIDTH);
		//绘制方块
	}
};


class Guard	//挡板类
{
public:
	int m_Top = 580;
	int m_Bottom = 600;
	int m_Left = 320;
	int m_Right = 480;

	void guardInitialization()	//游戏开始时对挡板位置和大小的初始化
	{
		m_Top = 580;
		m_Bottom = 600;
		m_Left = 320;
		m_Right = 480;
		//设置挡板位于画面正下方
	}

	string guardchange()	//改变挡板的位置
	{
		char direction = _getch();	//让用户决定挡板的移动方向

		if (direction == 'a' || direction == 'd')
		{
			if (direction == 'a')	//用户按下a键时 挡板向左移动半格
			{
				m_Left -= BLOCK_WIDTH / 2;
				m_Right -= BLOCK_WIDTH / 2;
			}
			else if (direction == 'd')	//用户按下d键时 挡板向右移动半格
			{
				m_Left += BLOCK_WIDTH / 2;
				m_Right += BLOCK_WIDTH / 2;
			}
			return "ENTER_CORRECT";
		}
		else
		{
			return "ENTER_EEROR";
		}
	}

	void guardprint()	//绘制挡板
	{
		setfillcolor(WHITE);
		setlinecolor(WHITE);
		fillrectangle(m_Left, m_Top, m_Right, m_Bottom);
	}

	void guardmove()	//实现一次挡板的移动
	{
		guardchange();
		guardprint();
	}
};

class Level_Data	//每一关的方块分布
{
public:
	vector<vector<Block>> Block_Arrangement;	// Block动态数组(颜色不同)的动态数组作为存储关卡布置的容器

	void printLevel()
	{
		for (vector<vector<Block>>::iterator it_1 = Block_Arrangement.begin(); it_1 != Block_Arrangement.end(); it_1++)
		{
			for (vector<Block>::iterator it_2 = (*it_1).begin(); it_2 != (*it_1).end(); it_2++)
			{
				setlinecolor(BLACK);
				setfillcolor((*it_2).m_Color);
				(*it_2).printBlock();
			}
		}
		// 使用双重循环遍历 从而绘制关卡的方块布置
	}

	Block getCrashBlock(int ball_X, int ball_Y, int ball_R)
		// 该函数传入的变量是小球当前的各个属性（坐标和半径）
		// 该函数用于获得被小球碰撞的方块 如果小球没有碰撞到方块 则返回 noCrash(-1,-1,BLACK) 的方块属性
	{
		for (vector<vector<Block>>::iterator it_1 = this->Block_Arrangement.begin(); it_1 != this->Block_Arrangement.end(); it_1++)
		{
			for (vector<Block>::iterator it_2 = (*it_1).begin(); it_2 != (*it_1).end(); it_2++)
			{
				HRGN rgn1 = CreateEllipticRgn(ball_X - ball_R, ball_Y - ball_R, ball_X + ball_R, ball_Y + ball_R);
				//创建一个与小球完全一样的区域
				HRGN rgn2 = CreateRectRgn((*it_2).m_X * BLOCK_WIDTH, (*it_2).m_Y * BLOCK_WIDTH, ((*it_2).m_X + 1) * BLOCK_WIDTH, ((*it_2).m_Y + 1) * BLOCK_WIDTH);
				//创建一个与循环到的方块完全一样的区域
				HRGN rgn_result = CreateRectRgn(0, 0, 0, 0);
				//创建一个空区域 用于接收组合后的区域
				int result = CombineRgn(rgn_result, rgn1, rgn2, RGN_AND);
				//CombineRgn返回值为int类型 这里用result接收 从而及时删除创建的区域 避免出现边框消失的情况
				DeleteObject(rgn_result);
				DeleteObject(rgn1);
				DeleteObject(rgn2);	//创建了区域就要销毁区域

				if (result != NULLREGION)
					//NULLREGION表示组合区域为空区域 如果不为空 则表示小球碰撞到了方块 此时返回所碰撞到的方块并让被碰撞到的方块消失
					//交集只有一个像素点时 似乎也认为是空区域
				{
					Block crashBlock = (*it_2);		//创建临时变量crashBlock作为返回值接收被碰撞到的方块 便于删除此方块
					(*it_1).erase(it_2);
					return crashBlock;
				}
			}
		}
		// 使用双重循环遍历 从而判断小球是否碰到了方块
		// 如果没有碰到方块 那么循环终将结束 到达下面步骤 从而返回noCrash方块 noCrash的三个属性都被设置为了反常数据 便于后续程序识别
		Block noCrash(-1, -1, BLACK);
		return noCrash;
	}
};

class Ball	//小球类
{
public:
	int m_X = 400;
	int m_Y = 559;
	int m_R = 20;
	int m_Vx = 0;
	int m_Vy = 0;

	void ballInitialization()	//游戏开始时对小球位置和大小的初始化
	{
		this->m_X = 400;
		this->m_Y = 559;
		this->m_R = 20;
		//设置球初始处于挡板中心的上方 但不接触挡板 避免初始小球运动异常

		char direction = _getch();	//让用户决定小球一开始运动的方向
		if (direction == 'a')	//如果用户按下a键 则小球初始向左上方运动
		{
			this->m_Vx = -5;
			this->m_Vy = -5;
		}
		else if (direction == 'd')	//如果用户按下d键 则小球初始向右上方运动
		{
			this->m_Vx = 5;
			this->m_Vy = -5;
		}
	}

	void ballchange(const Guard guard, Level_Data& level_data)	// 改变小球自身的属性（速度方向和位置）
	{
		if (this->m_X >= guard.m_Left && this->m_X <= guard.m_Right && this->m_Y + this->m_R >= guard.m_Top && this->m_Y + this->m_R <= guard.m_Bottom) //如果小球碰到挡板 则改变运动方向
		{
			this->m_Vy = -this->m_Vy;	// y方向速度反向
			this->m_Y = 559;
		}

		if (m_X + m_R >= 800 || m_X - m_R <= 0)	//如果小球碰到左右边缘 改变运动方向
		{
			this->m_Vx = -this->m_Vx;	// x方向速度反向
		}

		if (this->m_Y - this->m_R <= 0)	//如果小球碰到上边缘 改变运动方向
		{
			this->m_Vy = -this->m_Vy;	// y方向速度反向
		}

		if (level_data.getCrashBlock(this->m_X, this->m_Y, this->m_R).m_X != -1)	//如果小球碰到方块 改变运动方向
		{
			Block crashBlock = level_data.getCrashBlock(this->m_X, this->m_Y, this->m_R);	//获取碰撞的方块


			/* 之前的碰撞方块后的移动代码 思路是分为碰到方块四个边和四个角处理 但出现了不符合常理的运动现象 */
			//if (isOnAngle(*this, crashBlock) == true)	//如果小球碰到方块的四个角 做如下运动
			//{
			//	double v_Sum = sqrt(pow(this->m_Vx, 2) + pow(this->m_Vy, 2));	//求出小球合速度 并且在与方块碰撞过程中 小球合速度不变
			//	double alpha = atan((double)this->m_Vy / this->m_Vx);	//获得碰撞前合速度与x轴正方向夹角
			//	double gamma = atan((double)abs(this->m_X - crashBlock.m_X) / abs(this->m_Y - crashBlock.m_Y));	//获得小球中心和方块中心连线与垂直方向夹角
			//	double beta = 2 * gamma - alpha;	//获得碰撞后合速度与x轴正方向夹角

			//	this->m_Vx = v_Sum * cos(beta);
			//	this->m_Vy = v_Sum * sin(beta);
			//}
			//else if (isOnAngle(*this, crashBlock) == false)	//如果小球碰到方块的边边 做如下运动
			//{
			// 	int block_Top = crashBlock.m_Y - 0.5 * BLOCK_WIDTH;		//方块的顶部
			//	int block_Bottom = crashBlock.m_Y + 0.5 * BLOCK_WIDTH;	//方块的底部
			//	int block_Left = crashBlock.m_X - 0.5 * BLOCK_WIDTH;	//方块的左边
			//	int block_Right = crashBlock.m_X + 0.5 * BLOCK_WIDTH;	//方块的右边
			//	if (this->m_X < block_Left || this->m_X > block_Right)		//碰到左右边
			//	{
			//		this->m_Vx = -this->m_Vx;
			//	}
			//	else if (this->m_Y > block_Bottom || this->m_Y < block_Top)	//碰到上下边
			//	{
			//		this->m_Vy = -this->m_Vy;
			//	}
			//}

			/* 简化后的碰撞代码 无论小球以何种角度碰到方块 都认为是碰到四边之一 */
			int out_block_Top = crashBlock.m_Y - 0.5 * BLOCK_WIDTH - this->m_R / sqrt(2);	//方块的外顶部
			int out_block_Bottom = crashBlock.m_Y + 0.5 * BLOCK_WIDTH + this->m_R / sqrt(2);//方块的外底部
			int out_block_Left = crashBlock.m_X - 0.5 * BLOCK_WIDTH - this->m_R / sqrt(2);	//方块的外左边
			int out_block_Right = crashBlock.m_X + 0.5 * BLOCK_WIDTH + this->m_R / sqrt(2);	//方块的外右边
			if (this->m_Y > out_block_Bottom || this->m_Y < out_block_Top)
			{
				this->m_Vy = -this->m_Vy;
			}
			else if (this->m_X > out_block_Right || this->m_X < out_block_Left)
			{
				this->m_Vx = -this->m_Vx;
			}
		}

		//使用改变后的运动方向来改变小球的位置
		m_X += m_Vx;
		m_Y += m_Vy;
	}

	void ballprint()	//绘制小球
	{
		setfillcolor(WHITE);
		setlinecolor(WHITE);
		fillcircle(m_X, m_Y, m_R);
	}

	void ballmove(const Guard guard, Level_Data& level_data)		// 实现一次小球的运动
	{
		ballchange(guard, level_data);
		ballprint();
	}
};


/********************************** 类的定义分割线 **********************************/

/******************************** 编辑关卡数据分割线 ********************************/

vector<Block> White_Blocks;		//白色方块动态数组
vector<Block> Yellow_Blocks;	//黄色方块动态数组
vector<Block> Red_Blocks;		//红色方块动态数组

Level_Data setLevel_1_Data()	//用函数编辑各个关卡数据
{
	Level_Data result;	//最终作为返回值的变量

	White_Blocks.clear();
	Yellow_Blocks.clear();
	Red_Blocks.clear();		//清空三个vector容器 防止容器内原有元素影响

	/* 开始编辑盛放白色方块的容器 */
	for (int i = 0; i < 800 / BLOCK_WIDTH; i++)
	{
		Block block(i, 1, WHITE);
		White_Blocks.push_back(block);
	}
	for (int i = 0; i < 320 / BLOCK_WIDTH; i++)
	{
		Block block(3, i, WHITE);
		White_Blocks.push_back(block);
	}
	for (int i = 0; i < 320 / BLOCK_WIDTH; i++)
	{
		Block block(16, i, WHITE);
		White_Blocks.push_back(block);
	}

	/* 第一关暂且只设置白色方块 */

	result.Block_Arrangement.push_back(White_Blocks);
	result.Block_Arrangement.push_back(Yellow_Blocks);
	result.Block_Arrangement.push_back(Red_Blocks);	//将三个block动态数组插入

	return result;
}

//Level_Data setLevel_2_Data()
//{
//	return;
//}
//
//Level_Data setLevel_3_Data()
//{
//	return;
//}


const Level_Data level_1 = setLevel_1_Data();		//第一关关卡数据
//const Level_Data level_2 = setLevel_2_Data();		//第二关关卡数据
//const Level_Data level_3 = setLevel_3_Data();		//第三关关卡数据

/******************************* 编辑关卡数据分割线 **********************************/

void printGrid()
{
	setlinecolor(WHITE);
	for (int i = 1; i <= (800 / BLOCK_WIDTH) - 1; i++)
	{
		line(i * BLOCK_WIDTH, 0, i * BLOCK_WIDTH, 600);
	}
	for (int i = 1; i <= (600 / BLOCK_WIDTH) - 1; i++)
	{
		line(0, i * BLOCK_WIDTH, 800, i * BLOCK_WIDTH);
	}
}

bool isGameOver(Ball ball)	//判断游戏是否结束（小球下端运动到画面下端即为游戏结束）
{
	if (ball.m_Y + ball.m_R >= 600)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool isOnAngle(Ball ball, Block block)
{
	int block_Top = block.m_Y - 0.5 * BLOCK_WIDTH;		//方块的顶部
	int block_Bottom = block.m_Y + 0.5 * BLOCK_WIDTH;	//方块的底部
	int block_Left = block.m_X - 0.5 * BLOCK_WIDTH;		//方块的左边
	int block_Right = block.m_X + 0.5 * BLOCK_WIDTH;	//方块的右边

	if (ball.m_X > block_Right && ball.m_Y > block_Bottom)	//撞到右下角
	{
		return true;
	}
	else if (ball.m_X > block_Right && ball.m_Y < block_Top)	//撞到右上角
	{
		return true;
	}
	else if (ball.m_X < block_Left && ball.m_Y < block_Top)	//撞到左上角
	{
		return true;
	}
	else if (ball.m_X < block_Left && ball.m_Y > block_Bottom)//撞到左下角
	{
		return true;
	}

	return false;
}

void gameInitialization(Ball& ball, Guard& guard, Level_Data level_data)
{
	ball.ballInitialization();	guard.guardInitialization();	//对小球和挡板进行初始化（这里需要用户输入一个方向来决定小球运动的方向）
	ball.ballprint();			guard.guardprint();				//绘制小球和挡板
	level_data.printLevel();	//绘制关卡布局
}

int main()
{
	initgraph(800, 600);
	BeginBatchDraw();
	setbkcolor(RGB(164, 225, 202));
	cleardevice();

	FlushBatchDraw();

	Level_Data temp_Data = level_1;		//第一关
	//在定义level_1时 加上了const限定防止关卡数据被修改 这里创建新的临时变量来接受关卡数据
	Ball ball;	Guard guard;	//创建小球和挡板
	gameInitialization(ball, guard, level_1);	//对第一关进行初始化

	while (1)
	{
		cleardevice();	//清屏 重新绘制图画

		if (_kbhit() != 0)	//如果用户有输入 则进入该分支判断是否对挡板进行移动
		{
			guard.guardchange();
		}

		temp_Data.printLevel();	//绘制关卡布局
		guard.guardprint();		//绘制挡板
		ball.ballmove(guard, temp_Data);	//小球移动后 绘制小球

		if (isGameOver(ball) == true)	//如果游戏结束 则进行初始化
		{
			gameInitialization(ball, guard, level_1);	//对第一关进行初始化
			temp_Data = level_1;
		}

		FlushBatchDraw();	//批量绘图
		Sleep(1000.0 / 120);//获得120帧的游戏画面
	}

	EndBatchDraw();
	getchar();
	closegraph();
}