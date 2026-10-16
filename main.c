#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define V 50
#define MAX_E 1225
#define REPEAT 20000   

typedef struct {
    int u, v, w;
} Edge;

typedef struct Node {
    int vertex, weight;
    struct Node* next;
} Node;

Node* head[V];
Edge edges[MAX_E];         
int matrix[V][V];
int edgeCount = 0;

Edge sparseEdges[MAX_E], denseEdges[MAX_E];

void generateGraph(Edge out[], int target) {
    static int used[V][V];
    memset(used, 0, sizeof(used));
    int cnt = 0;

    for (int i = 0; i < V - 1; i++) {
        out[cnt++] = (Edge){ i, i + 1, rand() % 100 + 1 };
        used[i][i + 1] = used[i + 1][i] = 1;
    }

    while (cnt < target) {
        int u = rand() % V;
        int v = rand() % V;
        if (u != v && !used[u][v]) {
            out[cnt++] = (Edge){ u, v, rand() % 100 + 1 };
            used[u][v] = used[v][u] = 1;
        }
    }
}

void loadGraph(Edge src[], int count) {
    for (int i = 0; i < count; i++) edges[i] = src[i];
    edgeCount = count;
}

void buildList(void) {
    for (int i = 0; i < V; i++)
        head[i] = NULL;

    for (int i = 0; i < edgeCount; i++) {
        int u = edges[i].u, v = edges[i].v, w = edges[i].w;

        Node* a = malloc(sizeof(Node));
        Node* b = malloc(sizeof(Node));
        if (a == NULL || b == NULL) {
            printf("Memory allocation error!\n");
            exit(1);
        }

        *a = (Node){ v, w, head[u] };
        *b = (Node){ u, w, head[v] };
        head[u] = a;
        head[v] = b;
    }
}

void buildMatrix(void) {
    memset(matrix, 0, sizeof(matrix));
    for (int i = 0; i < edgeCount; i++) {
        matrix[edges[i].u][edges[i].v] = edges[i].w;
        matrix[edges[i].v][edges[i].u] = edges[i].w;
    }
}

int parent[V], rankSet[V];

int find(int x) {
    if (parent[x] != x)
        parent[x] = find(parent[x]);
    return parent[x];
}

void unite(int a, int b) {
    a = find(a);
    b = find(b);
    if (a == b) return;

    if (rankSet[a] < rankSet[b]) {
        parent[a] = b;
    }
    else if (rankSet[a] > rankSet[b]) {
        parent[b] = a;
    }
    else {
        parent[b] = a;
        rankSet[a]++;
    }
}

int compareEdges(const void* a, const void* b) {
    const Edge* x = (const Edge*)a;
    const Edge* y = (const Edge*)b;
    if (x->w < y->w) return -1;
    if (x->w > y->w) return 1;
    return 0;
}

int kruskal(Edge mst[], int* mstCount) {
    static Edge sorted[MAX_E];
    int m = 0;

    for (int u = 0; u < V; u++)
        for (Node* p = head[u]; p != NULL; p = p->next)
            if (u < p->vertex)                      
                sorted[m++] = (Edge){ u, p->vertex, p->weight };

    qsort(sorted, m, sizeof(Edge), compareEdges);

    for (int i = 0; i < V; i++) {
        parent[i] = i;
        rankSet[i] = 0;
    }

    int count = 0;
    int cost = 0;

    for (int i = 0; i < m && count < V - 1; i++) {
        int u = sorted[i].u;
        int v = sorted[i].v;

        if (find(u) != find(v)) {
            unite(u, v);
            mst[count++] = sorted[i];
            cost += sorted[i].w;
        }
    }

    *mstCount = count;
    return cost;
}

int prim(Edge mst[], int* mstCount) {
    int key[V], prev[V], visited[V] = { 0 };

    for (int i = 0; i < V; i++) {
        key[i] = 1000000;
        prev[i] = -1;
    }

    key[0] = 0;
    int cost = 0;
    int count = 0;

    for (int i = 0; i < V; i++) {
        int u = -1;
        int min = 1000000;

        for (int j = 0; j < V; j++) {
            if (!visited[j] && key[j] < min) {
                min = key[j];
                u = j;
            }
        }

        if (u == -1) break;

        visited[u] = 1;
        cost += key[u];

        if (prev[u] != -1)
            mst[count++] = (Edge){ prev[u], u, key[u] };

        for (int v = 0; v < V; v++) {
            if (matrix[u][v] != 0 && !visited[v] && matrix[u][v] < key[v]) {
                key[v] = matrix[u][v];
                prev[v] = u;
            }
        }
    }

    *mstCount = count;
    return cost;
}

void printMST(Edge mst[], int count) {
    printf("MST Edge List:\n");
    for (int i = 0; i < count; i++)
        printf("  %d - %d (weight %d)\n", mst[i].u, mst[i].v, mst[i].w);
}

void freeList(void) {
    for (int i = 0; i < V; i++) {
        Node* p = head[i];
        while (p != NULL) {
            Node* temp = p;
            p = p->next;
            free(temp);
        }
        head[i] = NULL;
    }
}

void printResult(int caseNo, const char* title, int count, int cost,
    size_t memory, double ms, Edge mst[]) {
    printf("\nCase %d : %s\n", caseNo, title);
    printf("Vertices      : %d\n", V);
    printf("Edges         : %d\n", edgeCount);
    printf("MST Edges     : %d\n", count);
    printf("MST Cost      : %d\n", cost);
    printf("Memory        : %zu Bytes\n", memory);
    printf("Execution Time: %.6f ms (average of %d runs)\n", ms, REPEAT);
    printMST(mst, count);
}

int runKruskal(int caseNo, const char* title) {
    Edge mst[V - 1];
    int count = 0, cost = 0;
    volatile int sink = 0;

    buildList();

    clock_t start = clock();
    for (int r = 0; r < REPEAT; r++) {
        cost = kruskal(mst, &count);
        sink += cost;
    }
    clock_t end = clock();
    double ms = 1000.0 * (end - start) / CLOCKS_PER_SEC / REPEAT;

    size_t memory =
        sizeof(head) +                            
        (size_t)(2 * edgeCount) * sizeof(Node) +    
        (size_t)edgeCount * sizeof(Edge) +         
        sizeof(parent) + sizeof(rankSet) +        
        sizeof(mst);                               

    printResult(caseNo, title, count, cost, memory, ms, mst);
    freeList();
    return cost;
}

int runPrim(int caseNo, const char* title) {
    Edge mst[V - 1];
    int count = 0, cost = 0;
    volatile int sink = 0;

    buildMatrix();

    clock_t start = clock();
    for (int r = 0; r < REPEAT; r++) {
        cost = prim(mst, &count);
        sink += cost;
    }
    clock_t end = clock();
    double ms = 1000.0 * (end - start) / CLOCKS_PER_SEC / REPEAT;

    size_t memory =
        sizeof(matrix) +                            
        sizeof(int) * V * 3 +                      
        sizeof(mst);                                

    printResult(caseNo, title, count, cost, memory, ms, mst);
    return cost;
}

int main(void) {
    srand((unsigned int)time(NULL));

    generateGraph(sparseEdges, 100);
    generateGraph(denseEdges, 600);

    loadGraph(sparseEdges, 100);
    int c1 = runKruskal(1, "Sparse Graph - Adjacency List - Kruskal");

    loadGraph(denseEdges, 600);
    int c2 = runKruskal(2, "Dense Graph - Adjacency List - Kruskal");

    loadGraph(sparseEdges, 100);
    int c3 = runPrim(3, "Sparse Graph - Adjacency Matrix - Prim");

    loadGraph(denseEdges, 600);
    int c4 = runPrim(4, "Dense Graph - Adjacency Matrix - Prim");

    printf("\n[Check] Sparse: Kruskal=%d, Prim=%d -> %s\n", c1, c3, c1 == c3 ? "OK" : "MISMATCH");
    printf("[Check] Dense : Kruskal=%d, Prim=%d -> %s\n", c2, c4, c2 == c4 ? "OK" : "MISMATCH");
    return 0;
}