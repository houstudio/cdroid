#include <core/transform.h>
#include <porting/cdlog.h>
#include <iomanip>
namespace cdroid::ui {

Transform::Transform() {
    reset();
}

Transform::Transform(const Transform&  other)
    : mMatrix(other.mMatrix), mType(other.mType) {
}

Transform::Transform(uint32_t orientation, int w, int h) {
    set(orientation, w, h);
}

Transform::~Transform() = default;

static const float EPSILON = 0.0f;

bool Transform::isZero(float f) {
    return fabs(f) <= EPSILON;
}

bool Transform::absIsOne(float f) {
    return isZero(fabs(f) - 1.0f);
}

bool Transform::operator==(const Transform& other) const {
    return mMatrix[0][0] == other.mMatrix[0][0] && mMatrix[0][1] == other.mMatrix[0][1] &&
            mMatrix[0][2] == other.mMatrix[0][2] && mMatrix[1][0] == other.mMatrix[1][0] &&
            mMatrix[1][1] == other.mMatrix[1][1] && mMatrix[1][2] == other.mMatrix[1][2] &&
            mMatrix[2][0] == other.mMatrix[2][0] && mMatrix[2][1] == other.mMatrix[2][1] &&
            mMatrix[2][2] == other.mMatrix[2][2];
}

Transform Transform::operator*(const Transform& rhs) const {
    if (/*CC_LIKELY*/(mType == IDENTITY))
        return rhs;

    Transform r(*this);
    if (rhs.mType == IDENTITY)
        return r;

    // TODO: we could use mType to optimize the matrix multiply
    const mat33& A(mMatrix);
    const mat33& B(rhs.mMatrix);
          mat33& D(r.mMatrix);
    for (size_t i = 0; i < 3; i++) {
        const float v0 = A[0][i];
        const float v1 = A[1][i];
        const float v2 = A[2][i];
        D[0][i] = v0*B[0][0] + v1*B[0][1] + v2*B[0][2];
        D[1][i] = v0*B[1][0] + v1*B[1][1] + v2*B[1][2];
        D[2][i] = v0*B[2][0] + v1*B[2][1] + v2*B[2][2];
    }
    r.mType |= rhs.mType;

    // TODO: we could recompute this value from r and rhs
    r.mType &= 0xFF;
    r.mType |= UNKNOWN_TYPE;
    return r;
}

Transform Transform::operator * (float value) const {
    Transform r(*this);
    const mat33& M(mMatrix);
    mat33& R(r.mMatrix);
    for (size_t i = 0; i < 3; i++) {
        for (size_t j = 0; j < 2; j++) {
            R[i][j] = M[i][j] * value;
        }
    }
    r.type();
    return r;
}

Transform& Transform::operator=(const Transform& other) {
    mMatrix = other.mMatrix;
    mType = other.mType;
    return *this;
}

const vec3& Transform::operator [] (size_t i) const {
    return mMatrix[i];
}

// x translate
float Transform::tx() const {
    return mMatrix[2][0];
}

// y translate
float Transform::ty() const {
    return mMatrix[2][1];
}

float Transform::dsdx() const {
    return mMatrix[0][0];
}

float Transform::dtdx() const {
    return mMatrix[1][0];
}

float Transform::dtdy() const {
    return mMatrix[0][1];
}

float Transform::dsdy() const {
    return mMatrix[1][1];
}

float Transform::det() const {
    return mMatrix[0][0] * mMatrix[1][1] - mMatrix[0][1] * mMatrix[1][0];
}

float Transform::getScaleX() const {
    return sqrt((dsdx() * dsdx()) + (dtdx() * dtdx()));
}

float Transform::getScaleY() const {
    return sqrt((dtdy() * dtdy()) + (dsdy() * dsdy()));
}

void Transform::reset() {
    mType = IDENTITY;
    for(size_t i = 0; i < 3; i++) {
        vec3& v(mMatrix[i]);
        for (size_t j = 0; j < 3; j++)
            v[j] = ((i == j) ? 1.0f : 0.0f);
    }
}

void Transform::set(float tx, float ty) {
    mMatrix[2][0] = tx;
    mMatrix[2][1] = ty;
    mMatrix[2][2] = 1.0f;

    if (isZero(tx) && isZero(ty)) {
        mType &= ~TRANSLATE;
    } else {
        mType |= TRANSLATE;
    }
}

// x and y are the coordinates in the destination (i.e. the screen)
// s and t are the coordinates in the source (i.e. the texture)
// d means derivative
// dsdx means ds/dx derivative of s with respect to x, etc.
void Transform::set(float dsdx, float dtdy, float dtdx, float dsdy) {
    mat33& M(mMatrix);
    M[0][0] = dsdx;    M[1][0] = dtdy;
    M[0][1] = dtdx;    M[1][1] = dsdy;
    M[0][2] = 0;       M[1][2] = 0;
    mType = UNKNOWN_TYPE;
}

status_t Transform::set(uint32_t flags, float w, float h) {
    if (flags & ROT_INVALID) {
        // that's not allowed!
        reset();
        return -1/*BAD_VALUE*/;
    }

    Transform H, V, R;
    if (flags & ROT_90) {
        // w & h are inverted when rotating by 90 degrees
        std::swap(w, h);
    }

    if (flags & FLIP_H) {
        H.mType = (FLIP_H << 8) | SCALE;
        H.mType |= isZero(w) ? IDENTITY : TRANSLATE;
        mat33& M(H.mMatrix);
        M[0][0] = -1;
        M[2][0] = w;
    }

    if (flags & FLIP_V) {
        V.mType = (FLIP_V << 8) | SCALE;
        V.mType |= isZero(h) ? IDENTITY : TRANSLATE;
        mat33& M(V.mMatrix);
        M[1][1] = -1;
        M[2][1] = h;
    }

    if (flags & ROT_90) {
        const float original_w = h;
        R.mType = (ROT_90 << 8) | ROTATE;
        R.mType |= isZero(original_w) ? IDENTITY : TRANSLATE;
        mat33& M(R.mMatrix);
        M[0][0] = 0;    M[1][0] =-1;    M[2][0] = original_w;
        M[0][1] = 1;    M[1][1] = 0;
    }

    *this = (R*(H*V));
    return 0/*NO_ERROR*/;
}

void Transform::set(const std::array<float, 9>& matrix) {
    mat33& M(mMatrix);
    M[0][0] = matrix[0];  M[1][0] = matrix[1];  M[2][0] = matrix[2];
    M[0][1] = matrix[3];  M[1][1] = matrix[4];  M[2][1] = matrix[5];
    M[0][2] = matrix[6];  M[1][2] = matrix[7];  M[2][2] = matrix[8];
    mType = UNKNOWN_TYPE;
    type();
}

vec2 Transform::transform(const vec2& v) const {
    vec2 r;
    const mat33& M(mMatrix);
    r[0] = M[0][0]*v[0] + M[1][0]*v[1] + M[2][0];
    r[1] = M[0][1]*v[0] + M[1][1]*v[1] + M[2][1];
    return r;
}

vec3 Transform::transform(const vec3& v) const {
    vec3 r;
    const mat33& M(mMatrix);
    r[0] = M[0][0]*v[0] + M[1][0]*v[1] + M[2][0]*v[2];
    r[1] = M[0][1]*v[0] + M[1][1]*v[1] + M[2][1]*v[2];
    r[2] = M[0][2]*v[0] + M[1][2]*v[1] + M[2][2]*v[2];
    return r;
}

vec2 Transform::transform(float x, float y) const {
    vec2 v2={x,y};
    return transform(v2);//vec2(x, y));
}

#if 0
Rect Transform::makeBounds(int w, int h) const {
    return transform( Rect(w, h) );
}

Rect Transform::transform(const Rect& bounds, bool roundOutwards) const {
    Rect r;
    vec2 lt( bounds.left,  bounds.top    );
    vec2 rt( bounds.right, bounds.top    );
    vec2 lb( bounds.left,  bounds.bottom );
    vec2 rb( bounds.right, bounds.bottom );

    lt = transform(lt);
    rt = transform(rt);
    lb = transform(lb);
    rb = transform(rb);

    if (roundOutwards) {
        r.left   = static_cast<int32_t>(floorf(std::min({lt[0], rt[0], lb[0], rb[0]})));
        r.top    = static_cast<int32_t>(floorf(std::min({lt[1], rt[1], lb[1], rb[1]})));
        r.right  = static_cast<int32_t>(ceilf(std::max({lt[0], rt[0], lb[0], rb[0]})));
        r.bottom = static_cast<int32_t>(ceilf(std::max({lt[1], rt[1], lb[1], rb[1]})));
    } else {
        r.left   = static_cast<int32_t>(floorf(std::min({lt[0], rt[0], lb[0], rb[0]}) + 0.5f));
        r.top    = static_cast<int32_t>(floorf(std::min({lt[1], rt[1], lb[1], rb[1]}) + 0.5f));
        r.right  = static_cast<int32_t>(floorf(std::max({lt[0], rt[0], lb[0], rb[0]}) + 0.5f));
        r.bottom = static_cast<int32_t>(floorf(std::max({lt[1], rt[1], lb[1], rb[1]}) + 0.5f));
    }

    return r;
}

FloatRect Transform::transform(const FloatRect& bounds) const {
    vec2 lt(bounds.left, bounds.top);
    vec2 rt(bounds.right, bounds.top);
    vec2 lb(bounds.left, bounds.bottom);
    vec2 rb(bounds.right, bounds.bottom);

    lt = transform(lt);
    rt = transform(rt);
    lb = transform(lb);
    rb = transform(rb);

    FloatRect r;
    r.left = std::min({lt[0], rt[0], lb[0], rb[0]});
    r.top = std::min({lt[1], rt[1], lb[1], rb[1]});
    r.right = std::max({lt[0], rt[0], lb[0], rb[0]});
    r.bottom = std::max({lt[1], rt[1], lb[1], rb[1]});

    return r;
}

Region Transform::transform(const Region& reg) const {
    Region out;
    if (CC_UNLIKELY(type() > TRANSLATE)) {
        if (CC_LIKELY(preserveRects())) {
            Region::const_iterator it = reg.begin();
            Region::const_iterator const end = reg.end();
            while (it != end) {
                out.orSelf(transform(*it++));
            }
        } else {
            out.set(transform(reg.bounds()));
        }
    } else {
        int xpos = static_cast<int>(floorf(tx() + 0.5f));
        int ypos = static_cast<int>(floorf(ty() + 0.5f));
        out = reg.translate(xpos, ypos);
    }
    return out;
}
#endif

uint32_t Transform::type() const {
    if (mType & UNKNOWN_TYPE) {
        // recompute what this transform is

        const mat33& M(mMatrix);
        const float a = M[0][0];
        const float b = M[1][0];
        const float c = M[0][1];
        const float d = M[1][1];
        const float x = M[2][0];
        const float y = M[2][1];

        bool scale = false;
        uint32_t flags = ROT_0;
        if (isZero(b) && isZero(c)) {
            if (a<0)    flags |= FLIP_H;
            if (d<0)    flags |= FLIP_V;
            if (!absIsOne(a) || !absIsOne(d)) {
                scale = true;
            }
        } else if (isZero(a) && isZero(d)) {
            flags |= ROT_90;
            if (b>0)    flags |= FLIP_V;
            if (c<0)    flags |= FLIP_H;
            if (!absIsOne(b) || !absIsOne(c)) {
                scale = true;
            }
        } else {
            // there is a skew component and/or a non 90 degrees rotation
            flags = ROT_INVALID;
        }

        mType = flags << 8;
        if (flags & ROT_INVALID) {
            mType |= UNKNOWN;
        } else {
            if ((flags & ROT_90) || ((flags & ROT_180) == ROT_180))
                mType |= ROTATE;
            if (flags & FLIP_H)
                mType ^= SCALE;
            if (flags & FLIP_V)
                mType ^= SCALE;
            if (scale)
                mType |= SCALE;
        }

        if (!isZero(x) || !isZero(y))
            mType |= TRANSLATE;
    }
    return mType;
}

Transform Transform::inverse() const {
    // our 3x3 matrix is always of the form of a 2x2 transformation
    // followed by a translation: T*M, therefore:
    // (T*M)^-1 = M^-1 * T^-1
    Transform result;
    if (mType <= TRANSLATE) {
        // 1 0 0
        // 0 1 0
        // x y 1
        result = *this;
        result.mMatrix[2][0] = -result.mMatrix[2][0];
        result.mMatrix[2][1] = -result.mMatrix[2][1];
    } else {
        // a c 0
        // b d 0
        // x y 1
        const mat33& M(mMatrix);
        const float a = M[0][0];
        const float b = M[1][0];
        const float c = M[0][1];
        const float d = M[1][1];
        const float x = M[2][0];
        const float y = M[2][1];

        const float idet = 1.0f / det();
        result.mMatrix[0][0] =  d*idet;
        result.mMatrix[0][1] = -c*idet;
        result.mMatrix[1][0] = -b*idet;
        result.mMatrix[1][1] =  a*idet;
        result.mType = mType;
        if (getOrientation() & ROT_90) {
            // Recalculate the type if there is a 90-degree rotation component, since the inverse
            // of ROT_90 is ROT_270 and vice versa.
            result.mType |= UNKNOWN_TYPE;
        }

        vec2 T={-x,-y};//(-x, -y);
        T = result.transform(T);
        result.mMatrix[2][0] = T[0];
        result.mMatrix[2][1] = T[1];
    }
    return result;
}

uint32_t Transform::getType() const {
    return type() & 0xFF;
}

uint32_t Transform::getOrientation() const {
    return (type() >> 8) & 0xFF;
}

bool Transform::preserveRects() const {
    return (getOrientation() & ROT_INVALID) ? false : true;
}

bool Transform::needsBilinearFiltering() const {
    return (!preserveRects() || getType() >= ui::Transform::SCALE);
}

#if 0
mat4 Transform::asMatrix4() const {
    // Internally Transform uses a 3x3 matrix since the transform is meant for
    // two-dimensional values. An equivalent 4x4 matrix means inserting an extra
    // row and column which adds as an identity transform on the third
    // dimension.

    mat4 m = mat4{mat4::NO_INIT}; // NO_INIT since we explicitly set every element

    m[0][0] = mMatrix[0][0];
    m[0][1] = mMatrix[0][1];
    m[0][2] = 0.f;
    m[0][3] = mMatrix[0][2];

    m[1][0] = mMatrix[1][0];
    m[1][1] = mMatrix[1][1];
    m[1][2] = 0.f;
    m[1][3] = mMatrix[1][2];

    m[2][0] = 0.f;
    m[2][1] = 0.f;
    m[2][2] = 1.f;
    m[2][3] = 0.f;

    m[3][0] = mMatrix[2][0];
    m[3][1] = mMatrix[2][1];
    m[3][2] = 0.f;
    m[3][3] = mMatrix[2][2];

    return m;
}
#endif

static std::string rotationToString(const uint32_t rotationFlags) {
    switch (rotationFlags) {
        case Transform::ROT_0:     return "ROT_0";
        case Transform::FLIP_H:    return "FLIP_H";
        case Transform::FLIP_V:    return "FLIP_V";
        case Transform::ROT_90:    return "ROT_90";
        case Transform::ROT_180:   return "ROT_180";
        case Transform::ROT_270:   return "ROT_270";
        case Transform::ROT_INVALID:
        default:            return "ROT_INVALID";
    }
}

static std::string transformToString(const uint32_t transform) {
    if (transform == Transform::IDENTITY) {
        return "IDENTITY";
    }

    if (transform == Transform::UNKNOWN) {
        return "UNKNOWN";
    }

    std::string out;
    if (transform & Transform::SCALE) out.append("SCALE ");
    if (transform & Transform::ROTATE) out.append("ROTATE ");
    if (transform & Transform::TRANSLATE) out.append("TRANSLATE");
    return out;
}

void Transform::dump(std::ostream& out, const char* name, const char* prefix) const {

    type(); // Ensure the information in mType is up to date

    const uint32_t type = mType;
    const uint32_t orient = type >> 8;

    out << prefix<<name<<" ";
    if (orient & ROT_INVALID) {
         out << "0x" << std::hex << std::setw(8) << std::setfill('0') << orient << std::dec << " ";
    }
    out << "(" << rotationToString(orient) << ") ";

    if (type & UNKNOWN) {
        out << "0x" << std::hex << std::setw(2) << std::setfill('0') << type << std::dec << " ";
    }
    out << "(" << transformToString(type) << ")\n";

    if (type == IDENTITY) {
        return;
    }

    for (size_t i = 0; i < 3; i++) {
         out << prefix << "    " 
                << std::fixed << std::setprecision(4) << static_cast<double>(mMatrix[0][i]) << "  "
                << std::fixed << std::setprecision(4) << static_cast<double>(mMatrix[1][i]) << "  "
                << std::fixed << std::setprecision(4) << static_cast<double>(mMatrix[2][i]) << std::endl;
    }
}

void Transform::dump(const char* name, const char* prefix) const {
    std::ostringstream out;
    dump(out, name, prefix);
    LOG(DEBUG)<< out.str();
}

} /*namespace cdroid::ui*/
