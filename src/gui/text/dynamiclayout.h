#ifndef __DYNAMIC_LAYOUT_H__
#define __DYNAMIC_LAYOUT_H__
#include <set>
#include <core/pools.h>
#include <text/staticlayout.h>
#include <text/packedintvector.h>
#include <text/packedobjectvector.h>
namespace cdroid{
class Editable;
class DynamicLayout :public Layout {
private:
    static constexpr int PRIORITY = 128;
    static constexpr int BLOCK_MINIMUM_CHARACTER_LENGTH = 400;
public:
    static constexpr int INVALID_BLOCK_INDEX = -1;
public:
    class Builder {
    private:
        friend DynamicLayout;
        CharSequence* mBase;
        CharSequence* mDisplay;
        TextPaint* mPaint;
        int mWidth;
        Alignment mAlignment;
        const TextDirectionHeuristic* mTextDir;
        float mSpacingMult;
        float mSpacingAdd;
        bool mIncludePad;
        bool mFallbackLineSpacing;
        int mBreakStrategy;
        int mHyphenationFrequency;
        int mJustificationMode;
        TextUtils::TruncateAt mEllipsize;
        int mEllipsizedWidth;
        mutable Paint::FontMetricsInt mFontMetricsInt;
        static Pools::SynchronizedPool<Builder> sPool;
    private:
        Builder()=default;
        static void recycle(Builder* b);
    public:
        static Builder* obtain(CharSequence* base, TextPaint* paint,int width);
        Builder& setDisplayText(CharSequence* display);
        Builder& setAlignment(Alignment alignment);
        Builder& setTextDirection(const TextDirectionHeuristic* textDir);
        Builder& setLineSpacing(float spacingAdd, float spacingMult);
        Builder& setIncludePad(bool includePad);
        Builder& setUseLineSpacingFromFallbacks(bool useLineSpacingFromFallbacks);
        Builder& setEllipsizedWidth(int ellipsizedWidth);
        Builder& setEllipsize(TextUtils::TruncateAt ellipsize);
        Builder& setBreakStrategy(int breakStrategy);
        Builder& setHyphenationFrequency(int hyphenationFrequency);
        Builder& setJustificationMode(int justificationMode);
        DynamicLayout* build();
    };
private:
    DynamicLayout(const Builder& b);
    static CharSequence* createEllipsizer(TextUtils::TruncateAt ellipsize, CharSequence* display);
    void generate(const Builder& b);
    bool contentMayProtrudeFromLineTopOrBottom(CharSequence* text, int start, int end);
    void createBlocks();
    void updateAlwaysNeedsToBeRedrawn(int blockIndex);
    void addBlockAtOffset(int offset);
    bool getContentMayProtrudeFromTopOrBottom(int line) const;
public:
    ~DynamicLayout()override;
    void reflow(CharSequence* s, int where, int before, int after);
    std::set<int> getBlocksAlwaysNeedToBeRedrawn() const;

    void updateBlocks(int startLine, int endLine, int newLineCount);

    void setBlocksDataForTest(const std::vector<int>& blockEndLines, const std::vector<int>& blockIndices,
            int numberOfBlocks, int totalLines);
    std::vector<int> getBlockEndLines() const;
    std::vector<int> getBlockIndices() const;
    int getBlockIndex(int index) const;

    void setBlockIndex(int index, int blockIndex);
    int getNumberOfBlocks() const;

    int getIndexFirstChangedBlock() const;
    void setIndexFirstChangedBlock(int i);
    int getLineCount() const override;

    int getLineTop(int line) const override;
    int getLineDescent(int line) const override;
    int getLineExtra(int line) const override;
    int getLineStart(int line) const override;
    bool getLineContainsTab(int line) const override;
    int getParagraphDirection(int line) const override;
    const Directions* getLineDirections(int line) const override;
    int getTopPadding() const override;
    int getBottomPadding() const override;
    int getStartHyphenEdit(int line) const override;
    int getEndHyphenEdit(int line)const override;
    int getEllipsizedWidth() const override;

    int getEllipsisStart(int line) const override;
    int getEllipsisCount(int line) const override;
private:
    class ChangeWatcher : public ParcelableSpan {//implements TextWatcher, SpanWatcher {
    private:
        DynamicLayout* mLayout;
        void reflow(CharSequence* s, int where, int before, int after);
    public:
        ChangeWatcher(DynamicLayout* layout);
        void beforeTextChanged(CharSequence* s, int where, int before, int after);
        void onTextChanged(CharSequence* s, int where, int before, int after);

        void afterTextChanged(Editable* s);
        void onSpanAdded(Spannable* s, ParcelableSpan* o, int start, int end);
        void onSpanRemoved(Spannable* s, ParcelableSpan* o, int start, int end);
        void onSpanChanged(Spannable* s, ParcelableSpan* o, int start, int end, int nstart, int nend);
    };
private:
    CharSequence* mBase;
    CharSequence* mDisplay;
    ChangeWatcher* mWatcher;
    bool mIncludePad;
    bool mFallbackLineSpacing;
    bool mEllipsize;
    int mEllipsizedWidth;
    TextUtils::TruncateAt mEllipsizeAt;
    int mBreakStrategy;
    int mHyphenationFrequency;
    int mJustificationMode;

    PackedIntVector* mInts;
    PackedObjectVector<const Directions*>* mObjects;

    // Stores the line numbers of the last line of each block (inclusive)
    std::vector<int> mBlockEndLines;
    // The indices of this block's display list in TextView's internal display list array or
    // INVALID_BLOCK_INDEX if this block has been invalidated during an edition
    std::vector<int> mBlockIndices;
    // Set of blocks that always need to be redrawn.
    std::set<int> mBlocksAlwaysNeedToBeRedrawn;
    // Number of items actually currently being used in the above 2 arrays
    int mNumberOfBlocks;
    // The first index of the blocks whose locations are changed
    int mIndexFirstChangedBlock;

    int mTopPadding, mBottomPadding;

    static StaticLayout* sStaticLayout;
    static StaticLayout::Builder* sBuilder;

    // START, DIR, and TAB share the same entry.
    static constexpr int START = 0;
    static constexpr int DIR = START;
    static constexpr int TAB = START;
    static constexpr int TOP = 1;
    static constexpr int DESCENT = 2;
    static constexpr int EXTRA = 3;
    // HYPHEN and MAY_PROTRUDE_FROM_TOP_OR_BOTTOM share the same entry.
    static constexpr int HYPHEN = 4;
    static constexpr int MAY_PROTRUDE_FROM_TOP_OR_BOTTOM = HYPHEN;
    static constexpr int COLUMNS_NORMAL = 5;

    static constexpr int ELLIPSIS_START = 5;
    static constexpr int ELLIPSIS_COUNT = 6;
    static constexpr int COLUMNS_ELLIPSIZE = 7;

    static constexpr int START_MASK = 0x1FFFFFFF;
    static constexpr int DIR_SHIFT  = 30;
    static constexpr int TAB_MASK   = 0x20000000;
    static constexpr int HYPHEN_MASK = 0xFF;
    static constexpr int MAY_PROTRUDE_FROM_TOP_OR_BOTTOM_MASK = 0x100;

    static constexpr int ELLIPSIS_UNDEFINED = 0x80000000;
};
}/*endof namespace*/
#endif/*__DYNAMIC_LAYOUT_H__*/
