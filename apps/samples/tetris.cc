// HWRECTView.cpp : implementation of the CHWRECTView class
//

#include <windows.h>
#include <cdlog.h>
#include <sstream>

namespace newglee{

static constexpr int BLOCK_SIZE=25; //单个方块单元的边长
static constexpr int MARGIN=5; //场景边距
static constexpr int AREA_ROW=25; //场景行数
static constexpr int AREA_COL=15; //场景列数
#define WM_REFRESH (View::WM_USER+100)
//方向
enum Direction{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    SPACE
};

//定义边界信息
struct Border{
    int ubound;
    int dbound;
    int lbound;
    int rbound;
};

//坐标
struct block_point{
    int pos_x;
    int pos_y;
    //    block_point(int x,int y):pos_x(x),pos_y(y){}
};
//下降，左移，右移宏定义
int item1[4][4]={
    {0,0,0,0},
    {0,1,1,0},
    {0,1,1,0},
    {0,0,0,0}
};
//右L
int item2[4][4]={
    {0,1,0,0},
    {0,1,0,0},
    {0,1,1,0},
    {0,0,0,0}
};
//左L
int item3[4][4]={
    {0,0,1,0},
    {0,0,1,0},
    {0,1,1,0},
    {0,0,0,0}
};
//右S
int item4[4][4]={
    {0,1,0,0},
    {0,1,1,0},
    {0,0,1,0},
    {0,0,0,0}
};
//左S
int item5[4][4]={
    {0,0,1,0},
    {0,1,1,0},
    {0,1,0,0},
    {0,0,0,0}
};
//山形
int item6[4][4]={
    {0,0,0,0},
    {0,0,1,0},
    {0,1,1,1},
    {0,0,0,0}
};
//长条
int item7[4][4]={
    {0,0,1,0},
    {0,0,1,0},
    {0,0,1,0},
    {0,0,1,0}
};

inline void block_cpy(int dblock[4][4],int sblock[4][4]){
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            dblock[i][j]=sblock[i][j];
}
class TetrisWindow:public Window{
private:
    int game_area[AREA_ROW][AREA_COL]; //场景区域，1表示活动的方块，2表示稳定的方块，0表示空
    block_point block_pos; //当前方块坐标
    int cur_block[4][4]; //当前方块形状
    Border cur_border; //当前方块边界
    int next_block[4][4]; //下一个方块形状
    bool isStable; //当前方块是否稳定了
    int score;  //游戏分数
    int game_timer; //方块下落计时器
    int paint_timer; //渲染刷新计时器
    int speed_ms; //下落时间间隔
    int refresh_ms; //刷新时间间隔
    int mGameIsOver;
    int mTopLeft;
    Spinner*mLevelSelector;
    Button*mStartButton;
protected:
    bool onMessage(DWORD msgid,DWORD wp,ULONG lp)override;
    void onDraw(Canvas&canvas)override;
    bool onKeyUp(int keyCode,KeyEvent&event)override;
public:
    TetrisWindow(int x,int y,int w,int h);
    void InitGame(); //初始化
    void StartGame(); //开始游戏
    void GameOver(); //游戏结束

    void ResetBlock(); //重置方块
    void BlockMove(Direction dir); //方块变动
    void BlockRotate(int block[4][4]); //方块旋转
    void CreateBlock(int block[4][4],int block_id); //产生方块
    void GetBorder(int block[4][4],Border &border); //计算边界
    void ConvertStable(int x,int y); //转换为稳定方块
    bool IsCollide(int x,int y,Direction dir); //判断是否会碰撞
};

TetrisWindow::TetrisWindow(int x,int y,int w,int h):Window(x,y,w,h){
    setText("Tetris");
    InitGame();
    mLevelSelector=new Spinner(300,30);
    mTopLeft=300;
    const int timeIntervals[]={700,600,500,400,300};
    /*for(int i=0;i<5;i++){
        //mLevelSelector->addItem(new Selector::ListItem(std::to_string(i+1),timeIntervals[i]));
    }
    mLevelSelector->setItemSelectListener([this](AbsListView&lv,const ListView::ListItem&lvitem,int index){
        speed_ms=lvitem.getId();
    });
    mLevelSelector->setSelection(0);*/
    int cx=mTopLeft+AREA_COL*BLOCK_SIZE+MARGIN*5;
    addView(mLevelSelector).setPos(cx,300);
    mStartButton=new Button("Start",300,30);
    addView(mStartButton).setPos(cx,350);
    mStartButton->setOnClickListener([this](View&v){
        mGameIsOver=0;
        StartGame();
    });
}
void TetrisWindow::InitGame(){
    for(int i=0;i<AREA_ROW;i++)
        for(int j=0;j<AREA_COL;j++)
            game_area[i][j]=0;

    speed_ms=800;
    refresh_ms=30;

    //初始化随机数种子
    srand(time(0));

    //分数清0
    score=0;
    mGameIsOver=true;
    //开始游戏
}
void TetrisWindow::GetBorder(int block[4][4],Border &border){
    //计算上下左右边界
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            if(block[i][j]==1)
            {
                border.dbound=i;
                break; //直到计算到最后一行有1
            }
    for(int i=3;i>=0;i--)
        for(int j=0;j<4;j++)
            if(block[i][j]==1)
            {
                border.ubound=i;
                break;
            }
    for(int j=0;j<4;j++)
        for(int i=0;i<4;i++)
            if(block[i][j]==1)
            {
                border.rbound=j;
                break;
            }
    for(int j=3;j>=0;j--)
        for(int i=0;i<4;i++)
            if(block[i][j]==1)
            {
                border.lbound=j;
                break;
            }
}

void TetrisWindow::CreateBlock(int block[4][4],int block_id){
    switch (block_id)
    {
    case 0:
        block_cpy(block,item1);
        break;
    case 1:
        block_cpy(block,item2);
        break;
    case 2:
        block_cpy(block,item3);
        break;
    case 3:
        block_cpy(block,item4);
        break;
    case 4:
        block_cpy(block,item5);
        break;
    case 5:
        block_cpy(block,item6);
        break;
    case 6:
        block_cpy(block,item7);
        break;
    default:
        break;
    }
}
void TetrisWindow::ResetBlock(){
    //产生当前方块
    block_cpy(cur_block,next_block);
    GetBorder(cur_block,cur_border);

    //产生下一个方块
    int block_id=rand()%7;
    CreateBlock(next_block,block_id);

    //设置初始方块坐标,以方块左上角为锚点
    block_point start_point;
    start_point.pos_x=AREA_COL/2-2;
    start_point.pos_y=0;
    block_pos=start_point;
}

void TetrisWindow::StartGame(){
    sendMessage(WM_REFRESH,0,0,speed_ms);
    //产生初始下一个方块
    int block_id=rand()%7;
    CreateBlock(next_block,block_id);
    ResetBlock(); //产生方块
}

bool TetrisWindow::onKeyUp(int keyCode,KeyEvent&event){
  if(mGameIsOver)
      return  Window::onKeyUp(keyCode,event);
  switch(keyCode)
    {
    case KEY_UP:
        BlockMove(UP);
        break;
    case KEY_DOWN:
        BlockMove(DOWN);
        break;
    case KEY_LEFT:
        BlockMove(LEFT);
        break;
    case KEY_RIGHT:
        BlockMove(RIGHT);
        break;
    case KEY_SPACE:
    case KEY_OK:
        BlockMove(SPACE);
        break;
    default:return Window::onKeyUp(keyCode,event);
    }
    invalidate(nullptr);
    return true;
}

bool TetrisWindow::onMessage(DWORD msgid,DWORD wp,ULONG lp){
    if((msgid==WM_REFRESH)&&(!mGameIsOver)){
         BlockMove(DOWN);
         invalidate(nullptr);
         sendMessage(msgid,wp,lp,speed_ms);
    }else
        return Window::onMessage(msgid,wp,lp);
}

void TetrisWindow::GameOver(){
    //游戏结束停止计时器
    //killTimer(game_timer);
    //killTimer(paint_timer);
    //QMessageBox::information(this,"failed","game over");
}

void TetrisWindow::BlockRotate(int block[4][4]){
    int temp_block[4][4];
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            temp_block[3-j][i]=block[i][j];
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            block[i][j]=temp_block[i][j];
}

void TetrisWindow::ConvertStable(int x,int y){
    for(int i=cur_border.ubound;i<=cur_border.dbound;i++)
        for(int j=cur_border.lbound;j<=cur_border.rbound;j++)
            if(cur_block[i][j]==1)
                game_area[y+i][x+j]=2; //x和y别搞反
}

bool TetrisWindow::IsCollide(int x,int y,Direction dir){
    //用拷贝的临时方块做判断
    int temp_block[4][4];
    block_cpy(temp_block,cur_block);
    Border temp_border;
    GetBorder(temp_block,temp_border);
    //先尝试按照某方向走一格
    switch(dir)
    {
    case UP:
        BlockRotate(temp_block);
        GetBorder(temp_block,temp_border); //旋转后要重新计算边界
        break;
    case DOWN:
        y+=1;
        break;
    case LEFT:
        x-=1;
        break;
    case RIGHT:
        x+=1;
        break;
    default:
        break;
    }
    for(int i=temp_border.ubound;i<=temp_border.dbound;i++)
        for(int j=temp_border.lbound;j<=temp_border.rbound;j++)
            if(game_area[y+i][x+j]==2&&temp_block[i][j]==1||x+temp_border.lbound<0||x+temp_border.rbound>AREA_COL-1)
                return true;
    return false;
}

void TetrisWindow::BlockMove(Direction dir){
    switch (dir) {
    case UP:
        if(IsCollide(block_pos.pos_x,block_pos.pos_y,UP))
            break;
        //逆时针旋转90度
        BlockRotate(cur_block);
        //防止旋转后bug,i和j从0到4重新设置方块
        for(int i=0;i<4;i++)
            for(int j=0;j<4;j++)
                game_area[block_pos.pos_y+i][block_pos.pos_x+j]=cur_block[i][j];
        //重新计算边界
        GetBorder(cur_block,cur_border);
        break;
    case DOWN:
        //方块到达边界则不再移动
        if(block_pos.pos_y+cur_border.dbound==AREA_ROW-1)
        {
            ConvertStable(block_pos.pos_x,block_pos.pos_y);
            ResetBlock();
            break;
        }
        //碰撞检测，只计算上下左右边界，先尝试走一格，如果碰撞则稳定方块后跳出
        if(IsCollide(block_pos.pos_x,block_pos.pos_y,DOWN))
        {
            //只有最终不能下落才转成稳定方块
            ConvertStable(block_pos.pos_x,block_pos.pos_y);
            ResetBlock();
            break;
        }
        //恢复方块上场景,为了清除移动过程中的方块残留
        for(int j=cur_border.lbound;j<=cur_border.rbound;j++)
            game_area[block_pos.pos_y][block_pos.pos_x+j]=0;
        //没有碰撞则下落一格
        block_pos.pos_y+=1;
        //方块下降一格，拷贝到场景,注意左右边界
        for(int i=0;i<4;i++) //必须是0到4而不是边界索引，考虑到旋转后边界重新计算
            for(int j=cur_border.lbound;j<=cur_border.rbound;j++)
                if(block_pos.pos_y+i<=AREA_ROW-1&&game_area[block_pos.pos_y+i][block_pos.pos_x+j]!=2) //注意场景数组不越界,而且不会擦出稳定的方块
                    game_area[block_pos.pos_y+i][block_pos.pos_x+j]=cur_block[i][j];
        break;
    case LEFT:
        //到左边界或者碰撞不再往左
        if(block_pos.pos_x+cur_border.lbound==0||IsCollide(block_pos.pos_x,block_pos.pos_y,LEFT))
            break;
        //恢复方块右场景,为了清除移动过程中的方块残留
        for(int i=cur_border.ubound;i<=cur_border.dbound;i++)
            game_area[block_pos.pos_y+i][block_pos.pos_x+3]=0;
        block_pos.pos_x-=1;
        //方块左移一格，拷贝到场景
        for(int i=cur_border.ubound;i<=cur_border.dbound;i++)
            for(int j=0;j<4;j++)
                if(block_pos.pos_x+j>=0&&game_area[block_pos.pos_y+i][block_pos.pos_x+j]!=2) //注意场景数组不越界
                    game_area[block_pos.pos_y+i][block_pos.pos_x+j]=cur_block[i][j];
        break;
    case RIGHT:
        if(block_pos.pos_x+cur_border.rbound==AREA_COL-1||IsCollide(block_pos.pos_x,block_pos.pos_y,RIGHT))
            break;
        //恢复方块左场景,为了清除移动过程中的方块残留
        for(int i=cur_border.ubound;i<=cur_border.dbound;i++)
            game_area[block_pos.pos_y+i][block_pos.pos_x]=0;
        block_pos.pos_x+=1;
        //方块右移一格，拷贝到场景
        for(int i=cur_border.ubound;i<=cur_border.dbound;i++)
            for(int j=0;j<4;j++)
                if(block_pos.pos_x+j<=AREA_COL-1&&game_area[block_pos.pos_y+i][block_pos.pos_x+j]!=2) //注意场景数组不越界
                    game_area[block_pos.pos_y+i][block_pos.pos_x+j]=cur_block[i][j];
        break;
    case SPACE: //一次到底
        //一格一格下移，直到不能下移
        while(block_pos.pos_y+cur_border.dbound<AREA_ROW-1&&!IsCollide(block_pos.pos_x,block_pos.pos_y,DOWN))
        {
            //恢复方块上场景,为了清除移动过程中的方块残留
            for(int j=cur_border.lbound;j<=cur_border.rbound;j++)
                game_area[block_pos.pos_y][block_pos.pos_x+j]=0;
            //没有碰撞则下落一格
            block_pos.pos_y+=1;
            //方块下降一格，拷贝到场景,注意左右边界
            for(int i=0;i<4;i++) //必须是0到4
                for(int j=cur_border.lbound;j<=cur_border.rbound;j++)
                    if(block_pos.pos_y+i<=AREA_ROW-1&&game_area[block_pos.pos_y+i][block_pos.pos_x+j]!=2) //注意场景数组不越界,而且不会擦出稳定的方块
                        game_area[block_pos.pos_y+i][block_pos.pos_x+j]=cur_block[i][j];
        }
        ConvertStable(block_pos.pos_x,block_pos.pos_y);
        ResetBlock();
        break;
    default:
        break;
    }
    //处理消行，整个场景上面的行依次下移
    int i=AREA_ROW-1;
    int line_count=0; //记消行数
    while(i>=1)
    {
        bool is_line_full=true;
        for(int j=0;j<AREA_COL;j++)
            if(game_area[i][j]==0)
            {
                is_line_full=false;
                i--;
                break;
            }
        if(is_line_full)
        {
            for(int k=i;k>=1;k--)
                for(int j=0;j<AREA_COL;j++)
                    game_area[k][j]=game_area[k-1][j];
            line_count++;//每次增加消行的行数
        }
    }
    score+=line_count*10; //得分
    //判断游戏是否结束
    for(int j=0;j<AREA_COL;j++)
        if(game_area[0][j]==2){ //最顶端也有稳定方块
            GameOver();
            mGameIsOver=true;
        }
}

void TetrisWindow::onDraw(Canvas&canvas){
    Window::onDraw(canvas);
    //画方块预告
    canvas.save();
    canvas.translate(mTopLeft,0);
    canvas.set_color(0xFFFFFFFF);
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            if(next_block[i][j]==1){
                canvas.rectangle(MARGIN*3+AREA_COL*BLOCK_SIZE+j*BLOCK_SIZE,MARGIN+i*BLOCK_SIZE,BLOCK_SIZE-0.2,BLOCK_SIZE-0.2);
                canvas.fill();
            } 
    //绘制得分
    RECT rect;
    rect.set(MARGIN*5+AREA_COL*BLOCK_SIZE,MARGIN*2+4*BLOCK_SIZE,BLOCK_SIZE*4,BLOCK_SIZE*4);
    canvas.set_font_size(26);
    canvas.draw_text(rect,std::string("Score: ")+std::to_string(score),DT_LEFT|DT_VCENTER);


    //绘制下落方块和稳定方块,注意方块边线的颜色是根据setPen来的，默认黑色
    for(int i=0;i<AREA_ROW;i++)
        for(int j=0;j<AREA_COL;j++)
        {
            //绘制活动方块
            if(game_area[i][j]==1){
                canvas.set_color(0xFFFF0000);
                canvas.rectangle(j*BLOCK_SIZE+MARGIN,i*BLOCK_SIZE+MARGIN,BLOCK_SIZE-0.2,BLOCK_SIZE-0.2);
            }
            //绘制稳定方块
            else if(game_area[i][j]==2){
                canvas.set_color(0xFFFFFFFF);
                canvas.rectangle(j*BLOCK_SIZE+MARGIN,i*BLOCK_SIZE+MARGIN,BLOCK_SIZE-0.2,BLOCK_SIZE-0.2);
            }
            canvas.fill();
        }
    canvas.set_color(0xFFFFFFFF);
    canvas.rectangle(MARGIN,MARGIN,AREA_COL*BLOCK_SIZE+MARGIN,AREA_ROW*BLOCK_SIZE+MARGIN);
    canvas.stroke();
    canvas.restore();
}

Window*CreateTetrisWindow(){
    Window*w=new TetrisWindow(0,0,1280,720);
};
}//namespace
#if 10
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=newglee::CreateTetrisWindow();
    app.exec();
}
#endif
