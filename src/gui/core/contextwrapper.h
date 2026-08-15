#ifndef __CONTEXT_WRAPPER_H__
#define __CONTEXT_WRAPPER_H__
#include <core/context.h>

namespace cdroid{

// AOSP android.content.ContextWrapper: proxy that delegates every Context method
// to a wrapped base context (mBase). Subclasses (ContextThemeWrapper, Activity)
// override specific methods (getTheme/getResources) to change behavior while the
// rest delegate. Essential for per-view theme switching.
class ContextWrapper : public Context{
protected:
    Context* mBase = nullptr;
public:
    ContextWrapper() = default;
    explicit ContextWrapper(Context* base) : mBase(base) {}
    ~ContextWrapper() override = default;

    // AOSP attachBaseContext / getBaseContext.
    virtual void attachBaseContext(Context* base){ mBase = base; }
    Context* getBaseContext() const { return mBase; }

    // --- delegate ALL pure-virtual Context methods to mBase ---
    const std::string getPackageName() const override { return mBase->getPackageName(); }
    void startActivity(const Intent& intent) override { mBase->startActivity(intent); }
    Resources::Theme getTheme() override { return mBase->getTheme(); }
    const std::string getThemeName() const override { return mBase->getThemeName(); }
    void setTheme(int resid) override { mBase->setTheme(resid); }
    const DisplayMetrics& getDisplayMetrics() const override { return mBase->getDisplayMetrics(); }
    int getNextAutofillId() override { return mBase->getNextAutofillId(); }
    //const std::string getString(const std::string& id, const std::string& lan="") override { return mBase->getString(id, lan); }
    std::unique_ptr<std::istream> getInputStream(const std::string& resname, std::string* outpkg=nullptr) override { return mBase->getInputStream(resname, outpkg); }
    Cairo::RefPtr<Cairo::ImageSurface> loadImage(const std::string& resname, int w, int h) override { return mBase->loadImage(resname, w, h); }
    Cairo::RefPtr<Cairo::ImageSurface> loadImage(int id, int w, int h) override { return mBase->loadImage(id, w, h); }
    Cairo::RefPtr<Cairo::ImageSurface> loadImage(std::istream& stream, int w, int h) override { return mBase->loadImage(stream, w, h); }
    std::string getResourceName(uint32_t resId) const override { return mBase->getResourceName(resId); }
    Resources& getResources() override { return mBase->getResources(); }
    AssetManager& getAssets() override { return mBase->getAssets(); }
    Drawable* getDrawable(int id) override { return mBase->getDrawable(id); }
    std::shared_ptr<ColorStateList> getColorStateList(int id) override { return mBase->getColorStateList(id); }
};

} // namespace cdroid
#endif // __CONTEXT_WRAPPER_H__
