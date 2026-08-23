#pragma once
#include <cdroid.h>

// Post-inflation wiring for each ViewPager2 page. Each function finds its page's
// views by id and attaches listeners/animators. Functions must be idempotent,
// because ViewPager2 may rebind a page's ViewHolder on recycle.
void setupButtons(cdroid::View* page);
void setupProgress(cdroid::View* page);
void setupText(cdroid::View* page);
void setupImages(cdroid::View* page);
void setupAnimation(cdroid::View* page);
void setupLists(cdroid::View* page);
void setupMisc(cdroid::View* page);
void setupDateTime(cdroid::View* page);
void setupConstraint(cdroid::View* page);
void setupMotion(cdroid::View* page);
