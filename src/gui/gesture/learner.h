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
#ifndef __LEARNER_H__
#define __LEARNER_H__
#include <string>
#include <vector>
#include <gesture/prediction.h>
namespace cdroid{
class Instance;
class Learner {
private:
    std::vector<Instance*> mInstances;
public:
    virtual ~Learner();
    /**
     * Add an instance to the learner
     *
     * @param instance
     */
    void addInstance(Instance* instance);
    /**
     * Retrieve all the instances
     *
     * @return instances
     */
    std::vector<Instance*>& getInstances();
    /**
     * Remove an instance based on its id
     *
     * @param id
     */
    void removeInstance(long id);
    /**
     * Remove all the instances of a category
     *
     * @param name the category name
     */
    void removeInstances(const std::string& name);
    virtual std::vector<Prediction> classify(int sequenceType, int orientationType, const std::vector<float>& vector)=0;
};
}/*endof namespace*/
#endif/*__LEARNER_H__*/
