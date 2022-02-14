import copy
import time
from graph_tool.all import *
from Gestion_Fichiers import *
from Special_graphs import *
import sys

def FreeO4(g, v1, adjMat):
    n = g.vertex_index[v1]+1
    i1 = g.vertex_index[v1]
    
    for i2 in range(n):
        #i2 = g.vertex_index[v2]
        if i2==i1:
            continue
        if not(adjMat[i2][i1]):
        #if (v2 in v1.out_neighbors())): 
            #for v3 in g.vertices():
            for i3 in range(n):
                #if not(v3 in v1.out_neighbors()) and (v3!=v1) and not (v3 in v2.out_neighbors()) and (v3!=v2):
                if i3 == i1 or i3 == i2:
                    continue
                if not(adjMat[i3][i1]) and  not (adjMat[i3][i2]):
                    #for v4 in g.vertices():
                    for i4 in range(n):
                        #if not (v4 in v1.out_neighbors()) and (v4!=v1) and not (v4 in v2.out_neighbors()) and (v4!=v2) and (not v4 in v3.out_neighbors()) and (v4!=v3):
                        if i4 == i1 or i4 == i2 or i4 == i3:
                            continue
                        if not (adjMat[i4][i1]) and not(adjMat[i4][i2]) and not(adjMat[i4][i3]):
                            return False


def FreeC4O4(g,v1, adjMat):
    n = g.vertex_index[v1]+1
    
    #Retourne true si v1 n'est pas inclus dans un C4
    i1 = g.vertex_index[v1]
    
    for v2 in v1.out_neighbors() :
        i2 = g.vertex_index[v2]
        if i2!=i1:
            for v3 in v1.out_neighbors() :
                i3 = g.vertex_index[v3]
                if i3==i1 or i3==i2:
                    continue
                if not(adjMat[i3][i2]):# in v2.out_neighbors())):
                    for v4 in v2.out_neighbors() :
                        i4 = g.vertex_index[v4]
                        if i4==i1 or i4==i2 or i4==i3:
                            continue
                        if adjMat[i4][i3] and not(adjMat[i4][i1]):##and (v4 in v3.out_neighbors()) and (not(v4 in v1.out_neighbors())):
                            return False
   
    for i2 in range(n):
        if i2==i1:
            continue
        if not(adjMat[i2][i1]):
            for i3 in range(i2+1,n):
                if i3 == i1 or i3 == i2:
                    continue
                if not(adjMat[i3][i1]) and  not (adjMat[i3][i2]):
                    for i4 in range(i3+1,n):
                        if i4 == i1 or i4 == i2 or i4 == i3:
                            continue
                        if not (adjMat[i4][i1]) and not(adjMat[i4][i2]) and not(adjMat[i4][i3]):
                            return False
    return True



def aunjumeau(g,v):
    """Attention v est dans g 
    #Regarde si v a un jumeau dans g. True si oui et False sinon """
    for w in v.out_neighbors():
        #Regarde si w est  un jumeau de v
        jumeau=True # w est il jumeau?
        for x in w.out_neighbors():
            #Prend tout les voisins de w et verifie que c'est un voisin de v
            if x!=v and not(x in v.out_neighbors()):
                #Si il y a un non voisin 
                jumeau=False
                break # Si j'en ai trouve un je veux tout de suite sortir de la boucle. 
        if jumeau : # Si j'ai une difference avant, je ne veux pas savoir apres. 
            for x in v.out_neighbors():
                #Prend tout les voisins de v et verifie que c'est un voisin de w
                if x!=w and not(x in w.out_neighbors()):
                    #Si il y a un non voisin 
                    jumeau=False 
                    break 
        if jumeau :
            return True 
            #La fonction s'arrette si j'ai trouve un jumeau 
    #Si je ne me suis pas arreter c'est qu'il n'y a pas de jumeau 
    return False 



def sousens_nonord_k(k, n):
    """Retourne tout les sous ensmbles nonordones de taille k de nombres de 1 a n"""
    L=[]
    k=int(k)
    n=int(n)
    sousliste=list(range(1,k+1))
    i=k-1
    end=1
    if k!=0 and k<=n:
        while end!=0:
            i=k-1
            #Stockage de la derniere sous liste 
            Aaccrocher=copy.copy(sousliste)
            L.append(Aaccrocher)
            #changement de la derniere sous liste 
            #Reccuperation du curseur qui n'est pas sature
            while sousliste[i]==n-k+i+1 and i!=-1:
                i=i-1
            #Changement de la liste
            if i==-1 :
                end=0
            else:
                sousliste[i]=sousliste[i]+1
                for z in range(i+1,k):
                    sousliste[z]=sousliste[z-1]+1

    return L



def attacheavecnombrefixedarretes(g,m, adjMat):
    """Fonction qui test si on peut rajoute un sommet v de degre m a g. 
    Si le graphe resultant n'appartiens pas a Free(C4;O4)
    ou que v a un jumeau ou qu'il est complet a g
    retourner TRUE sinon FALSE"""
    if m==0:
        G=copy.copy(g)
        v=G.add_vertex()
        if FreeC4O4(G,v, adjMat)==True :
            if aunjumeau(G,v)==False:
                return False
        return True 

    else:
        #Obtention du nombre de sommets de g
        n=0
        for v in g.vertices(): 
            n=n+1
        #Pour tout les ensembles ordones de taille m d'entier de 1 a taille(n) P
        LP=sousens_nonord_k(m,n)
        for P in LP :
            G=copy.copy(g)
            v=G.add_vertex()
            #Rajouter toutes les aretes entre v et les sommets de g correspondants aux indices de P
            for i in P :
                w = G.vertex(i-1)
                G.add_edge(v,w)
            #Tester g 
            if FreeC4O4(G,v, adjMat)==True :
                if aunjumeau(G,v)==False:
                    for w in g.vertices():
                        if not(w in v.out_neighbors()):
                            #de que c possible on s'arrete
                            return False
        return True 


def Ets_Blowup(g):
    """Test si pour tout graph contenant g  comme sous graph est un blowup"""

    #Reccupere le nombre de sommets
    n=0
    
    for v in g.vertices():
        n=n+1
    #Test pour toutes les combinaison de 1 a n 
    k=0
    G=copy.copy(g)
    while k<n+1:
        if attacheavecnombrefixedarretes(G,k)== False:
            return False
        k=k+1
    return True 



###########################
# FONCTION DE GENERATION  #
###########################



def ReccupDegre(g):
    """Renvois un tupple de tout les degres de g par ordre croissant"""
    ListeDeDegre=[]
    for v in g.vertices():
        ListeDeDegre.append(v.out_degree())
    ListeDeDegre=sorted(ListeDeDegre)
    return tuple(ListeDeDegre)


def ToutLesGraphsAnSommets(n):
    """Renvois une liste de graphs a n sommets appartenants a Free(C4;O4)"""
    L=[]
    #On part d'un graphs a n-1 sommets 
    if n==1:
        g=Graph(directed=False)
        g.add_vertex()
        L.append(g)
    elif n==2:
        g2NA=Graph(directed=False)
        v1=g2NA.add_vertex()
        v2=g2NA.add_vertex()
        L.append(g2NA)
        g2A=Graph(directed=False)
        v1A=g2A.add_vertex()
        v2A=g2A.add_vertex()
        g2A.add_edge(v2A,v1A)
        L.append(g2A)      
    else  : 
        lnmoinsun=ToutLesGraphsAnSommets(n-1)
        print("j'ai genere les graphes a ", n-1, " sommets")
        L=[]
        Dico = {}
        #Je genere tout les ensembles ordones de taille m d'entier de 1 a taille(n) P. Ils vont etre les somemts a attacher 
        LP=[]
        for m in range(1,n):
            LP=LP+sousens_nonord_k(m,n-1)
        print("je dois racrocher un sommets a ", len(lnmoinsun), 'graphes')
        for i in range(len(lnmoinsun)):
            if i%10==0:
                print("Nous somme sur le ",i+1,"eme graph sur", len(lnmoinsun), "graphes")

            g=copy.copy(lnmoinsun[i])
            #g contiens n-1 commets 

            #Regardons d'abord sans arrete suplementaire 
            GNonConnex=copy.copy(g)
            q=GNonConnex.add_vertex()
            for (u,v) in g.get_edges():
                adjMat[u][v] = True
                adjMat[v][u] = True
            if FreeC4O4(GNonConnex,q, adjMat)==True:
                Dico[ReccupDegre(GNonConnex)]=[GNonConnex]
                
            #J'atteche les sommets
            lToRemove = [] # liste des arêtes de l'adjacency matrix à enlever au prochain tour de boucle
            for P in LP :
    
                #Creer une copie G de g
                G=copy.copy(g)
                #Rajouter un sommet a G
                v=G.add_vertex()
                #Rajouter toutes les aretes entre v et les sommets de g correspondants aux indices de P
                for (u,v) in lToRemove:
                    adjMat[u][v] = False
                    adjMat[v][u] = False

                lToRemove = []
                for i in P :

                    w = G.vertex(i-1)
                    G.add_edge(v,w)
                    
                    adjMat[n-1][i-1] = True
                    adjMat[i-1][n-1] = True
                    lToRemove.append((n-1, i-1))

                #Tester g dans FreeC4O4 et aussi pas isomorphe a un autre 
                if FreeC4O4(G,v, adjMat)==True:
                    LDG=ReccupDegre(G)
                    ilyalememedansDico=False
                    #si la cle existe je dois verifier si il y a un isomorphisme
                    if (LDG in Dico)==True:
                        ListeAVerifer=Dico.get(LDG)
                        for g2 in ListeAVerifer:
                             if graph_tool.topology.isomorphism(G,g2)==True:
                                ilyalememedansDico=True
                                break
                        if ilyalememedansDico==False:
                            Dico[LDG].append(G)
                    #Sinon je cree la cle
                    else: 
                        Dico[LDG]=[G]

        #Je reccupere la liste 
        for cle in Dico :
            L=L+Dico.get(cle)

        return L  


def ToutLesGraphsAnSommets_Par_Fichier(n):
    """Cette fonction renvois une liste de graphs a n sommets appartenants a Free(C4;O4)"""
    #Je regarde si ceux d'avant ont etes creaient sinon je fait un appelle par reccurent 
    L=[]
    if n==1:
        g=Graph(directed=False)
        g.add_vertex()
        L.append(g) 
    elif n==2:
        g2NA=Graph(directed=False)
        v1=g2NA.add_vertex()
        v2=g2NA.add_vertex()
        L.append(g2NA)
        g2A=Graph(directed=False)
        v1A=g2A.add_vertex()
        v2A=g2A.add_vertex()
        g2A.add_edge(v2A,v1A)
        L.append(g2A)      
    else  : 
        if Verifie_est_dans_sommaire(n-1):
            lnmoinsun=Lecture_Graph(n-1)
        else :
            lnmoinsun=ToutLesGraphsAnSommets(n-1)

        print("j'ai genere les graphs a ", n-1, " sommets")
        L=[]
        Dico = {}
        #Je genere tout les ensembles ordones de taille m d'entier de 1 a taille(n) P. Ils vont etre les somemts a attacher 
        LP=[[]]
        for m in range(1,n):
            LP=LP+sousens_nonord_k(m,n-1)


        print("je dois racrocher un sommets a ", len(lnmoinsun), 'graphes')
        for i in range(len(lnmoinsun)):
            if i%10==0:
                print("Nous somme sur le ",i+1,"eme graphe sur", len(lnmoinsun), "graphes")

            g=copy.copy(lnmoinsun[i])
            adjMat = [[False]*n for i in range(n)]
            #g contiens n-1 commets 

            #Regardons d'abord sans arrete suplementaire 
            GNonConnex=copy.copy(g)
            q=GNonConnex.add_vertex()
            for (u,v) in g.get_edges():
                adjMat[u][v] = True
                adjMat[v][u] = True
            if FreeC4O4(GNonConnex,q, adjMat)==True: #TODO on cherche un O3...
                Dico[ReccupDegre(GNonConnex)]=[GNonConnex]
                
            #J'atteche les sommets 
            lToRemove = [] # liste des arêtes de l'adjacency matrix à enlever au prochain tour de boucle
            for P in LP :
                #Creer une copie G de g
                G=copy.copy(g)
                #Rajouter un sommet a G
                v=G.add_vertex()
                #Rajouter toutes les aretes entre v et les sommets de g correspondants aux indices de P
                for (uu,vv) in lToRemove:
                    adjMat[uu][vv] = False
                    adjMat[vv][uu] = False

                lToRemove = []
                
                for i in P :
                    w = G.vertex(i-1)
                    G.add_edge(v,w)
                    assert(G.vertex_index[v] == n-1 and G.vertex_index[w] == i-1)
                    adjMat[n-1][i-1] = True
                    adjMat[i-1][n-1] = True
                    lToRemove.append((n-1, i-1))
                #Tester g dans FreeC4O4 et aussi pas isomorphe a un autre 
                if FreeC4O4(G,v, adjMat)==True:
                    LDG=ReccupDegre(G)
                    ilyalememedansDico=False
                    #si la cle existe je dois verifier si il y a un isomorphisme
                    if (LDG in Dico)==True:
                        ListeAVerifer=Dico.get(LDG)
                        for g2 in ListeAVerifer:
                             if graph_tool.topology.isomorphism(G,g2)==True:
                                ilyalememedansDico=True
                                break
                        if ilyalememedansDico==False:
                            Dico[LDG].append(G)
                    #Sinon je cree la cle
                    else: 
                        Dico[LDG]=[G]

        #Je reccupere la liste 
        for cle in Dico :
            L=L+Dico.get(cle)
    #Je range dans le fichier
    #Faut l'effacer avant
    for i in range(len(L)):
        Graph_dans_bon_fichier(L[i],n)
    return L  


def Est_Blow_up_sans_generer(n):
    #Reccupere les graphes par les fichiers, lest tests et les elimines au fur et a mesure.
    if Verifie_est_dans_sommaire(n):
            L=Lecture_Graph(n)
    else :
            L=ToutLesGraphsAnSommets_Par_Fichier(n)
    LFinale=[]
    print("J'ai", len(L), "graphes a tester")

    while len(L)!=0:
        if len(L)%100==0:
            print("Il reste", len(L),"graphes a tester")
        Gquontest=copy.copy(L[0])
        if Ets_Blowup(L[0]):
            LFinale.append(Gquontest)
        del L[0]
        
    return LFinale






def main():
    n=int(sys.argv[1])
    ToutLesGraphsAnSommetstest = sys.argv[2]
    if ToutLesGraphsAnSommetstest == "G":
        L=ToutLesGraphsAnSommets_Par_Fichier(n)
        print("Il y a ",len (L)," graphes de taille ",n)
        Jevoislesgraphs="N"#input("Voulez-vous les voirs ? (Y or N)")
        if Jevoislesgraphs=="Y" : 
            for i in range(len(L)) :
                print(L[i])
                for e in L[i].edges():
                    print(e) 
    else : 
        L=Est_Blow_up_sans_generer(n)
        if len(L)==0:
            print("Pas de blowup")
        else:
            Nouveau_fichier_special(n)
            for i in range(len(L)):
                print(L[i])
                for e in L[i].edges():
                    print(e)
            #Je range dans le fichier
        for i in range(len(L)):
            Graph_dans_bon_fichier_Special(L[i],n)
       
    print("Pour rappel : ", n)


import cProfile

if __name__=="__main__":
    main()
    #cProfile.run('main()')
    #G=graphe_icosaedre_moins_1()
    #print(G)
    #for e in G.edges():
     #   print(e)

