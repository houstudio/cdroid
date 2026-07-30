/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.DependencyNode.
 */
#include <widgetEx/constraintlayout/core/widgets/analyzer/dependencynode.h>
#include <widgetEx/constraintlayout/core/widgets/analyzer/dimensiondependency.h>
#include <widgetEx/constraintlayout/core/widgets/analyzer/widgetrun.h>

namespace cdroid {

DependencyNode::DependencyNode(WidgetRun* run)
    : mRun(run) {
}

std::string DependencyNode::toString() {
    std::string resolvedStr = resolved ? std::to_string(value) : std::string("unresolved");
    return mRun->mWidget->getDebugName() + ":" /* + mType name */ + "("
            + resolvedStr + ") <t=" + std::to_string(mTargets.size())
            + ":d=" + std::to_string(mDependencies.size()) + ">";
}

void DependencyNode::resolve(int value) {
    if (resolved) {
        return;
    }
    resolved = true;
    this->value = value;
    for (Dependency* node : mDependencies) {
        node->update(node);
    }
}

void DependencyNode::update(Dependency* /*node*/) {
    for (DependencyNode* target : mTargets) {
        if (!target->resolved) {
            return;
        }
    }
    readyToSolve = true;
    if (updateDelegate != nullptr) {
        updateDelegate->update(this);
    }
    if (delegateToWidgetRun) {
        mRun->update(this);
        return;
    }
    DependencyNode* target = nullptr;
    int numTargets = 0;
    for (DependencyNode* t : mTargets) {
        if (dynamic_cast<DimensionDependency*>(t) != nullptr) {
            continue;
        }
        target = t;
        numTargets++;
    }
    if (target != nullptr && numTargets == 1 && target->resolved) {
        if (mMarginDependency != nullptr) {
            if (mMarginDependency->resolved) {
                mMargin = mMarginFactor * mMarginDependency->value;
            } else {
                return;
            }
        }
        resolve(target->value + mMargin);
    }
    if (updateDelegate != nullptr) {
        updateDelegate->update(this);
    }
}

void DependencyNode::addDependency(Dependency* dependency) {
    mDependencies.push_back(dependency);
    if (resolved) {
        dependency->update(dependency);
    }
}

std::string DependencyNode::name() {
    std::string definition = mRun->mWidget->getDebugName();
    if (mType == Type::LEFT || mType == Type::RIGHT) {
        definition += "_HORIZONTAL";
    } else {
        definition += "_VERTICAL";
    }
    // definition += ":" + mType.name();
    return definition;
}

void DependencyNode::clear() {
    mTargets.clear();
    mDependencies.clear();
    resolved = false;
    value = 0;
    readyToSolve = false;
    delegateToWidgetRun = false;
}

} // namespace cdroid
