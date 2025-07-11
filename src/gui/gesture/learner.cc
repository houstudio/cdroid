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
#include <gesture/learner.h>
#include <gesture/instance.h>
namespace cdroid{
Learner::~Learner(){
}
void Learner::addInstance(Instance* instance) {
    mInstances.push_back(instance);
}

/**
 * Retrieve all the instances
 *
 * @return instances
 */
std::vector<Instance*>& Learner::getInstances() {
    return mInstances;
}

/**
 * Remove an instance based on its id
 *
 * @param id
 */
void Learner::removeInstance(long id) {
    for (auto it = mInstances.begin(); it!=mInstances.end(); it++) {
        if ((*it)->id == id) {
            mInstances.erase(it);
            return;
        }
    }
}

/**
 * Remove all the instances of a category
 *
 * @param name the category name
 */
void Learner::removeInstances(const std::string& name) {
    for (auto it = mInstances.begin(); it !=mInstances.end();) {
        // the label can be null, as specified in Instance
        if ((*it)->label.compare(name)==0) {
            it = mInstances.erase(it);
        }else it++;
    }
}

}
