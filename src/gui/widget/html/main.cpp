#include <cdroid.h>
#include <htmlview.h>
#include <litehtml/url.h>

static const char*htmlbody=R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>小侯不摆烂</title>
</head>
<style>
background-image: url(file:///home/houzh/images/8.jpg);
background-position: center center;
background-repeat: no-repeat;
background-attachment: fixed;
background-size: cover;
background-color: #464646;
</style>
<body style="background-image:url(assets://cdroid@mipmap/btn_radio_on);">
    <h4>我爱吃的零食:</h4>
    <ul>
        <li>薯片</li>
        <li>汉堡</li>
        <li>鸡腿</li>
    </ul>
    <a href=http://www.sina.com.cn>ClickME</a>
    <h4>我爱吃的水果的排序</h4>
    <ol>
        <li>橘子</li>
        <li>苹果</li>
        <li>火龙果</li>
    </ol>
    <h4>自定义列表</h4>
    <dl>
        <dt>乐队名称</dt>
        <dd>黑屋乐队</dd>
        <dd>对角巷乐队</dd>
        <dd>时间不够以后乐队</dd>
        <dd>蛙池乐队</dd>
        <dt>成员</dt>
        <dd>王泽南</dd>
        <dd>郭玉都</dd>
        <dd>李巧巧</dd>
    </dl>
    <img src="file:///home/houzh/images/8.jpg/>
</body>
</html>
)";

int main (int argc, char *argv[]){
    App app;
    litehtml::url url1("assets","cdroid@mipmap","/test.png","","");
    LOGD("scheme=%s auth=%s path=%s",url1.scheme().c_str(),url1.authority().c_str(),url1.path().c_str());
    LOGD("url1=%s  path=%s",url1.string().c_str() ,(url1.authority()+url1.path()).c_str());
    litehtml::url url("assets://cdroid@mipmap/aaa.png");
    LOGD("scheme=%s path=%s",url.scheme().c_str(),url.path().c_str());
    Window*w=new Window(0,0,-1,-1);
    cdroid::HtmlView*html=new cdroid::HtmlView(400,400);
    w->addView(html);
    if(app.getParamCount())
        html->open_page(app.getParam(0,htmlbody),"");
    else 
	html->open_page(htmlbody,"");
    return app.exec();
}
