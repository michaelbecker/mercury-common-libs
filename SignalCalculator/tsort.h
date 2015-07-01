/**
 *  @ref    http://en.wikipedia.org/wiki/Topological_sorting
 */
#ifndef TSORT_H__
#define TSORT_H__

#include <list>
#include <stdlib.h>

#include <iomanip>
#include <iostream>

using namespace std;


/**
 *  Vertex contains data and a list of other vertices.  
 *  This implements part of an Adjacency List.
 */
template <typename T>
class Vertex {

    public:

        /**
         *  Data contained in this Node
         */
        T VertexData;

        /**
         *  List of the other nodes this one connects to
         */
        list<T> Edges;

        /**
         *  ctor
         */
        Vertex(T data)
            : VertexData(data)
        {};

        /**
         *  Convience wrapper function.
         */
        void AddEdge(T data)
        {
            Edges.push_back(data);
        }

    
    /**
     *  Helper stream function to print the class internals.
     */
    template <typename sT>
    friend ostream& operator<<(ostream& os, const Vertex<sT>& v);
};


/**
 *  Definition of Helper stream function to print the class internals.
 */
template <typename sT>
ostream& operator<<(ostream& os, const Vertex<sT>& v)
{
    //----------------------------------
    typename list<sT>::iterator it;
    list<sT> edges = v.Edges;
    //----------------------------------

    os << "  Vertex: " << setw(4) << v.VertexData ;
    os << "  Edges: ";

    for (   it = edges.begin();
            it != edges.end();
            it++){
        sT e = *it;
        os << setw(4) << e << " ";
    }

    os << " - ";

    return os;
}


/**
 *  Implements a DAG using an Adjancy list, which here is a list of 
 *  Vertex objects.
 */
template <typename T>
class DirectedAcyclicGraph {

    private:
        /**
         *  Assignment operator - no public access.
         */
        DirectedAcyclicGraph& operator= (const DirectedAcyclicGraph &g);
            

    public:
        /**
         *  List of all the vertices in this graph
         */
        list< Vertex<T>* > VertexList;

        /**
         *  Add a vertex when constructing a graph
         */
        Vertex<T>* AddVertex(T data)
        {
            Vertex<T>* v = new Vertex<T>(data);
            VertexList.push_back(v);
            return v;
        }
        
        /**
         *  Constructor
         */
        DirectedAcyclicGraph()
        {};

        /**
         *  Copy constructor, needed for Topologic Sort.
         */
        DirectedAcyclicGraph(const DirectedAcyclicGraph <T>&g)
        : VertexList(g.VertexList)
        {}

        /**
         *  Given a vertex in the DAG, see if there are any incoming edges.
         */
        bool HasIncommingEdges(Vertex<T>& InputVertex)
        {
            //-------------------------------------------
            typename list < Vertex<T>* >::iterator vit;
            typename list <T>::iterator eit;
            //-------------------------------------------

            for (   vit = VertexList.begin();
                    vit != VertexList.end();
                    vit++){

                Vertex<T>* v = *vit;

                for (   eit = v->Edges.begin();
                        eit != v->Edges.end();
                        eit++){

                    T e = *eit;

                    if (e == InputVertex.VertexData){
                        return true;
                    }
                }
            }

            return false;
        }

        /**
         *  Given a data element, look up the Vertex in the DAG
         */
        Vertex<T>* GetVertexFromEdge(T edge)
        {
            //-------------------------------------------
            typename list < Vertex<T>* >::iterator vit;
            //-------------------------------------------

            for (   vit = VertexList.begin();
                    vit != VertexList.end();
                    vit++){

                Vertex<T>* v = *vit;

                if (edge == v->VertexData){
                    return v;
                }
            }

            //
            //  This "could" happen if the user forgot to add a 
            //  vertex that correlates to an edge.
            //
            return NULL;
        }

        /**
         *  Does this DAG have any edges at all?
         */
        bool HasEdges()
        {
            //-------------------------------------------
            typename list < Vertex<T>* >::iterator vit;
            //-------------------------------------------

            for (   vit = VertexList.begin();
                    vit != VertexList.end();
                    vit++){

                Vertex<T>* v = *vit;

                if (!v->Edges.empty()){
                    return true;
                }
            }

            return false;
        }


    /**
     *  Helper function to print contents of a DirectedAcyclicGraph object.
     */
    template <typename sT>
    friend ostream& operator<<(ostream& os, DirectedAcyclicGraph<sT>& g);
};



/**
 *  Definition of helper function to print contents of a 
 *  DirectedAcyclicGraph object.
 */
template <typename sT>
ostream& operator<<(ostream& os, DirectedAcyclicGraph<sT>& g)
{
    //-------------------------------------------
    typename list < Vertex<sT>* >::iterator it;
    //-------------------------------------------

    os  << "DirectedAcyclicGraph (" << &g << ")\n";

    for (   it = g.VertexList.begin();
            it != g.VertexList.end();
            it++){
        Vertex<sT>*v = *it;
        os << *v;

        if (g.HasIncommingEdges(*v)){
            os << " Has incomming edges\n";
        }
        else{
            os << " NO incomming edges\n";
        }
    }

    return os;
}


/**
 *  Utility class encapsulating a topological sort.
 *  Designed to be called only once!
 */
template <typename T>
class TopologicalSort {

    private:
        /**
         *  Set of all nodes that have no incomming edges.
         */
        list < Vertex<T>* > S;

        /**
         *  List that contains the sorted elements.
         */
        list < Vertex<T>* > L;

        DirectedAcyclicGraph<T>* Dag;


    public:
        /**
         *  Contructor
         */
        TopologicalSort( DirectedAcyclicGraph<T>& g )
        {
            //
            //  Make a copy of the dag, because the sort is destructive to 
            //  the data structure.
            //
            Dag = new DirectedAcyclicGraph<T>(g);
        };

        /**
         *  Destructor
         */
        ~TopologicalSort()
        {
            Vertex<T>* v;

            while (!Dag->VertexList.empty()){
                v = Dag->VertexList.front();
                Dag->VertexList.pop_front();
                delete v;
            }
            delete Dag;
        }

        /**
         *  This is the interesting function.
         */
        bool Sort()
        {
            //----------------------------------
            typename list < Vertex<T>* >::iterator vit;
            Vertex<T>* v;
            //----------------------------------

            //
            //  Preload S - the set of all nodes with no incoming edges
            //
            for (   vit = Dag->VertexList.begin();
                    vit != Dag->VertexList.end();
                    vit++){
                v = *vit;
                
                if (!Dag->HasIncommingEdges(*v)){
                    S.push_back(v);
                }
            }

            //
            //  Run the sort algorithm
            //
            while (!S.empty()){

                Vertex<T>* n = S.front();

                S.pop_front();

                L.push_back(n);

                while (!n->Edges.empty()){

                    T e = n->Edges.front();
                    n->Edges.pop_front();

                    Vertex<T>* m = Dag->GetVertexFromEdge(e);

                    //
                    //  This "could" happen if the user forgot to add a 
                    //  vertex that correlates to an edge.
                    //
                    if (m == NULL){
                        cout << "INVALID GRAPH!!!!" << endl;
                        exit(-1);
                    }

                    if (!Dag->HasIncommingEdges(*m)){
                        S.push_back(m);
                    }
                }
            }

            return !Dag->HasEdges();
        }

        /**
         *  Get a list of the sorted elements.
         */
        list<T> GetSortedList()
        {
            //-------------------------------------------
            typename list < Vertex<T>* >::iterator it;
            list<T> sortedList;
            //-------------------------------------------

            for (   it = L.begin();
                    it != L.end();
                    it++){
                sortedList.push_back( (*it)->VertexData );
            }

            return sortedList;
        }


    /**
     *  Helper stream function to print the class internals.
     */
    template <typename sT>
    friend ostream& operator<< (ostream& os, TopologicalSort<sT>& ts);
};



/**
 *  Definition of helper stream function to print the class internals.
 */
template <typename sT>
ostream& operator<< (ostream& os, TopologicalSort<sT>& ts)
{
    //-------------------------------------------
    typename list < Vertex<sT>* >::iterator it;
    //-------------------------------------------

    os << "TopologicalSort Object (" << &ts << ")\n";
    os << *ts.Dag;

    os << "S: ";
    for (   it = ts.S.begin();
            it != ts.S.end();
            it++){
        os << (*it)->VertexData << " ";
    }
    os << "\n";

    os << "L: ";
    for (   it = ts.L.begin();
            it != ts.L.end();
            it++){
        os << (*it)->VertexData << " ";
    }
    os << "\n";

    return os;
}

#endif


