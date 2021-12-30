#include <core/path.h>
#include <cairomm/surface.h>
#include <cairomm/context.h>
#include <map>
namespace cdroid{

Path::Path(){
    mCTX=Cairo::Context::create(Cairo::ImageSurface::create(Cairo::Surface::Format::A8,1,1));
}

Path::Path(const Path&o):Path(){
    o.append_to_context(mCTX);
}

void Path::reset(){
    mCTX->begin_new_path();
}

void Path::begin_new_sub_path(){
    mCTX->begin_new_sub_path();
}

void Path::close_path(){
    mCTX->close_path();
}

void Path::append_to_context(Cairo::Context*to)const{
    const Cairo::RefPtr<Cairo::Path>from=Cairo::make_refptr_for_instance<Cairo::Path>(mCTX->copy_path());
    to->append_path(*from);
}

void Path::append_to_context(const Cairo::RefPtr<Cairo::Context>&to)const{
    Cairo::RefPtr<Cairo::Path>from=Cairo::make_refptr_for_instance<Cairo::Path>(mCTX->copy_path()); 
    to->append_path(*from);
}

void Path::set_fill_rule(Cairo::Context::FillRule fill_rule){
    mCTX->set_fill_rule(fill_rule);
}

void Path::move_to(double x,double y){
    mCTX->move_to(x,y);
}

void Path::line_to(double x,double y){
    mCTX->line_to(x,y);
}

void Path::curve_to(double x1, double y1, double x2, double y2, double x3, double y3){
    mCTX->curve_to(x1,y1,x2,y2,x3,y3);
}

void Path::arc(double xc, double yc, double radius, double angle1, double angle2){
    mCTX->arc(xc,yc,radius,angle1,angle2);
}

void Path::rectangle(double x, double y, double width, double height){
    mCTX->rectangle(x,y,width,height);
}

void Path::round_rectangle(double x,double y,double width,double height,const std::vector<float>& radii){
    //const RectF rect={x,y,width,height};
    round_rectangle({float(x),float(y),float(width),float(height)},radii);
}
void Path::round_rectangle(const RectF&rect,const std::vector<float>& radii){
    constexpr double circleControlPoint=0.447715;
    move_to(rect.left+radii[0],rect.top);
    line_to(rect.right()-radii[2],rect.top);
    if((radii[2]>0||radii[3])){//topright
         curve_to(rect.right()-radii[2]*circleControlPoint,rect.top,
             rect.right(),rect.top+radii[3]*circleControlPoint,
             rect.right(),rect.top+radii[3]); 
    }
    line_to(rect.right(),rect.bottom()-radii[5]);

    if(radii[4]>0||radii[5]){//bottomright 
         curve_to(rect.right(),rect.bottom()-radii[5]*circleControlPoint,
             rect.right()-radii[4]*circleControlPoint, rect.bottom(),
             rect.right()-radii[4],rect.bottom());
    }
    line_to(rect.left+radii[4],rect.bottom());

    if(radii[6]>0||radii[7]>0){//bottomleft
        curve_to(rect.left+radii[6]*circleControlPoint,rect.bottom(),
            rect.left,rect.bottom()-radii[7]*circleControlPoint,
            rect.left,rect.bottom()-radii[7]);
    }
    line_to(rect.left,rect.top+radii[7]);

    if(radii[0]>0||radii[1]>0){//topleft
        curve_to(rect.left,rect.top+radii[1]*circleControlPoint,
           rect.left+radii[0]*circleControlPoint,rect.top,
           rect.left+radii[0],rect.top);
    }
    mCTX->close_path();//closeSubpath();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
typedef PointF (*BezierCalculation)(float t, const PointF* points);
static void addMove(std::vector<PointF>&segmentPoints,std::vector<float>&lengths,PointF&toPoint){
    const float length=lengths.size()?lengths.back():.0f;
    segmentPoints.push_back(toPoint);
    lengths.push_back(length); 
}

static void addLine(std::vector<PointF>&segmentPoints,std::vector<float>&lengths,PointF&toPoint){
    if(segmentPoints.size()==0)
        segmentPoints.push_back(PointF{0,0});

    PointF& fpt=segmentPoints.back();
    const float dx=fpt.x-toPoint.x;
    const float dy=fpt.y-toPoint.y;

    if(dx==.0&&dy==.0f)  return ;
    float length=lengths.back()+sqrt(dx*dx+dy*dy);
    segmentPoints.push_back(toPoint);
    lengths.push_back(length);
}

static float cubicCoordinateCalculation(float t, float p0, float p1, float p2, float p3) {
    float oneMinusT = 1 - t;
    float oneMinusTSquared = oneMinusT * oneMinusT;
    float oneMinusTCubed = oneMinusTSquared * oneMinusT;
    float tSquared = t * t;
    float tCubed = tSquared * t;
    return (oneMinusTCubed * p0) + (3 * oneMinusTSquared * t * p1)
             + (3 * oneMinusT * tSquared * p2) + (tCubed * p3);
}

static PointF cubicBezierCalculation(float t, const PointF* points) {
    PointF pt;
    pt.x = cubicCoordinateCalculation(t, points[0].x, points[1].x,
           points[2].x, points[3].x);
    pt.y = cubicCoordinateCalculation(t, points[0].y, points[1].y,
           points[2].y, points[3].y);
    return pt;
}

static bool subdividePoints(const PointF* points, BezierCalculation bezierFunction,
       float t0, const PointF &p0, float t1, const PointF &p1,
       float& midT, PointF &midPoint, float errorSquared) {
     midT = (t1 + t0) / 2;
     float midX = (p1.x + p0.x) / 2;
     float midY = (p1.y + p0.y) / 2;
     midPoint = (*bezierFunction)(midT, points);
     float xError = midPoint.x - midX;
     float yError = midPoint.y - midY;
     float midErrorSquared = (xError * xError) + (yError * yError);
     return midErrorSquared > errorSquared;
}
static void addBezier(PointF*points,BezierCalculation bezierFunction,std::vector<PointF>&segmentPoints,
     std::vector<float>&lengths,float errorSquared,float doubleCheckDivision){
    typedef std::map<float,PointF>PointMap;
    PointMap tToPoint;
    tToPoint[0]=bezierFunction(0,points);
    tToPoint[1]=bezierFunction(1,points);
    PointMap::iterator iter=tToPoint.begin();
    PointMap::iterator next=iter;
    ++next;
    while(next!=tToPoint.end()){
        bool needsSubdivision = true;
        PointF midPoint;
        do {
            float midT;
            needsSubdivision = subdividePoints(points, bezierFunction, iter->first,
                iter->second, next->first, next->second, midT, midPoint, errorSquared);
            if (!needsSubdivision && doubleCheckDivision) {
                PointF quarterPoint;
                float quarterT;
                needsSubdivision = subdividePoints(points, bezierFunction, iter->first,
                    iter->second, midT, midPoint, quarterT, quarterPoint, errorSquared);
                if (needsSubdivision) {
                    // Found an inflection point. No need to double-check.
                    doubleCheckDivision = false;
                }
            }
            if (needsSubdivision) {
                next = tToPoint.insert(iter, PointMap::value_type(midT, midPoint));
            }
        } while (needsSubdivision);
        iter = next;
        next++;        
    }

    // Now that each division can use linear interpolation with less than the allowed error
    for (iter = tToPoint.begin(); iter != tToPoint.end(); ++iter) {
       addLine(segmentPoints, lengths, iter->second);
    }
}

static void createVerbSegments(cairo_path_data_t*data,PointF*points,std::vector<PointF>&segmentPoints,
            std::vector<float>&lengths,float errorSquared,float errorConic){
    switch(data->header.type){
    case CAIRO_PATH_MOVE_TO: addMove(segmentPoints,lengths,points[0]);break;
    case CAIRO_PATH_LINE_TO: addLine(segmentPoints,lengths,points[1]);break;
    case CAIRO_PATH_CURVE_TO:
         addBezier(points,cubicBezierCalculation,segmentPoints,lengths,errorSquared,errorConic);
         break;
    case CAIRO_PATH_CLOSE_PATH:addLine(segmentPoints,lengths,points[0]);break;
    }
}

void Path::approximate(std::vector<float>&,float acceptableError){
    PointF points[4];
    std::vector<PointF>segmentPoints;
    std::vector<float>lengths;
    float errorSquared=acceptableError*acceptableError;
    float errorConic =acceptableError/2.f;
    cairo_path_t*path=cairo_copy_path(mCTX->cobj());
    for (int i=0; i < path->num_data; i += path->data[i].header.length){
        cairo_path_data_t *data=path->data+i;
        createVerbSegments(data,points,segmentPoints,lengths,errorSquared,errorConic);
    }
    cairo_path_destroy(path);

    if (segmentPoints.empty()) {
        int numVerbs = path->num_data;//path->countVerbs();
        PointF pt={0,0};
        if (numVerbs == 1) {
            pt.x=path->data[1].point.x;
            pt.y=path->data[1].point.y;
            addMove(segmentPoints, lengths, pt);//path->getPoint(0));
        } else {
            // Invalid or empty path. Fall back to point(0,0)
            addMove(segmentPoints, lengths, pt);
        }
    }

    float totalLength = lengths.back();
    if (totalLength == 0) {
        // Lone Move instructions should still be able to animate at the same value.
        segmentPoints.push_back(segmentPoints.back());
        lengths.push_back(1);
        totalLength = 1;
    }

    size_t numPoints = segmentPoints.size();
    size_t approximationArraySize = numPoints * 3;

    float* approximation = new float[approximationArraySize];
    int approximationIndex = 0;
    for (size_t i = 0; i < numPoints; i++) {
        const PointF& point = segmentPoints[i];
        approximation[approximationIndex++] = lengths[i] / totalLength;
        approximation[approximationIndex++] = point.x;
        approximation[approximationIndex++] = point.y;
    }    
}

}
