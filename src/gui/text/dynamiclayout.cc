#include <text/dynamiclayout.h>
#include <text/precomputedtext.h>
namespace cdroid{

Pools::SynchronizedPool<DynamicLayout::Builder>DynamicLayout::Builder::sPool(3);
StaticLayout* DynamicLayout::sStaticLayout=nullptr;
StaticLayout::Builder* DynamicLayout::sBuilder=nullptr;

DynamicLayout::Builder* DynamicLayout::Builder::obtain(CharSequence* base, TextPaint* paint,int width) {
    Builder* b = sPool.acquire();
    if (b == nullptr) {
        b = new Builder();
    }

    // set default initial values
    b->mBase = base;
    b->mDisplay = base;
    b->mPaint = paint;
    b->mWidth = width;
    b->mAlignment = Alignment::ALIGN_NORMAL;
    b->mTextDir = TextDirectionHeuristics::FIRSTSTRONG_LTR;
    b->mSpacingMult = DEFAULT_LINESPACING_MULTIPLIER;
    b->mSpacingAdd = DEFAULT_LINESPACING_ADDITION;
    b->mIncludePad = true;
    b->mFallbackLineSpacing = false;
    b->mEllipsizedWidth = width;
    b->mEllipsize = TextUtils::TruncateAt::NONE;
    b->mBreakStrategy = Layout::BREAK_STRATEGY_SIMPLE;
    b->mHyphenationFrequency = Layout::HYPHENATION_FREQUENCY_NONE;
    b->mJustificationMode = Layout::JUSTIFICATION_MODE_NONE;
    b->mLineBreakConfig = LineBreakConfig();  // LineBreakConfig.NONE
    b->mUseBoundsForWidth = false;
    b->mShiftDrawingOffsetForStartOverhang = false;
    b->mMinimumFontMetrics = nullptr;
    return b;
}

void DynamicLayout::Builder::recycle(Builder* b) {
    b->mBase = nullptr;
    b->mDisplay = nullptr;
    b->mPaint = nullptr;
    sPool.release(b);
}

DynamicLayout::Builder& DynamicLayout::Builder::setDisplayText(CharSequence* display) {
    mDisplay = display;
    return *this;
}

DynamicLayout::Builder& DynamicLayout::Builder::setAlignment(Alignment alignment) {
    mAlignment = alignment;
    return *this;
}

DynamicLayout::Builder& DynamicLayout::Builder::setTextDirection(const TextDirectionHeuristic* textDir) {
    mTextDir = textDir;
    return *this;
}

DynamicLayout::Builder& DynamicLayout::Builder::setLineSpacing(float spacingAdd, float spacingMult) {
    mSpacingAdd = spacingAdd;
    mSpacingMult = spacingMult;
    return *this;
}

DynamicLayout::Builder& DynamicLayout::Builder::setIncludePad(bool includePad) {
    mIncludePad = includePad;
    return *this;
}

DynamicLayout::Builder& DynamicLayout::Builder::setUseLineSpacingFromFallbacks(bool useLineSpacingFromFallbacks) {
    mFallbackLineSpacing = useLineSpacingFromFallbacks;
    return *this;
}

DynamicLayout::Builder& DynamicLayout::Builder::setEllipsizedWidth(int ellipsizedWidth) {
    mEllipsizedWidth = ellipsizedWidth;
    return *this;
}

DynamicLayout::Builder& DynamicLayout::Builder::setEllipsize(TextUtils::TruncateAt ellipsize) {
    mEllipsize = ellipsize;
    return *this;
}

DynamicLayout::Builder& DynamicLayout::Builder::setBreakStrategy(int breakStrategy) {
    mBreakStrategy = breakStrategy;
    return *this;
}

DynamicLayout::Builder& DynamicLayout::Builder::setHyphenationFrequency(int hyphenationFrequency) {
    mHyphenationFrequency = hyphenationFrequency;
    return *this;
}

DynamicLayout::Builder& DynamicLayout::Builder::setJustificationMode(int justificationMode) {
    mJustificationMode = justificationMode;
    return *this;
}

DynamicLayout::Builder& DynamicLayout::Builder::setLineBreakConfig(const LineBreakConfig& lineBreakConfig) {
    mLineBreakConfig = lineBreakConfig;
    return *this;
}

DynamicLayout::Builder& DynamicLayout::Builder::setUseBoundsForWidth(bool useBoundsForWidth) {
    mUseBoundsForWidth = useBoundsForWidth;
    return *this;
}

DynamicLayout::Builder& DynamicLayout::Builder::setShiftDrawingOffsetForStartOverhang(
        bool shiftDrawingOffsetForStartOverhang) {
    mShiftDrawingOffsetForStartOverhang = shiftDrawingOffsetForStartOverhang;
    return *this;
}

DynamicLayout::Builder& DynamicLayout::Builder::setMinimumFontMetrics(const Paint::FontMetrics* minimumFontMetrics) {
    mMinimumFontMetrics = minimumFontMetrics;
    return *this;
}

DynamicLayout* DynamicLayout::Builder::build() {
    DynamicLayout* result = new DynamicLayout(*this);
    Builder::recycle(this);
    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Deprecated public constructors — each delegates toward the master constructor below,
// matching android-36 DynamicLayout's constructor chain.
DynamicLayout::DynamicLayout(CharSequence* base, TextPaint* paint, int width, Alignment align,
        float spacingmult, float spacingadd, bool includepad)
    : DynamicLayout(base, base, paint, width, align, spacingmult, spacingadd, includepad) {
}

DynamicLayout::DynamicLayout(CharSequence* base, CharSequence* display, TextPaint* paint,
        int width, Alignment align, float spacingmult, float spacingadd, bool includepad)
    : DynamicLayout(base, display, paint, width, align, spacingmult, spacingadd, includepad,
            TextUtils::TruncateAt::NONE, 0) {
}

DynamicLayout::DynamicLayout(CharSequence* base, CharSequence* display, TextPaint* paint,
        int width, Alignment align, float spacingmult, float spacingadd, bool includepad,
        TextUtils::TruncateAt ellipsize, int ellipsizedWidth)
    : DynamicLayout(base, display, paint, width, align, TextDirectionHeuristics::FIRSTSTRONG_LTR,
            spacingmult, spacingadd, includepad, Layout::BREAK_STRATEGY_SIMPLE,
            Layout::HYPHENATION_FREQUENCY_NONE, Layout::JUSTIFICATION_MODE_NONE,
            LineBreakConfig(), ellipsize, ellipsizedWidth) {
}

DynamicLayout::DynamicLayout(CharSequence* base, CharSequence* display, TextPaint* paint,
        int width, Alignment align, const TextDirectionHeuristic* textDir,
        float spacingmult, float spacingadd, bool includepad, int breakStrategy,
        int hyphenationFrequency, int justificationMode,
        const LineBreakConfig& lineBreakConfig, TextUtils::TruncateAt ellipsize,
        int ellipsizedWidth)
    : Layout(createEllipsizer(ellipsize, display),
            paint, width, align, textDir, spacingmult, spacingadd, includepad,
            false /* fallbackLineSpacing */, ellipsizedWidth, ellipsize,
            INT_MAX /* maxLines */, breakStrategy, hyphenationFrequency,
            {} /* leftIndents */, {} /* rightIndents */, justificationMode,
            lineBreakConfig, false /* useBoundsForWidth */, false,
            nullptr /* minimumFontMetrics */) {

    Builder* bp = Builder::obtain(base, paint, width);
    Builder& b = bp->setAlignment(align)
            .setTextDirection(textDir)
            .setLineSpacing(spacingadd, spacingmult)
            .setEllipsizedWidth(ellipsizedWidth)
            .setEllipsize(ellipsize);

    mDisplay = display;
    mIncludePad = includepad;
    mBreakStrategy = breakStrategy;
    mJustificationMode = justificationMode;
    mHyphenationFrequency = hyphenationFrequency;
    mLineBreakConfig = lineBreakConfig;

    generate(b);

    Builder::recycle(&b);
}

DynamicLayout::DynamicLayout(const Builder& b)
    : Layout(createEllipsizer(b.mEllipsize, b.mDisplay),
            b.mPaint, b.mWidth, b.mAlignment, b.mTextDir, b.mSpacingMult, b.mSpacingAdd,
            b.mIncludePad, b.mFallbackLineSpacing, b.mEllipsizedWidth, b.mEllipsize,
            INT_MAX /* maxLines */, b.mBreakStrategy, b.mHyphenationFrequency,
            {} /* leftIndents */, {} /* rightIndents */, b.mJustificationMode,
            b.mLineBreakConfig, b.mUseBoundsForWidth, b.mShiftDrawingOffsetForStartOverhang,
            b.mMinimumFontMetrics) {

    mDisplay = b.mDisplay;
    mIncludePad = b.mIncludePad;
    mBreakStrategy = b.mBreakStrategy;
    mJustificationMode = b.mJustificationMode;
    mHyphenationFrequency = b.mHyphenationFrequency;
    mLineBreakConfig = b.mLineBreakConfig;

    generate(b);
}

DynamicLayout::~DynamicLayout(){
    delete mInts;
    delete mObjects;
}

CharSequence* DynamicLayout::createEllipsizer(TextUtils::TruncateAt ellipsize,CharSequence* display) {
    if (ellipsize == TextUtils::TruncateAt::NONE) {
        return display;
    } else {
        Spanned* spanned = dynamic_cast<Spanned*>(display);
        if (spanned != nullptr) {
            return new SpannedEllipsizer(spanned);
        } else {
            return new Ellipsizer(display);
        }
    }
}

void DynamicLayout::generate(const Builder& b) {
    mBase = b.mBase;
    mFallbackLineSpacing = b.mFallbackLineSpacing;
    mUseBoundsForWidth = b.mUseBoundsForWidth;
    mShiftDrawingOffsetForStartOverhang = b.mShiftDrawingOffsetForStartOverhang;
    mMinimumFontMetrics = b.mMinimumFontMetrics;
    if (b.mEllipsize != TextUtils::TruncateAt::NONE) {
        mInts = new PackedIntVector(COLUMNS_ELLIPSIZE);
        mEllipsizedWidth = b.mEllipsizedWidth;
        mEllipsizeAt = b.mEllipsize;

        Ellipsizer* e = dynamic_cast<Ellipsizer*>(getText());
        e->mLayout = this;
        e->mWidth = b.mEllipsizedWidth;
        e->mMethod = b.mEllipsize;
        mEllipsize = true;
    } else {
        mInts = new PackedIntVector(COLUMNS_NORMAL);
        mEllipsizedWidth = b.mWidth;
        mEllipsizeAt = TextUtils::TruncateAt::NONE;
    }

    mObjects = new PackedObjectVector<const Directions*>(1);

    // Initial state is a single line with 0 characters (0 to 0), with top at 0 and bottom at
    // whatever is natural, and undefined ellipsis.

    std::vector<int> start;

    if (b.mEllipsize != TextUtils::TruncateAt::NONE) {
        start.resize(COLUMNS_ELLIPSIZE);
        start[ELLIPSIS_START] = ELLIPSIS_UNDEFINED;
    } else {
        start.resize(COLUMNS_NORMAL);
    }

    std::vector<const Directions*> dirs ={ &DIRS_ALL_LEFT_TO_RIGHT };

    Paint::FontMetricsInt& fm = b.mFontMetricsInt;
    b.mPaint->getFontMetricsInt(&fm);
    const int asc = fm.ascent;
    const int desc = fm.descent;

    start[DIR] = DIR_LEFT_TO_RIGHT << DIR_SHIFT;
    start[TOP] = 0;
    start[DESCENT] = desc;
    mInts->insertAt(0, start);

    start[TOP] = desc - asc;
    mInts->insertAt(1, start);

    mObjects->insertAt(0, dirs);

    const int baseLength = mBase->length();
    // Update from 0 characters to whatever the real text is
    reflow(mBase, 0, 0, baseLength);

    Spannable* sp = dynamic_cast<Spannable*>(mBase);
    if (sp != nullptr) {
        if (mWatcher == nullptr)
            mWatcher = new ChangeWatcher(this);

        // Strip out any watchers for other DynamicLayouts.
        auto spans = sp->getSpans(0, baseLength, make_span_filter<ChangeWatcher>());
        for (int i = 0; i < spans.size(); i++) {
            sp->removeSpan(spans[i]);
        }

        sp->setSpan(mWatcher, 0, baseLength, Spannable::SPAN_INCLUSIVE_INCLUSIVE |
                   (PRIORITY << Spannable::SPAN_PRIORITY_SHIFT));
    }
}

void DynamicLayout::reflow(CharSequence* s, int where, int before, int after) {
    if (s != mBase)
        return;

    CharSequence* text = mDisplay;
    const int len = text->length();

    // seek back to the start of the paragraph

    int find = TextUtils::lastIndexOf(text, '\n', where - 1);
    if (find < 0)
        find = 0;
    else
        find = find + 1;

    {
        int diff = where - find;
        before += diff;
        after += diff;
        where -= diff;
    }

    // seek forward to the end of the paragraph

    int look = TextUtils::indexOf(text, '\n', where + after);
    if (look < 0)
        look = len;
    else
        look++; // we want the index after the \n

    int change = look - (where + after);
    before += change;
    after += change;

    // seek further out to cover anything that is forced to wrap together

    Spanned* sp = dynamic_cast<Spanned*>(text);
    if (sp != nullptr) {
        bool again;

        do {
            again = false;

            auto force = sp->getSpans(where, where + after, make_span_filter<WrapTogetherSpan>());

            for (int i = 0; i < force.size(); i++) {
                int st = sp->getSpanStart(force[i]);
                int en = sp->getSpanEnd(force[i]);

                if (st < where) {
                    again = true;

                    int diff = where - st;
                    before += diff;
                    after += diff;
                    where -= diff;
                }

                if (en > where + after) {
                    again = true;

                    int diff = en - (where + after);
                    before += diff;
                    after += diff;
                }
            }
        } while (again);
    }

    // find affected region of old layout

    int startline = getLineForOffset(where);
    int startv = getLineTop(startline);

    int endline = getLineForOffset(where + before);
    if (where + after == len)
        endline = getLineCount();
    int endv = getLineTop(endline);
    bool islast = (endline == getLineCount());

    // generate new layout for affected text

    StaticLayout* reflowed;
    StaticLayout::Builder* b;

    //synchronized (sLock)
    {
        reflowed = sStaticLayout;
        b = sBuilder;
        sStaticLayout = nullptr;
        sBuilder = nullptr;
    }

    if (b == nullptr) {
        b = StaticLayout::Builder::obtain(text, where, where + after, getPaint(), getWidth());
    }

    b->setText(text, where, where + after)
            .setPaint(getPaint())
            .setWidth(getWidth())
            .setTextDirection(getTextDirectionHeuristic())
            .setLineSpacing(getSpacingAdd(), getSpacingMultiplier())
            .setUseLineSpacingFromFallbacks(mFallbackLineSpacing)
            .setEllipsizedWidth(mEllipsizedWidth)
            .setEllipsize(mEllipsizeAt)
            .setBreakStrategy(mBreakStrategy)
            .setHyphenationFrequency(mHyphenationFrequency)
            .setJustificationMode(mJustificationMode)
            .setLineBreakConfig(mLineBreakConfig)
            .setAddLastLineLineSpacing(!islast)
            .setIncludePad(false)
            .setUseBoundsForWidth(mUseBoundsForWidth)
            .setShiftDrawingOffsetForStartOverhang(mShiftDrawingOffsetForStartOverhang)
            .setMinimumFontMetrics(mMinimumFontMetrics)
            .setCalculateBounds(true);

    reflowed = b->buildPartialStaticLayoutForDynamicLayout(true /* trackpadding */, reflowed);
    int n = reflowed->getLineCount();
    // If the new layout has a blank line at the end, but it is not
    // the very end of the buffer, then we already have a line that
    // starts there, so disregard the blank line.

    if (where + after != len && reflowed->getLineStart(n - 1) == where + after)
        n--;

    // remove affected lines from old layout
    mInts->deleteAt(startline, endline - startline);
    mObjects->deleteAt(startline, endline - startline);

    // adjust offsets in layout for new height and offsets

    int ht = reflowed->getLineTop(n);
    int toppad = 0, botpad = 0;

    if (mIncludePad && startline == 0) {
        toppad = reflowed->getTopPadding();
        mTopPadding = toppad;
        ht -= toppad;
    }
    if (mIncludePad && islast) {
        botpad = reflowed->getBottomPadding();
        mBottomPadding = botpad;
        ht += botpad;
    }

    mInts->adjustValuesBelow(startline, START, after - before);
    mInts->adjustValuesBelow(startline, TOP, startv - endv + ht);

    // insert new layout

    std::vector<int> ints;

    if (mEllipsize) {
        ints.resize(COLUMNS_ELLIPSIZE);
        ints[ELLIPSIS_START] = ELLIPSIS_UNDEFINED;
    } else {
        ints.resize(COLUMNS_NORMAL);
    }

    std::vector<const Directions*> objects(1);

    for (int i = 0; i < n; i++) {
        const int start = reflowed->getLineStart(i);
        ints[START] = start;
        ints[DIR] |= reflowed->getParagraphDirection(i) << DIR_SHIFT;
        ints[TAB] |= reflowed->getLineContainsTab(i) ? TAB_MASK : 0;

        int top = reflowed->getLineTop(i) + startv;
        if (i > 0)
            top -= toppad;
        ints[TOP] = top;

        int desc = reflowed->getLineDescent(i);
        if (i == n - 1)
            desc += botpad;

        ints[DESCENT] = desc;
        ints[EXTRA] = reflowed->getLineExtra(i);
        objects[0] = reflowed->getLineDirections(i);

        const int end = (i == n - 1) ? where + after : reflowed->getLineStart(i + 1);
        ints[HYPHEN] = StaticLayout::packHyphenEdit(
                reflowed->getStartHyphenEdit(i), reflowed->getEndHyphenEdit(i));
        ints[MAY_PROTRUDE_FROM_TOP_OR_BOTTOM] |=
                contentMayProtrudeFromLineTopOrBottom(text, start, end) ?
                        MAY_PROTRUDE_FROM_TOP_OR_BOTTOM_MASK : 0;

        if (mEllipsize) {
            ints[ELLIPSIS_START] = reflowed->getEllipsisStart(i);
            ints[ELLIPSIS_COUNT] = reflowed->getEllipsisCount(i);
        }

        mInts->insertAt(startline + i, ints);
        mObjects->insertAt(startline + i, objects);
    }

    updateBlocks(startline, endline - 1, n);

    b->finish();
    //synchronized (sLock) 
    {
        sStaticLayout = reflowed;
        sBuilder = b;
    }
}

bool DynamicLayout::contentMayProtrudeFromLineTopOrBottom(CharSequence* text, int start, int end) {
    Spanned* spanned = dynamic_cast<Spanned*>(text);
    if (spanned != nullptr) {
        if (spanned->getSpans(start, end, make_span_filter<ReplacementSpan>()).size() > 0) {
            return true;
        }
    }
    // Spans other than ReplacementSpan can be ignored because line top and bottom are
    // disjunction of all tops and bottoms, although it's not optimal.
    Paint* paint = getPaint();
    Rect mTempRect={};
    PrecomputedText* precomputed = dynamic_cast<PrecomputedText*>(text);
    if (precomputed != nullptr) {
        precomputed->getBounds(start, end, mTempRect);
    } else {
        paint->getTextBounds(text, start, end, mTempRect);
    }
    Paint::FontMetricsInt fm;
    paint->getFontMetricsInt(&fm);
    return mTempRect.top < fm.top || mTempRect.bottom() > fm.bottom;
}

void DynamicLayout::createBlocks() {
    int offset = BLOCK_MINIMUM_CHARACTER_LENGTH;
    mNumberOfBlocks = 0;
    CharSequence* text = mDisplay;

    while (true) {
        offset = TextUtils::indexOf(text, '\n', offset);
        if (offset < 0) {
            addBlockAtOffset(text->length());
            break;
        } else {
            addBlockAtOffset(offset);
            offset += BLOCK_MINIMUM_CHARACTER_LENGTH;
        }
    }

    // mBlockIndices and mBlockEndLines should have the same length
    mBlockIndices.resize(mBlockEndLines.size());
    for (int i = 0; i < mBlockEndLines.size(); i++) {
        mBlockIndices[i] = INVALID_BLOCK_INDEX;
    }
}

std::set<int> DynamicLayout::getBlocksAlwaysNeedToBeRedrawn() const{
    return mBlocksAlwaysNeedToBeRedrawn;
}

void DynamicLayout::updateAlwaysNeedsToBeRedrawn(int blockIndex) {
    int startLine = blockIndex == 0 ? 0 : (mBlockEndLines[blockIndex - 1] + 1);
    int endLine = mBlockEndLines[blockIndex];
    for (int i = startLine; i <= endLine; i++) {
        if (getContentMayProtrudeFromTopOrBottom(i)) {
            /*if (mBlocksAlwaysNeedToBeRedrawn == null) {
                mBlocksAlwaysNeedToBeRedrawn = new ArraySet<>();
            }*/
            mBlocksAlwaysNeedToBeRedrawn.insert(blockIndex);
            return;
        }
    }
    if (!mBlocksAlwaysNeedToBeRedrawn.empty()) {
        mBlocksAlwaysNeedToBeRedrawn.erase(blockIndex);
    }
}

void DynamicLayout::addBlockAtOffset(int offset) {
    const int line = getLineForOffset(offset);
    if (mBlockEndLines.empty()) {
        // Initial creation of the array, no test on previous block ending line
        mBlockEndLines.resize(1);// = ArrayUtils.newUnpaddedIntArray(1);
        mBlockEndLines[mNumberOfBlocks] = line;
        updateAlwaysNeedsToBeRedrawn(mNumberOfBlocks);
        mNumberOfBlocks++;
        return;
    }

    const int previousBlockEndLine = mBlockEndLines[mNumberOfBlocks - 1];
    if (line > previousBlockEndLine) {
        mBlockEndLines.push_back(line);// = GrowingArrayUtils.append(mBlockEndLines, mNumberOfBlocks, line);
        updateAlwaysNeedsToBeRedrawn(mNumberOfBlocks);
        mNumberOfBlocks++;
    }
}

void DynamicLayout::updateBlocks(int startLine, int endLine, int newLineCount) {
    if (mBlockEndLines.empty()) {
        createBlocks();
        return;
    }

    int firstBlock = -1;
    int lastBlock = -1;
    for (int i = 0; i < mNumberOfBlocks; i++) {
        if (mBlockEndLines[i] >= startLine) {
            firstBlock = i;
            break;
        }
    }
    for (int i = firstBlock; i < mNumberOfBlocks; i++) {
        if (mBlockEndLines[i] >= endLine) {
            lastBlock = i;
            break;
        }
    }
    const int lastBlockEndLine = mBlockEndLines[lastBlock];

    const bool createBlockBefore = startLine > (firstBlock == 0 ? 0 :
            mBlockEndLines[firstBlock - 1] + 1);
    const bool createBlock = newLineCount > 0;
    const bool createBlockAfter = endLine < mBlockEndLines[lastBlock];

    int numAddedBlocks = 0;
    if (createBlockBefore) numAddedBlocks++;
    if (createBlock) numAddedBlocks++;
    if (createBlockAfter) numAddedBlocks++;

    const int numRemovedBlocks = lastBlock - firstBlock + 1;
    const int newNumberOfBlocks = mNumberOfBlocks + numAddedBlocks - numRemovedBlocks;

    if (newNumberOfBlocks == 0) {
        // Even when text is empty, there is actually one line and hence one block
        mBlockEndLines[0] = 0;
        mBlockIndices[0] = INVALID_BLOCK_INDEX;
        mNumberOfBlocks = 1;
        return;
    }

    if (newNumberOfBlocks > mBlockEndLines.size()) {
        /*int[] blockEndLines = ArrayUtils.newUnpaddedIntArray( std::max(mBlockEndLines.length * 2, newNumberOfBlocks));
        int[] blockIndices = new int[blockEndLines.length];
        System.arraycopy(mBlockEndLines, 0, blockEndLines, 0, firstBlock);
        System.arraycopy(mBlockIndices, 0, blockIndices, 0, firstBlock);
        System.arraycopy(mBlockEndLines, lastBlock + 1, blockEndLines, firstBlock + numAddedBlocks, mNumberOfBlocks - lastBlock - 1);
        System.arraycopy(mBlockIndices, lastBlock + 1, blockIndices, firstBlock + numAddedBlocks, mNumberOfBlocks - lastBlock - 1);
        mBlockEndLines = blockEndLines;
        mBlockIndices = blockIndices;*/
        
        const int newSize = std::max(static_cast<int>(mBlockEndLines.size()) * 2, newNumberOfBlocks);
        mBlockEndLines.resize(newSize);
        mBlockIndices.resize(newSize);
        std::copy_backward(mBlockEndLines.begin() + lastBlock + 1, mBlockEndLines.begin() + mNumberOfBlocks,
                       mBlockEndLines.begin() + firstBlock + numAddedBlocks);
        std::copy_backward(mBlockIndices.begin() + lastBlock + 1, mBlockIndices.begin() + mNumberOfBlocks,
                       mBlockIndices.begin() + firstBlock + numAddedBlocks);

    } else if (numAddedBlocks + numRemovedBlocks != 0) {
        //System.arraycopy(mBlockEndLines, lastBlock + 1, mBlockEndLines, firstBlock + numAddedBlocks, mNumberOfBlocks - lastBlock - 1);
        //System.arraycopy(mBlockIndices, lastBlock + 1, mBlockIndices, firstBlock + numAddedBlocks, mNumberOfBlocks - lastBlock - 1);
        std::copy_backward(mBlockEndLines.begin() + lastBlock + 1, mBlockEndLines.begin() + mNumberOfBlocks,
                       mBlockEndLines.begin() + firstBlock + numAddedBlocks);
        std::copy_backward(mBlockIndices.begin() + lastBlock + 1, mBlockIndices.begin() + mNumberOfBlocks,
                       mBlockIndices.begin() + firstBlock + numAddedBlocks);
    }

    if ((numAddedBlocks + numRemovedBlocks != 0) && mBlocksAlwaysNeedToBeRedrawn.size()) {
        std::set<int> set;
        const int changedBlockCount = numAddedBlocks - numRemovedBlocks;
        for (auto block:mBlocksAlwaysNeedToBeRedrawn) {
            if (block < firstBlock) {
                // block index is before firstBlock add it since it did not change
                set.insert(block);
            }
            if (block > lastBlock) {
                // block index is after lastBlock, the index reduced to += changedBlockCount
                block += changedBlockCount;
                set.insert(block);
            }
        }
        mBlocksAlwaysNeedToBeRedrawn = set;
    }

    mNumberOfBlocks = newNumberOfBlocks;
    int newFirstChangedBlock;
    const int deltaLines = newLineCount - (endLine - startLine + 1);
    if (deltaLines != 0) {
        // Display list whose index is >= mIndexFirstChangedBlock is valid
        // but it needs to update its drawing location.
        newFirstChangedBlock = firstBlock + numAddedBlocks;
        for (int i = newFirstChangedBlock; i < mNumberOfBlocks; i++) {
            mBlockEndLines[i] += deltaLines;
        }
    } else {
        newFirstChangedBlock = mNumberOfBlocks;
    }
    mIndexFirstChangedBlock = std::min(mIndexFirstChangedBlock, newFirstChangedBlock);

    int blockIndex = firstBlock;
    if (createBlockBefore) {
        mBlockEndLines[blockIndex] = startLine - 1;
        updateAlwaysNeedsToBeRedrawn(blockIndex);
        mBlockIndices[blockIndex] = INVALID_BLOCK_INDEX;
        blockIndex++;
    }

    if (createBlock) {
        mBlockEndLines[blockIndex] = startLine + newLineCount - 1;
        updateAlwaysNeedsToBeRedrawn(blockIndex);
        mBlockIndices[blockIndex] = INVALID_BLOCK_INDEX;
        blockIndex++;
    }

    if (createBlockAfter) {
        mBlockEndLines[blockIndex] = lastBlockEndLine + deltaLines;
        updateAlwaysNeedsToBeRedrawn(blockIndex);
        mBlockIndices[blockIndex] = INVALID_BLOCK_INDEX;
    }
}

void DynamicLayout::setBlocksDataForTest(const std::vector<int>& blockEndLines,const std::vector<int>& blockIndices, int numberOfBlocks,
        int totalLines) {
    mBlockEndLines = blockEndLines;
    mBlockIndices = blockIndices;
    mNumberOfBlocks = numberOfBlocks;
    while (mInts->size() < totalLines) {
        mInts->insertAt(mInts->size(), std::vector<int>(COLUMNS_NORMAL));
    }
}

std::vector<int> DynamicLayout::getBlockEndLines() const{
    return mBlockEndLines;
}

std::vector<int> DynamicLayout::getBlockIndices() const{
    return mBlockIndices;
}

 int DynamicLayout::getBlockIndex(int index) const{
    return mBlockIndices[index];
}

void DynamicLayout::setBlockIndex(int index, int blockIndex) {
    mBlockIndices[index] = blockIndex;
}

int DynamicLayout::getNumberOfBlocks() const{
    return mNumberOfBlocks;
}

int DynamicLayout::getIndexFirstChangedBlock() const{
    return mIndexFirstChangedBlock;
}

void DynamicLayout::setIndexFirstChangedBlock(int i) {
    mIndexFirstChangedBlock = i;
}

int DynamicLayout::getLineCount() const{
    return mInts->size() - 1;
}

int DynamicLayout::getLineTop(int line) const{
    return mInts->getValue(line, TOP);
}

int DynamicLayout::getLineDescent(int line) const{
    return mInts->getValue(line, DESCENT);
}

int DynamicLayout::getLineExtra(int line) const{
    return mInts->getValue(line, EXTRA);
}

int DynamicLayout::getLineStart(int line) const{
    return mInts->getValue(line, START) & START_MASK;
}

bool DynamicLayout::getLineContainsTab(int line) const{
    return (mInts->getValue(line, TAB) & TAB_MASK) != 0;
}

int DynamicLayout::getParagraphDirection(int line) const{
    return mInts->getValue(line, DIR) >> DIR_SHIFT;
}

const Directions* DynamicLayout::getLineDirections(int line) const{
    return mObjects->getValue(line, 0);
}

int DynamicLayout::getTopPadding() const{
    return mTopPadding;
}

int DynamicLayout::getBottomPadding() const{
    return mBottomPadding;
}

int DynamicLayout::getStartHyphenEdit(int line) const{
    return StaticLayout::unpackStartHyphenEdit(mInts->getValue(line, HYPHEN) & HYPHEN_MASK);
}

int DynamicLayout::getEndHyphenEdit(int line) const{
    return StaticLayout::unpackEndHyphenEdit(mInts->getValue(line, HYPHEN) & HYPHEN_MASK);
}

bool DynamicLayout::getContentMayProtrudeFromTopOrBottom(int line) const{
    return (mInts->getValue(line, MAY_PROTRUDE_FROM_TOP_OR_BOTTOM)
            & MAY_PROTRUDE_FROM_TOP_OR_BOTTOM_MASK) != 0;
}

int DynamicLayout::getEllipsizedWidth() const{
    return mEllipsizedWidth;
}

int DynamicLayout::getEllipsisStart(int line) const{
    if (mEllipsizeAt == TextUtils::TruncateAt::NONE) {
        return 0;
    }

    return mInts->getValue(line, ELLIPSIS_START);
}

int DynamicLayout::getEllipsisCount(int line) const{
    if (mEllipsizeAt == TextUtils::TruncateAt::NONE) {
        return 0;
    }

    return mInts->getValue(line, ELLIPSIS_COUNT);
}

LineBreakConfig DynamicLayout::getLineBreakConfig() const{
    return mLineBreakConfig;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

DynamicLayout::ChangeWatcher::ChangeWatcher(DynamicLayout* layout) {
    mLayout = layout;
}

void DynamicLayout::ChangeWatcher::reflow(CharSequence* s, int where, int before, int after) {
    DynamicLayout* ml = mLayout;

    if (ml != nullptr) {
        ml->reflow(s, where, before, after);
    } else {
        Spannable* sp = dynamic_cast<Spannable*>(s);
        if (sp != nullptr) {
            sp->removeSpan(this);
        }
    }
}

void DynamicLayout::ChangeWatcher::beforeTextChanged(CharSequence* s, int where, int before, int after) {
    // Intentionally empty
}

void DynamicLayout::ChangeWatcher::onTextChanged(CharSequence* s, int where, int before, int after) {
    reflow(s, where, before, after);
}

void DynamicLayout::ChangeWatcher::afterTextChanged(Editable* s) {
    // Intentionally empty
}

void DynamicLayout::ChangeWatcher::onSpanAdded(Spannable* s, ParcelableSpan* o, int start, int end) {
    if (dynamic_cast<UpdateLayout*>(o))
        reflow(s, start, end - start, end - start);
}

void DynamicLayout::ChangeWatcher::onSpanRemoved(Spannable* s, ParcelableSpan* o, int start, int end) {
    if (dynamic_cast<UpdateLayout*>(o))
        reflow(s, start, end - start, end - start);
}

void DynamicLayout::ChangeWatcher::onSpanChanged(Spannable* s, ParcelableSpan* o, int start, int end, int nstart, int nend) {
    if (dynamic_cast<UpdateLayout*>(o)) {
        if (start > end) {
            // Bug: 67926915 start cannot be determined, fallback to reflow from start
            // instead of causing an exception
            start = 0;
        }
        reflow(s, start, end - start, end - start);
        reflow(s, nstart, nend - nstart, nend - nstart);
    }
}

}/*endof namespace*/
