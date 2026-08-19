  #pragma once

  #include <iostream>
  #include <map>
  #include <set>
  #include <unordered_map>
  #include <vector>

  using namespace std;

  /// @brief Simple directed graph using an adjacency list.
  /// @tparam VertexT vertex type
  /// @tparam WeightT edge weight type
  template <typename VertexT, typename WeightT>
  class graph {
  private:
    map<VertexT, map<VertexT, WeightT>> adjList;
    size_t edgeCount;

  public:
    /// Default constructor
    graph(){
      this->edgeCount = 0;
    }

    /// @brief Add the vertex `v` to the graph, must typically be O(1).
    /// @param v
    /// @return true if successfully added; false if it existed already
    bool addVertex(VertexT v) {
      // TODO_STUDENT
      if (adjList.count(v)) {
        return false;
      }
      adjList[v] = map<VertexT, WeightT>();
      return true;
    }

    /// @brief Add or overwrite directed edge in the graph, must typically be
    /// O(1).
    /// @param from starting vertex
    /// @param to ending vertex
    /// @param weight edge weight / label
    /// @return true if successfully added or overwritten;
    ///         false if either vertices isn't in graph
    bool addEdge(VertexT from, VertexT to, WeightT weight) {
      if (adjList.find(from) == adjList.end() || adjList.find(to) == adjList.end()) {
        return false;
      }
      
      if (adjList[from].find(to) == adjList[from].end()) {
        this->edgeCount++;
      }
      
      adjList[from][to] = weight;
      return true;
    }

    /// @brief Maybe get the weight associated with a given edge, must typically
    /// be O(1).
    /// @param from starting vertex
    /// @param to ending vertex
    /// @param weight output parameter
    /// @return true if the edge exists, and `weight` is set;
    ///         false if the edge does not exist
    bool getWeight(VertexT from, VertexT to, WeightT& weight) const {
      // TODO_STUDENT
      if (adjList.count(from) == 0 || adjList.count(to) == 0) {
        return false;
      }
      if(adjList.at(from).count(to) == 0){
        return false;
      }
      weight = adjList.at(from).at(to);
      return true;
    }

    /// @brief Get the out-neighbors of `v`. Must run in at most O(|V|).
    /// @param v
    /// @return vertices that v has an edge to
    set<VertexT> neighbors(VertexT v) const {
      set<VertexT> S;
      if(adjList.count(v) == 0){
        return S;
      }
      
      for(auto [vertex, weight]: adjList.at(v)){
        S.insert(vertex);
      }
      return S;
    }

    /// @brief Return a vector containing all vertices in the graph
    vector<VertexT> getVertices() const {
      // TODO_STUDENT
      vector<VertexT> vertices;
      for(auto [vertex, weight]: adjList){
        vertices.push_back(vertex);
      }
      return vertices;
    }

    /// @brief Get the number of vertices in the graph. Runs in O(1).
    size_t numVertices() const {
      // TODO_STUDENT
      return adjList.size();
    }

    /// @brief Get the number of directed edges in the graph. Runs in at most
    /// O(|V|), but should be O(1).
    size_t numEdges() const {
      // TODO_STUDENT
      return this->edgeCount;
    }
  };
