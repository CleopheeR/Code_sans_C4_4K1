import copy
import time
from graph_tool.all import *


def CycleQuatre():
#Retourne un cycle de taille 4
    g=Graph(directed=False)
    g.add_vertex(4)
    g.add_edge(0,1)
    g.add_edge(0,3)
    g.add_edge(1,2)
    g.add_edge(2,3)
    return g



def StableQuatre(): 
#Retourne un stable de taille 4 
    g=Graph(directed=False)
    g.add_vertex(4)
    return g 



def graphe_icosaedre():
#Retourne un icosaedre 
    g=Graph(directed=False)
    g.add_vertex(12)
    for i in range(1,6):
        g.add_edge(0, i)
        if i!=1 :
            g.add_edge(i-1, i)
        g.add_edge(i,i+5)
        g.add_edge(i,i+4)
        g.add_edge(5+i,11)
        if i!=5 :
            g.add_edge(5+i,6+i)
    g.add_edge(1,10)
    g.add_edge(10,6)
    return g 

def graphe_icosaedre_moins_1():
    #Retourne un icosaedre 
    g=Graph(directed=False)
    g.add_vertex(11)
    for i in range(1,6):
        g.add_edge(0, i)
        if i!=1 :
            g.add_edge(i-1, i)
        g.add_edge(i,i+5)
        g.add_edge(i,i+4)
        if i!=5 :
            g.add_edge(5+i,6+i)
    g.add_edge(1,10)
    g.add_edge(10,6)
    return g 




def CycleHuit():
    g=Graph(directed=False)
    g.add_vertex(8)
    g.add_edge(0,1)
    g.add_edge(1,2)
    g.add_edge(3,2)
    g.add_edge(4,3)
    g.add_edge(4,5)
    g.add_edge(5,6)
    g.add_edge(6,7)
    g.add_edge(0,7)

    return g

def CycleSept():
    g=Graph(directed=False)
    g.add_vertex(7)
    g.add_edge(0,1)
    g.add_edge(1,2)
    g.add_edge(3,2)
    g.add_edge(4,3)
    g.add_edge(4,5)
    g.add_edge(5,6)
    g.add_edge(0,6)

    return g

def CycleCinq():
    g=Graph(directed=False)
    g.add_vertex(5)
    g.add_edge(0,1)
    g.add_edge(1,2)
    g.add_edge(3,2)
    g.add_edge(4,3)
    g.add_edge(4,0)

    return g
