#include <iostream>
#include <vector>
#include <fstream>
#include <stack>
#include <queue>
#include <memory>
#include <algorithm>

using namespace std;

constexpr long N = 10572827;
constexpr long NI = 21378770;
constexpr long NO = 24573071;
constexpr long NT = 10532115;

struct EdgeLabel {
    long txPos;
    long amount;
};

struct NodeLabel {
    long txId;
    long timestamp;
    long isCoinbase;
    long fee;
};

vector<vector<pair<NodeLabel, EdgeLabel>>> adj(N);
vector<vector<long>> adjUndirected(N);
vector<bool> visited, onStack;
vector<long> sum;
bool isAcyclic, stop;
long countPaths, pathLimit, totalFlow;
vector<long> currentPath;

void loadOutputs() {
    ifstream fin("outputs_ordered.txt");
    if (!fin) {
        cerr << "Errore: cannot open outputs_ordered.txt" << endl;
        return;
    }
    long txId, txPos, addressId, amount, scriptType;
    while (fin >> txId >> txPos >> addressId >> amount >> scriptType) {
        if (txId >= 0 && txId < N) {
            adj[txId].push_back({{-1, -1, -1, -1}, {txPos, amount}});
        }
    }
    fin.close();
    cout << "Outputs loaded successfully" << endl;
}

void loadInputs() {
    ifstream fin("inputs_ordered.txt");
    if (!fin) {
        cerr << "Errore: cannot open inputs_ordered.txt" << endl;
        return;
    }
    long txId, prevTxId, prevTxPos;
    while (fin >> txId >> prevTxId >> prevTxPos) {
        if (prevTxId >= 0 && prevTxId < N && 
            prevTxPos >= 0 && prevTxPos < adj[prevTxId].size()) {
            adj[prevTxId][prevTxPos].first.txId = txId;
        }
    }
    fin.close();
    cout << "Inputs loaded successfully" << endl;
}

void loadTransactions() {
    ifstream fin("transactions.txt");
    if (!fin) {
        cerr << "Errore: cannot open transactions.txt" << endl;
        return;
    }
    long timestamp, blockId, txId, isCoinbase, fee;
    while (fin >> timestamp >> blockId >> txId >> isCoinbase >> fee) {
        if (txId >= 0 && txId < N) {
            for (auto& p : adj[txId]) {
                p.first.timestamp = timestamp;
                p.first.isCoinbase = isCoinbase;
                p.first.fee = fee;
            }
        }
    }
    fin.close();
    cout << "Transactions loaded successfully" << endl;
}

void buildGraph() {
    cout << "Building graph..." << endl;
    loadOutputs();
    loadInputs();
    loadTransactions();
}

void buildUndirectedGraph() {
    cout << "Building undirected graph..." << endl;
    for (long i = 0; i < N; ++i) {
        for (const auto& edge : adj[i]) {
            if (edge.first.txId != -1) {
                adjUndirected[i].push_back(edge.first.txId);
                adjUndirected[edge.first.txId].push_back(i);
            }
        }
    }
}

long dfsComponentIterative(long start) {
    vector<bool> localVisited(N, false);
    stack<long> st;
    st.push(start);
    long size = 0;
    
    while (!st.empty()) {
        long u = st.top();
        st.pop();
        
        if (!localVisited[u]) {
            localVisited[u] = true;
            size++;
            for (long v : adjUndirected[u]) {
                if (!localVisited[v]) {
                    st.push(v);
                }
            }
        }
    }
    return size;
}

pair<long, long> largestComponent() {
    cout << "Searching largest connected component..." << endl;
    vector<bool> localVisited(N, false);
    long maxSize = 0, index = -1;
    
    for (long i = 0; i < N; ++i) {
        if (!localVisited[i] && !adj[i].empty()) {
            long size = 0;
            stack<long> st;
            st.push(i);
            
            while (!st.empty()) {
                long u = st.top();
                st.pop();
                
                if (!localVisited[u]) {
                    localVisited[u] = true;
                    size++;
                    for (long v : adjUndirected[u]) {
                        if (!localVisited[v]) {
                            st.push(v);
                        }
                    }
                }
            }
            
            if (size > maxSize) {
                maxSize = size;
                index = i;
            }
        }
    }
    return {maxSize, index};
}

long computeFlow(const vector<long>& path) {
    long flow = 0;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        long u = path[i];
        long v = path[i + 1];
        for (const auto& edge : adj[u]) {
            if (edge.first.txId == v) {
                flow += edge.second.amount;
                break;
            }
        }
    }
    return flow;
}

void dfsFlowIterative(long start, long threshold, long maxPaths) {
    struct State {
        long node;
        vector<long> path;
    };
    
    stack<State> st;
    st.push({start, {start}});
    countPaths = 0;
    
    while (!st.empty() && countPaths < maxPaths) {
        State current = st.top();
        st.pop();
        
        long u = current.node;
        vector<long> path = current.path;
        
        long flow = computeFlow(path);
        if (path.size() > 1 && flow >= threshold) {
            cout << "|  (Start) ";
            for (size_t i = 0; i < path.size(); ++i) {
                cout << path[i];
                if (i + 1 < path.size()) cout << " -> ";
            }
            cout << " | Flow: " << flow << " (End)\n";
            countPaths++;
        }
        
        for (const auto& edge : adj[u]) {
            if (edge.first.txId != -1) {
                vector<long> newPath = path;
                newPath.push_back(edge.first.txId);
                st.push({edge.first.txId, newPath});
            }
        }
    }
}

void dfsCycleCheckIterative(long start, vector<bool>& localVisited, vector<bool>& localOnStack) {
    stack<pair<long, bool>> st; // pair<node, isReturning>
    st.push({start, false});
    
    while (!st.empty() && isAcyclic) {
        auto [u, returning] = st.top();
        st.pop();
        
        if (returning) {
            localOnStack[u] = false;
        } else {
            if (localOnStack[u]) {
                isAcyclic = false;
                return;
            }
            if (!localVisited[u]) {
                localVisited[u] = true;
                localOnStack[u] = true;
                
                st.push({u, true}); // push return marker
                
                for (const auto& edge : adj[u]) {
                    long v = edge.first.txId;
                    if (v != -1) {
                        if (!localVisited[v]) {
                            st.push({v, false});
                        } else if (localOnStack[v]) {
                            isAcyclic = false;
                            return;
                        }
                    }
                }
            }
        }
    }
}

void checkAcyclicity() {
    cout << "Checking acyclicity..." << endl;
    isAcyclic = true;
    vector<bool> localVisited(N, false);
    vector<bool> localOnStack(N, false);
    
    for (long i = 0; i < N && isAcyclic; ++i) {
        if (!localVisited[i] && !adj[i].empty()) {
            dfsCycleCheckIterative(i, localVisited, localOnStack);
        }
    }
    cout << "|\n|  The graph is: " << (isAcyclic ? "acyclic.\n" : "cyclic.\n");
}

void filterGraph(long start) {
    cout << "Filtering graph..." << endl;
    vector<bool> localVisited(N, false);
    stack<long> st;
    st.push(start);
    
    while (!st.empty()) {
        long u = st.top();
        st.pop();
        
        if (!localVisited[u]) {
            localVisited[u] = true;
            for (long v : adjUndirected[u]) {
                if (!localVisited[v]) {
                    st.push(v);
                }
            }
        }
    }
    
    for (long i = 0; i < N; ++i) {
        if (!localVisited[i]) {
            adj[i].clear();
            adjUndirected[i].clear();
        }
    }
}

void mergeEdges() {
    cout << "Merging edges..." << endl;
    for (long i = 0; i < N; ++i) {
        if (!adj[i].empty()) {
            vector<pair<NodeLabel, EdgeLabel>> merged;
            vector<bool> mergedIndex(adj[i].size(), false);
            
            for (size_t j = 0; j < adj[i].size(); ++j) {
                if (!mergedIndex[j]) {
                    auto mergedEdge = adj[i][j];
                    for (size_t k = j + 1; k < adj[i].size(); ++k) {
                        if (!mergedIndex[k] && adj[i][j].first.txId == adj[i][k].first.txId) {
                            mergedEdge.second.amount += adj[i][k].second.amount;
                            mergedIndex[k] = true;
                        }
                    }
                    merged.push_back(mergedEdge);
                    mergedIndex[j] = true;
                }
            }
            adj[i] = merged;
        }
    }
}

void runAlgorithm(long startNode, long threshold, long maxPaths) {
    if (startNode < 0 || startNode >= N || adj[startNode].empty()) {
        cout << "Invalid start node: " << startNode << "\n";
        return;
    }
    cout << "|\n|  First " << maxPaths << " valid paths from node " << startNode << " with flow ≥ " << threshold << ":\n";
    dfsFlowIterative(startNode, threshold, maxPaths);
}

void initializeVectors() {
    visited.resize(N, false);
    onStack.resize(N, false);
    sum.resize(N, 0);
}

int main() {
    initializeVectors();
    
    cout << "=== Bitcoin Transaction Graph Analysis ===\n";
    buildGraph();
    buildUndirectedGraph();
    
    auto [size, startNode] = largestComponent();
    cout << "|\n|  Largest component starts at " << startNode << " with size " << size << ".\n";
    
    filterGraph(startNode);
    mergeEdges();
    checkAcyclicity();
    
    cout << "Counting valid paths...\n";
    runAlgorithm(183, 0, 100);
    
    cout << "|\n=== Analysis Complete ===\n";
    return 0;
}