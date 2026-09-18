#include <app/espresso/generallocation.h>

#include <core/rect.h>
#include <view/view.h>

namespace cdroid {
namespace espresso {

namespace {

/** The AOSP enum bodies: one provider per (vertical, horizontal) Position pair. */
class GeneralLocationProvider : public CoordinatesProvider {
public:
    GeneralLocationProvider(GeneralLocation::Position vertical,
            GeneralLocation::Position horizontal, bool visiblePartOnly)
        : mVertical(vertical), mHorizontal(horizontal), mVisiblePartOnly(visiblePartOnly) {}

    FloatCoord calculateCoordinates(View& view) override {
        if (mVisiblePartOnly) {
            return GeneralLocation::getCoordinatesOfVisiblePart(view, mVertical, mHorizontal);
        }
        return GeneralLocation::getCoordinates(view, mVertical, mHorizontal);
    }

private:
    GeneralLocation::Position mVertical;
    GeneralLocation::Position mHorizontal;
    bool mVisiblePartOnly;
};

} // namespace

CoordinatesProviderPtr GeneralLocation::TOP_LEFT() {
    return std::make_shared<GeneralLocationProvider>(Position::BEGIN, Position::BEGIN, false);
}
CoordinatesProviderPtr GeneralLocation::TOP_CENTER() {
    return std::make_shared<GeneralLocationProvider>(Position::BEGIN, Position::MIDDLE, false);
}
CoordinatesProviderPtr GeneralLocation::TOP_RIGHT() {
    return std::make_shared<GeneralLocationProvider>(Position::BEGIN, Position::END, false);
}
CoordinatesProviderPtr GeneralLocation::CENTER_LEFT() {
    return std::make_shared<GeneralLocationProvider>(Position::MIDDLE, Position::BEGIN, false);
}
CoordinatesProviderPtr GeneralLocation::CENTER() {
    return std::make_shared<GeneralLocationProvider>(Position::MIDDLE, Position::MIDDLE, false);
}
CoordinatesProviderPtr GeneralLocation::CENTER_RIGHT() {
    return std::make_shared<GeneralLocationProvider>(Position::MIDDLE, Position::END, false);
}
CoordinatesProviderPtr GeneralLocation::BOTTOM_LEFT() {
    return std::make_shared<GeneralLocationProvider>(Position::END, Position::BEGIN, false);
}
CoordinatesProviderPtr GeneralLocation::BOTTOM_CENTER() {
    return std::make_shared<GeneralLocationProvider>(Position::END, Position::MIDDLE, false);
}
CoordinatesProviderPtr GeneralLocation::BOTTOM_RIGHT() {
    return std::make_shared<GeneralLocationProvider>(Position::END, Position::END, false);
}
CoordinatesProviderPtr GeneralLocation::VISIBLE_CENTER() {
    return std::make_shared<GeneralLocationProvider>(Position::MIDDLE, Position::MIDDLE, true);
}

CoordinatesProviderPtr GeneralLocation::translate(CoordinatesProviderPtr coords,
        float dx, float dy) {
    class TranslatedProvider : public CoordinatesProvider {
    public:
        TranslatedProvider(CoordinatesProviderPtr coords, float dx, float dy)
            : mCoords(std::move(coords)), mDx(dx), mDy(dy) {}
        FloatCoord calculateCoordinates(View& view) override {
            FloatCoord xy = mCoords->calculateCoordinates(view);
            xy[0] += mDx * view.getWidth();
            xy[1] += mDy * view.getHeight();
            return xy;
        }
    private:
        CoordinatesProviderPtr mCoords;
        float mDx;
        float mDy;
    };
    return std::make_shared<TranslatedProvider>(std::move(coords), dx, dy);
}

// static
float GeneralLocation::getPosition(Position position, int viewPos, int viewLength) {
    switch (position) {
        case Position::BEGIN:
            return viewPos;
        case Position::MIDDLE:
            // Midpoint between the leftmost and rightmost pixel (position viewLength - 1).
            return viewPos + (viewLength - 1) / 2.0f;
        case Position::END:
            return viewPos + viewLength - 1;
    }
    return viewPos;
}

// static
FloatCoord GeneralLocation::getCoordinates(View& view, Position vertical, Position horizontal) {
    int xy[2] = {0, 0};
    view.getLocationOnScreen(xy);
    return FloatCoord{getPosition(horizontal, xy[0], view.getWidth()),
            getPosition(vertical, xy[1], view.getHeight())};
}

// static
FloatCoord GeneralLocation::getCoordinatesOfVisiblePart(View& view, Position vertical,
        Position horizontal) {
    int xy[2] = {0, 0};
    view.getLocationOnScreen(xy);
    Rect visibleParts;
    view.getGlobalVisibleRect(visibleParts, nullptr);
    return FloatCoord{getPosition(horizontal, xy[0], visibleParts.width),
            getPosition(vertical, xy[1], visibleParts.height)};
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
