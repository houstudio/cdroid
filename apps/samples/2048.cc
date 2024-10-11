#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <cdroid.h>
#include <vector>
#include <sstream>
#include <cdlog.h>


class GameWindow :public Window {
  protected:
    uint16_t score;
    uint16_t scheme;
    uint16_t SIZE;
    std::vector<std::vector<uint32_t> >board;
    TextView*tfscore;
  public:
    GameWindow(int x,int y,int w,int h):Window(x,y,w,h) {
        /*tfscore=new TextView("0",200,40);
        tfscore->setTextSize(30);
        addView(tfscore).setPos(800,50);*/
    }
    void getColor(uint8_t value, char *color, size_t length);
    void initBoard(uint16_t size);
    void drawBoard(Canvas&canvas);
    uint8_t findTarget(std::vector<uint32_t> array,uint8_t x,uint8_t stop);
    bool slideArray(std::vector<uint32_t>& array);
    void rotateBoard();
    bool moveUp();
    bool moveDown();
    bool moveLeft();
    bool moveRight();
    bool findPairDown();
    uint8_t countEmpty();
    void addRandom();
    bool gameEnded();
    void setScore(int s);
    virtual bool onKeyUp(int ,KeyEvent&k)override;
    virtual void onDraw(Canvas&canvas)override {
        Window::onDraw(canvas);
        canvas.reset_clip();
        drawBoard(canvas);
    }
};

void GameWindow::getColor(uint8_t value, char *color, size_t length) {
    uint8_t original[] = {8,255,1,255,2,255,3,255,4,255,5,255,6,255,7,255,9,0,10,0,11,0,12,0,13,0,14,0,255,0,255,0};
    uint8_t blackwhite[] = {232,255,234,255,236,255,238,255,240,255,242,255,244,255,246,0,248,0,249,0,250,0,251,0,252,0,253,0,254,0,255,0};
    uint8_t bluered[] = {235,255,63,255,57,255,93,255,129,255,165,255,201,255,200,255,199,255,198,255,197,255,196,255,196,255,196,255,196,255,196,255};
    uint8_t *schemes[] = {original,blackwhite,bluered};
    uint8_t *background = schemes[scheme]+0;
    uint8_t *foreground = schemes[scheme]+1;
    if (value > 0) while (value--) {
        if (background+2<schemes[scheme]+sizeof(original)) {
            background+=2;
            foreground+=2;
        }
    }
    snprintf(color,length,"\033[38;5;%d;48;5;%dm",*foreground,*background);
}

static int getwb(int v) {
    int r=0;
    while(v) {
        r++;
        v/=10;
    }
    return r;
}
void GameWindow::drawBoard(Canvas&canvas) {
    uint8_t x,y;
    const int pow2[]= {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192};
    int CW=std::min(getWidth(),getHeight())/SIZE;
    for (y=0; y<SIZE; y++) {
        for (x=0; x<SIZE; x++) {
            char CC[8];
            canvas.set_color(board[x][y]?0xFFAAAAAA:0xFF222222);
            canvas.rectangle(x*CW,y*CW,CW,CW);
            canvas.fill();
            canvas.set_color(0xFFFFFFFF);
            if(board[x][y]) {
                RECT r= {x*CW,y*CW,CW,CW};
                int v=pow2[board[x][y]-1];
                canvas.set_font_size(CW/getwb(v));
                sprintf(CC,"%d",v);
                canvas.draw_text(r,CC,Gravity::CENTER);
            }
        }
    }
    canvas.set_color(0xFF00FF00);
    for(x=0; x<=SIZE; x++) {
        canvas.move_to(x*CW,0);
        canvas.line_to(x*CW,CW*SIZE);
    }
    for(y=0; y<=SIZE; y++) {
        canvas.move_to(0,y*CW);
        canvas.line_to(CW*SIZE,y*CW);
    }
    if(gameEnded())
        canvas.show_text("Game Over");
    canvas.stroke();
}

uint8_t GameWindow::findTarget(std::vector<uint32_t> array,uint8_t x,uint8_t stop) {
    uint8_t t;
    if (x==0) {// if the position is already on the first, don't evaluate
        return x;
    }
    for(t=x-1;; t--) {
        if (array[t]!=0) {
            if (array[t]!=array[x]) {// merge is not possible, take next position
                return t+1;
            }
            return t;
        } else { // we should not slide further, return this one
            if (t==stop) {
                return t;
            }
        }
    }
    // we did not find a
    return x;
}

bool GameWindow::slideArray(std::vector<uint32_t>& array) {
    bool success = false;
    uint32_t x,t,stop=0;

    for (x=0; x<SIZE; x++) {
        if (array[x]!=0) {
            t = findTarget(array,x,stop);
            // if target is not original position, then move or merge
            if (t!=x) {
                // if target is zero, this is a move
                if (array[t]==0) {
                    array[t]=array[x];
                } else if (array[t]==array[x]) {
                    // merge (increase power of two)
                    array[t]++;
                    // increase score
                    score+=(uint32_t)1<<array[t];
                    setScore(score);
                    // set stop to avoid double merge
                    stop = t+1;
                }
                array[x]=0;
                success = true;
            }
        }
    }
    return success;
}

void GameWindow::rotateBoard() {
    uint8_t i,j,n=SIZE;
    uint32_t tmp;
    for (i=0; i<n/2; i++) {
        for (j=i; j<n-i-1; j++) {
            tmp = board[i][j];
            board[i][j] = board[j][n-i-1];
            board[j][n-i-1] = board[n-i-1][n-j-1];
            board[n-i-1][n-j-1] = board[n-j-1][i];
            board[n-j-1][i] = tmp;
        }
    }
}

bool GameWindow::moveUp() {
    bool success = false;
    uint8_t x;
    for (x=0; x<SIZE; x++) {
        bool slid= slideArray(board[x]);
        success |= slid;
        LOGV_IF(slid,"slide[%d]",x); 
    }
    return success;
}

bool GameWindow::moveLeft() {
    bool success;
    rotateBoard();
    success = moveUp();
    rotateBoard();
    rotateBoard();
    rotateBoard();
    return success;
}

bool GameWindow::moveDown() {
    bool success;
    rotateBoard();
    rotateBoard();
    success = moveUp();
    rotateBoard();
    rotateBoard();
    return success;
}

bool GameWindow::moveRight() {
    bool success;
    rotateBoard();
    rotateBoard();
    rotateBoard();
    success = moveUp();
    rotateBoard();
    return success;
}

bool GameWindow::findPairDown() {
    bool success = false;
    uint8_t x,y;
    for (x=0; x<SIZE; x++) {
        for (y=0; y<SIZE-1; y++) {
            if (board[x][y]==board[x][y+1]) return true;
        }
    }
    return success;
}

uint8_t GameWindow::countEmpty() {
    uint8_t x,y;
    uint8_t count=0;
    for (x=0; x<SIZE; x++) {
        for (y=0; y<SIZE; y++) {
            if (board[x][y]==0) {
                count++;
            }
        }
    }
    return count;
}

bool GameWindow::gameEnded() {
    bool ended = true;
    if (countEmpty()>0) return false;
    if (findPairDown()) return false;
    rotateBoard();
    if (findPairDown()) ended = false;
    rotateBoard();
    rotateBoard();
    rotateBoard();
    return ended;
}

void GameWindow::addRandom() {
    static bool initialized = false;
    uint8_t x,y;
    uint8_t r,len=0;
    uint8_t n,list[SIZE*SIZE][2];

    if (!initialized) {
        srand(time(NULL));
        initialized = true;
    }

    for (x=0; x<SIZE; x++) {
        for (y=0; y<SIZE; y++) {
            if (board[x][y]==0) {
                list[len][0]=x;
                list[len][1]=y;
                len++;
            }
        }
    }

    if (len>0) {
        r = rand()%len;
        x = list[r][0];
        y = list[r][1];
        n = (rand()%10)/9+1;
        board[x][y]=n;
    }
    invalidate(NULL);
}
void GameWindow::setScore(int s) {
    char  buf[64];
    sprintf(buf,"SCORE:%d",s);
    score=s;
    //tfscore->setText(buf);
}
void GameWindow::initBoard(uint16_t size) {
    uint16_t x,y;
    board.resize(size);
    for (x=0; x<size; x++) {
        board[x].resize(size);
        for (y=0; y<size; y++) {
            board[x][y]=0;
        }
    }
    SIZE=size;
    addRandom();
    addRandom();
    setScore(0);
    //drawBoard();
}
bool GameWindow::onKeyUp(int keyCode,KeyEvent&k) {
    bool success=false;
    switch(keyCode) {
    case KeyEvent::KEYCODE_DPAD_UP   :
        success=moveUp();
        break;
    case KeyEvent::KEYCODE_DPAD_DOWN :
        success=moveDown();
        break;
    case KeyEvent::KEYCODE_DPAD_LEFT :
        success=moveLeft();
        break;
    case KeyEvent::KEYCODE_DPAD_RIGHT:
        success=moveRight();
        break;
    case KeyEvent::KEYCODE_ENTER:
        if(gameEnded()){
	    setScore(0);
	    initBoard(4);
	    addRandom();
	}break;
    default:   break;
    }
    if(success){
        addRandom();
        if(gameEnded()) {
            Toast::makeText(getContext(),"Game Over! Press OK to Restart");
        }
        return true;
    }
    return false;
}

int main(int argc, const char *argv[]) {
    App app(argc,argv);
    GameWindow*gw=new GameWindow(200,0,1080,720);
    gw->initBoard(4);
    return app.exec();

}
