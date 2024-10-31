#ifndef __DIRECTEDACYCLICGRAPH_H__
#define __DIRECTEEACYCLICGRAPH_H__
#include <set>
#include <map>
#include <vector>
#include <core/pools.h>

namespace cdroid{

template<typename T>
class DirectedAcyclicGraph{
private:
    Pools::SimplePool<std::vector<T*>>* mListPool;// (10);
    std::map<T*, std::vector<T*>*> mGraph;

    std::vector<T*> mSortResult;
    std::set<T*> mSortTmpMarked;
private:
    void dfs(T* node, std::vector<T*>&result, std::set<T*>& tmpMarked) {
        if (std::find(result.begin(),result.end(),node)!=result.end()) {//result.contains(node)) {
            // We've already seen and added the node to the result list, skip...
            return;
        }
        if (tmpMarked.find(node)!= tmpMarked.end()) {//contains(node)) {
            LOGE("This graph contains cyclic dependencies");
        }
        // Temporarily mark the node
        tmpMarked.insert(node);
        // Recursively dfs all of the node's edges
        auto itg = mGraph.find(node);
        std::vector<T*>* edges = itg!=mGraph.end()?itg->second:nullptr;// mGraph.get(node);
        if (edges!=nullptr) {
            for (int i = 0, size = edges->size(); i < size; i++) {
                dfs(edges->at(i), result, tmpMarked);
            }
        }
        // Unmark the node from the temporary list
        tmpMarked.erase(node);
        // Finally add it to the result list
        result.push_back(node);
    }

    std::vector<T*>* getEmptyList() {
        std::vector<T*>* list = mListPool->acquire();
        if (list == nullptr) {
            list = new std::vector<T*>;
        }
        return list;
    }

    void poolList(std::vector<T*>& list) {
        list.clear();
        mListPool->release(&list);
    }
public:
    DirectedAcyclicGraph() {
        mListPool = new Pools::SimplePool<std::vector<T*>>(10);
    }
    ~DirectedAcyclicGraph() {
        delete mListPool;
    }
    void addNode(T* node) {
        if (mGraph.find(node)==mGraph.end()) {
            mGraph.insert({ node, nullptr });
        }
    }

    bool contains(T* node) const{
        return mGraph.find(node)!=mGraph.end();
    }

    void addEdge(T* node, T* incomingEdge) {
        if ((mGraph.find(node)!=mGraph.end()) || (mGraph.find(incomingEdge)!=mGraph.end())) {
            LOGE("All nodes must be present in the graph before being added as an edge");
        }
        auto it = mGraph.find(node);
        std::vector<T*>* edges = it->second;// mGraph.get(node);
        if (edges == nullptr) {
            // If edges is null, we should try and get one from the pool and add it to the graph
            edges = getEmptyList();
            mGraph.insert({ node, edges });
        }
        // Finally add the edge to the list
        edges->push_back(incomingEdge);
    }

    std::vector<T*> getIncomingEdges(T* node) {
        auto it = mGraph.find(node);
        return *it->second;
    }

    std::vector<T*> getOutgoingEdges(T* node) {
        std::vector<T*> result;
        for (auto p:mGraph){//int i = 0, size = mGraph.size(); i < size; i++) {
            std::vector<T*>* edges = p.second;// mGraph.valueAt(i);
            if (edges != nullptr && (std::find(edges->begin(),edges->end(),node)!=edges->end())) {
                result.push_back(p.first);// mGraph.keyAt(i));
            }
        }
        return result;
    }

    bool hasOutgoingEdges(T* node) {
        for (const auto p:mGraph){//int i = 0, size = mGraph.size(); i < size; i++) {
            std::vector<T*>* edges = p.second;// mGraph.valueAt(i);
            if ((edges != nullptr) && (std::find(edges->begin(),edges->end(),node)!=edges->end())) {
                return true;
            }
        }
        return false;
    }

    void clear() {
        for (auto p:mGraph){//int i = 0, size = mGraph.size(); i < size; i++) {
            std::vector<T*>* edges = p.second;// mGraph.valueAt(i);
            if (edges != nullptr) {
                poolList(*edges);
            }
        }
        mGraph.clear();
    }

    std::vector<T*>& getSortedList() {
        mSortResult.clear();
        mSortTmpMarked.clear();
        // Start a DFS from each node in the graph
        for (const auto itm:mGraph){// int i = 0, size = mGraph.size(); i < size; i++) {
            dfs(itm.first, mSortResult, mSortTmpMarked);
            //dfs(mGraph.keyAt(i), mSortResult, mSortTmpMarked);
        }
        return mSortResult;
    }
    
    size_t size() const{
        return mGraph.size();
    }
};
}/*endof namespace*/

#endif/*__DIRECTAYCLICGRAPH_H__*/

