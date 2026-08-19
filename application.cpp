#include "application.h"

#include <iostream>
#include <limits>
#include <map>
#include <queue> // priority_queue
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "dist.h"
#include "graph.h"

#include "json.hpp"
using json = nlohmann::json;

using namespace std;
class prioritize {
 public:
  bool operator()(const pair<long long, double>& p1,
                  const pair<long long, double>& p2) const {
    return p1.second > p2.second;
  }
};


double INF = numeric_limits<double>::max();

void buildGraph(istream &input, graph<long long, double> &g,
                vector<BuildingInfo> &buildings,
                unordered_map<long long, Coordinates> &coords) {
    json j;
    input >> j;

    // 1. Waypoints → non-building vertices
    if (j.contains("waypoints")) {
        for (auto &node : j["waypoints"]) {
            long long id = node["id"];
            double lat = node["lat"];
            double lon = node["lon"];

            Coordinates c(lat, lon);
            coords[id] = c;
            g.addVertex(id);
        }
    }

    // 2. Footways → undirected edges
    if (j.contains("footways")) {
        for (auto &footway : j["footways"]) {
            vector<long long> nodes;
            if (footway.is_array()) {
                nodes = footway.get<vector<long long>>();
            } else if (footway.contains("nodes")) {
                nodes = footway["nodes"].get<vector<long long>>();
            }

            for (size_t i = 0; i + 1 < nodes.size(); i++) {
                long long n1 = nodes[i];
                long long n2 = nodes[i + 1];

                // Verify both nodes exist in the graph/coords
                if (coords.count(n1) && coords.count(n2)) {
                    double dist = distBetween2Points(coords[n1], coords[n2]);
                    g.addEdge(n1, n2, dist);
                    g.addEdge(n2, n1, dist);
                }
            }
        }
    }

    // 3. Buildings → vertices (Not added to coords map)
    if (j.contains("buildings")) {
        for (auto &b : j["buildings"]) {
            long long id = b["id"];
            double lat = b["lat"];
            double lon = b["lon"];
            
            // Use .value() to safely handle missing strings
            string name = b.value("name", "");
            string abbr = b.value("abbr", "");

            Coordinates loc(lat, lon);
            buildings.emplace_back(id, loc, name, abbr);

            // Add the building as a vertex
            g.addVertex(id);
        }
    }

    // 4. Connect buildings → ALL waypoints within 0.036 miles
    // This makes the building part of the undirected graph network
    for (auto &b : buildings) {
        for (auto const& [nodeID, nodeCoord] : coords) {
            double d = distBetween2Points(b.location, nodeCoord);

            if (d <= 0.036) {
                // Add undirected edge (both directions)
                g.addEdge(b.id, nodeID, d);
                g.addEdge(nodeID, b.id, d);
            }
        }
    }
}

BuildingInfo getBuildingInfo(const vector<BuildingInfo> &buildings,
                             const string &query) {
  for (const BuildingInfo &building : buildings) {
    if (building.abbr == query) {
      return building;
    } else if (building.name.find(query) != string::npos) {
      return building;
    }
  }
  BuildingInfo fail;
  fail.id = -1;
  return fail;
}

BuildingInfo getClosestBuilding(const vector<BuildingInfo> &buildings,
                                Coordinates c) {
  double minDestDist = INF;
  BuildingInfo ret = buildings.at(0);
  for (const BuildingInfo &building : buildings) {
    double dist = distBetween2Points(building.location, c);
    if (dist < minDestDist) {
      minDestDist = dist;
      ret = building;
    }
  }
  return ret;
}

vector<long long> dijkstra(const graph<long long, double> &G, long long start,
                           long long target,
                           const set<long long> &ignoreNodes) {
  // TODO_STUDENT
    if(start == target){
      vector<long long> v; 
      v.push_back(start);
      return v;
    }
    priority_queue<pair<long long, double>,
               vector<pair<long long, double>>,
               prioritize>
    worklist;

    pair<long long, double> s;
    s.first = start;
    s.second = 0.0;
    worklist.push(s);

    unordered_map<long long, double> distances;
    unordered_map<long long, long long> predecessors;

    for(long long vertex: G.getVertices()){
      distances[vertex] = INF;
    }
    distances[start] = 0.0;

    while(!worklist.empty()){
      pair<long long, double> curr = worklist.top();
      double currDist = distances[curr.first];
      worklist.pop();

      for(long long neighbor: G.neighbors(curr.first)){
        if(neighbor == target || !ignoreNodes.count(neighbor)){
          pair<long long, double> n;
          n.first = neighbor;
          G.getWeight(curr.first, neighbor, n.second);
          n.second += currDist;
          if(distances[neighbor] > n.second){
            distances[neighbor] = n.second;
            worklist.push(n);
            predecessors[neighbor] = curr.first;
          }
        }
      }
    }
    if(predecessors.count(target) != 1){
      return vector<long long>{};
    }
    else {
      long long currVert = target;
      vector<long long> reverse;
      reverse.push_back(target);
      while(start != currVert){
        currVert = predecessors[currVert];
        reverse.push_back(currVert);
      }
      vector<long long> finalResult;
      for(int i = reverse.size()-1; i>=0; i--){
        finalResult.push_back(reverse[i]);
      }
      return finalResult;
    }
}

double pathLength(const graph<long long, double> &G,
                  const vector<long long> &path) {
  double length = 0.0;
  double weight;
  for (size_t i = 0; i + 1 < path.size(); i++) {
    bool res = G.getWeight(path.at(i), path.at(i + 1), weight);
    if (!res) {
      return -1;
    }
    length += weight;
  }
  return length;
}

void outputPath(const vector<long long> &path) {
  for (size_t i = 0; i < path.size(); i++) {
    cout << path.at(i);
    if (i != path.size() - 1) {
      cout << "->";
    }
  }
  cout << endl;
}

// Honestly this function is just a holdover from an old version of the project
void application(const vector<BuildingInfo> &buildings,
                 const graph<long long, double> &G) {
  string person1Building, person2Building;

  set<long long> buildingNodes;
  for (const auto &building : buildings) {
    buildingNodes.insert(building.id);
  }

  cout << endl;
  cout << "Enter person 1's building (partial name or abbreviation), or #> ";
  getline(cin, person1Building);

  while (person1Building != "#") {
    cout << "Enter person 2's building (partial name or abbreviation)> ";
    getline(cin, person2Building);

    // Look up buildings by query
    BuildingInfo p1 = getBuildingInfo(buildings, person1Building);
    BuildingInfo p2 = getBuildingInfo(buildings, person2Building);
    Coordinates P1Coords, P2Coords;
    string P1Name, P2Name;

    if (p1.id == -1) {
      cout << "Person 1's building not found" << endl;
    } else if (p2.id == -1) {
      cout << "Person 2's building not found" << endl;
    } else {
      cout << endl;
      cout << "Person 1's point:" << endl;
      cout << " " << p1.name << endl;
      cout << " " << p1.id << endl;
      cout << " (" << p1.location.lat << ", " << p1.location.lon << ")" << endl;
      cout << "Person 2's point:" << endl;
      cout << " " << p2.name << endl;
      cout << " " << p2.id << endl;
      cout << " (" << p2.location.lon << ", " << p2.location.lon << ")" << endl;

      Coordinates centerCoords = centerBetween2Points(p1.location, p2.location);
      BuildingInfo dest = getClosestBuilding(buildings, centerCoords);

      cout << "Destination Building:" << endl;
      cout << " " << dest.name << endl;
      cout << " " << dest.id << endl;
      cout << " (" << dest.location.lat << ", " << dest.location.lon << ")"
           << endl;

      vector<long long> P1Path = dijkstra(G, p1.id, dest.id, buildingNodes);
      vector<long long> P2Path = dijkstra(G, p2.id, dest.id, buildingNodes);

      // This should NEVER happen with how the graph is built
      if (P1Path.empty() || P2Path.empty()) {
        cout << endl;
        cout << "At least one person was unable to reach the destination "
                "building. Is an edge missing?"
             << endl;
        cout << endl;
      } else {
        cout << endl;
        cout << "Person 1's distance to dest: " << pathLength(G, P1Path);
        cout << " miles" << endl;
        cout << "Path: ";
        outputPath(P1Path);
        cout << endl;
        cout << "Person 2's distance to dest: " << pathLength(G, P2Path);
        cout << " miles" << endl;
        cout << "Path: ";
        outputPath(P2Path);
      }
    }

    //
    // another navigation?
    //
    cout << endl;
    cout << "Enter person 1's building (partial name or abbreviation), or #> ";
    getline(cin, person1Building);
  }
}
