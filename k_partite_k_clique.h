#ifndef KPartiteKClique_header
#define KPartiteKClique_header

#include <cstdint>
#include <vector>
#include <iostream>
#include <cassert>
using namespace std;
#define DBG 0

class Bitset;

class Bitset {

    public:
        Bitset(int n_vertices, bool fill=false);
        Bitset(const bool* set_bits, int n_vertices);
        Bitset(const Bitset& obj){
            // The code does not create shallow copies of bitsets.
            assert(0);
        }
        ~Bitset();
        void unset(int index);
        bool has(int index);
        void set(int index);
        int intersection_count(Bitset& r, int start, int stop);
        void intersection_assign(Bitset& l, Bitset& r);
    private:
        uint64_t* data;
        int limbs;
        void allocate(int n_vertices);
        uint64_t& operator[](int i){ return data[i]; }
};

class KPartiteKClique {
        class Vertex {
            inline friend bool operator<(const Vertex& l, const Vertex& r){
                // The lower the weight, the higher the obstruction when
                // selecting this vertex.
                // We want to select vertices with high obstruction first
                // and put the last (to be popped first).
                return l.weight > r.weight;
            }
            inline friend void intersection(Bitset& c, Vertex& l, Bitset& r){
                c.intersection_assign(*(l.bitset), r);
            }

            public:
                int index;  // The index in the original graph.
                int part;  // The part in the orginal graph.
                int weight;  // The higher, the higher the likelihood of a k-clique with this vertex.

                Vertex();
                Vertex(const Vertex& obj);
                void init(KPartiteKClique* problem, const bool* incidences, int n_vertices, int part, int index);
                ~Vertex();
                bool set_weight();
                inline int intersection_count(Bitset& r, int start, int stop){
                    return bitset->intersection_count(r, start, stop);
                }
                inline int intersection_count(Bitset& r, int part){
                    return intersection_count(r, get_parts()[part], get_parts()[part+1]);
                }

            private:
                bool is_shallow;
                Bitset* bitset;
                KPartiteKClique* problem;
                const int* get_parts() { return problem->parts; }
                const int get_k() { return problem->k; }
                Bitset& get_active_vertices() { return *(problem->current_graph()).active_vertices; }
        };

    class KPartiteGraph {
        public:
            vector<Vertex> vertices;
            Bitset* active_vertices;

            KPartiteGraph();
            void init(KPartiteKClique* problem, bool fill);
            ~KPartiteGraph();
            Vertex* last_vertex();
            void pop_last_vertex();
            bool is_valid();
            inline bool set_weights(){
                bool new_knowledge = false;
                for(Vertex& v: vertices){
#if DBG
                    cout << "set weight of " << v->index << endl;
#endif
                    new_knowledge |= v.set_weight();
#if DBG
                    cout << "weight is " << v->weight << endl;
#endif
                }
                return new_knowledge;
            }
            bool select(KPartiteGraph& next);
            inline int count_active_vertices(int start, int stop){
                return active_vertices->intersection_count(*active_vertices, start, stop);
            }
            inline int count_active_vertices(int part){
                return count_active_vertices(get_parts()[part], get_parts()[part+1]);
            }
        private:
            const int* get_parts() { assert(problem); return problem->parts; }
            const int get_k() { assert(problem); return problem->k; }
            KPartiteGraph& current_graph(){ return problem->current_graph(); }
            KPartiteGraph& next_graph(){ return problem->next_graph(); }
            int* part_sizes;
            KPartiteKClique* problem;
    };

    public:
        const int* k_clique(){ return _k_clique; }
        KPartiteKClique(const bool* const* incidences, const int n_vertices, const int* first_per_part, const int k);
        KPartiteKClique();
        ~KPartiteKClique();
        bool next();
    private:
        int* _k_clique;
        int* parts;
        int k;
        int current_depth;
        int n_vertices;
        Vertex* all_vertices;
        KPartiteGraph* recursive_graphs;
        KPartiteGraph& current_graph(){ return recursive_graphs[current_depth]; }
        KPartiteGraph& next_graph(){ return recursive_graphs[current_depth + 1]; }
        bool traceback();
};

#endif
