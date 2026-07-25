/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.DependencyNode.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_DEPENDENCY_NODE_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_DEPENDENCY_NODE_H

#include <string>
#include <vector>

#include <widgetEx/constraintlayout/core/widgets/analyzer/dependency.h>

namespace cdroid {

class WidgetRun;
class DimensionDependency;

class DependencyNode : public Dependency {
public:
    enum class Type {
        UNKNOWN, HORIZONTAL_DIMENSION, VERTICAL_DIMENSION,
        LEFT, RIGHT, TOP, BOTTOM, BASELINE
    };

    Dependency* updateDelegate = nullptr;
    bool delegateToWidgetRun = false;
    bool readyToSolve = false;

    WidgetRun* mRun;
    Type mType = Type::UNKNOWN;
    int mMargin = 0;
    int value = 0;
    int mMarginFactor = 1;
    DimensionDependency* mMarginDependency = nullptr;
    bool resolved = false;

    std::vector<Dependency*> mDependencies;
    std::vector<DependencyNode*> mTargets;

    explicit DependencyNode(WidgetRun* run);

    std::string toString();
    virtual void resolve(int value);
    void update(Dependency* node) override;
    void addDependency(Dependency* dependency);
    std::string name();
    void clear();
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_DEPENDENCY_NODE_H
