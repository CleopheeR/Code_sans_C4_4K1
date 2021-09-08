import copy
import time
from graph_tool.all import *
from Gestion_Fichiers import *
from Special_graphs import *
from Graph_dans_fichier_v4 import *


def attacheavecnombrefixedarretes_plus(g,m,L_special):
    """Fonction qui test si on peut rajoute un sommet v de degre m à g. 
    Si le graphe resultant n'appartiens pas à Free(C4;O4)
    ou qu'il est déjà l'un des connus
    ou que v a un jumeau ou qu'il est complet à g
    retourner TRUE sinon FALSE"""
    if m==0:
        G=copy.copy(g)
        v=G.add_vertex()
        if FreeC4O4(G,v)==True :
            if aunjumeau(G,v)==False:
                not_special=True
                for G_special in L_special :
                    if graph_tool.topology.isomorphism(G,G_special)==True:
                        not_special=False
                        break
                if not_special==True :
                   return False
            """ ici"""
        return True 

    else:
        #Obtention du nombre de sommets de g
        n=0
        for v in g.vertices(): 
            n=n+1
        #Pour tout les ensembles ordonés de taille m d'entier de 1 à taille(n) P
        LP=sousens_nonord_k(m,n)
        for P in LP :
            G=copy.copy(g)
            v=G.add_vertex()
            #Rajouter toutes les aretes entre v et les sommets de g correspondants aux indices de P
            for i in P :
                w = G.vertex(i-1)
                G.add_edge(v,w)
            #Tester g 
            if FreeC4O4(G,v)==True :
                if aunjumeau(G,v)==False:
                    not_univ=False
                    for w in g.vertices():
                        if not(w in v.out_neighbors()):
                            #dé que c possible on s'arrete
                            not_univ=True
                            break
                    if not_univ==True:
                        not_special=True
                        for G_special in L_special :
                            if graph_tool.topology.isomorphism(G,G_special)==True:
                                not_special=False
                                break
                        if not_special==True :
                            return False
        return True
    

def Est_Blowup_etendu(g,L_special):
    """Test si pour tout graph contenant g  comme sous graph est un blowup ou un qu'on connais deja"""
    #Reccupere le nombre de sommets
    n=0
    for v in g.vertices():
        n=n+1
    #Test pour toutes les combinaison de 1 à n 
    k=0
    G=copy.copy(g)
    while k<n+1:
        if attacheavecnombrefixedarretes_plus(G,k,L_special)== False:
            return False
        k=k+1
    return True 



def Blow_up_etendu(n):
    """Reccupere les graphes par les fichiers, les tests et élimines au fur et à mesure."""
    if Verifie_est_dans_sommaire(n):
            L=Lecture_Graph(n)
    else :
            L=ToutLesGraphsAnSommets_Par_Fichier(n)
    LFinale=[]
    print("J'ai", len(L), "graphes à tester")
    L_special=Lecture_Graph_speciale(n+1)
    print("Je cheche si il y a des graphes speciaux")
    if Verifie_est_dans_sommaire_speciale(n+1)==True :
        print("Il y en a ",len(L_special))
        while len(L)!=0:
            if len(L)%100==0:
                print("Il reste", len(L),"graphes à tester")
            Gquontest=copy.copy(L[0])
            if Est_Blowup_etendu(L[0],L_special):
                LFinale.append(Gquontest)
            del L[0]
    else : 
        print("Il n'y en a pas !")
        while len(L)!=0:
            if len(L)%100==0:
                print("Il reste", len(L),"graphes à tester")
            Gquontest=copy.copy(L[0])
            if Ets_Blowup(L[0]):
                LFinale.append(Gquontest)
            del L[0]

        
    return LFinale
    


def main():
    n=int(input("Entrez le nombre de sommets : "))
    Supertest=input("Voulez-vous generer les graphs (G) ou les traiters (T) :")
    if Supertest == "G":
        L=ToutLesGraphsAnSommets_Par_Fichier(n)
        print("il y a ", len(L)," graphes à ",n, "sommets. ")
        Jevoislesgraphs=input("Voulez-vous juste les generer (G) ou aussi les affichers (A) :")
        if Jevoislesgraphs=="A" : 
            for i in range(len(L)) :
                print(L[i])
                for e in L[i].edges():
                    print(e)

    else :
        Juste_blow_up=input("Voulez-vous juste verrfier qu'ils donnent un blow-up (B) ou aussi si il donne un graphe identifé (I)")
        if Juste_blow_up == "B" : 
            L=Est_Blow_up_sans_generer(n)
        else :
            L=Blow_up_etendu(n)
        #Affichage
        if len(L)==0:
            print("Pas de blowup")
        else:
            m=len(L)
            for i in range(len(L)):
                print(L[i])
                for e in L[i].edges():
                    print(e)
        #Je range dans le fichier
        Nouveau_fichier_special(n)
        for i in range(len(L)):
            Graph_dans_bon_fichier_Special(L[i],n)

                    
            
    print("Pour rappel : ", n," il y a ",m," blowup ")




    
if __name__=="__main__":
    main()
    #cProfile.run('main()')
    #G=graphe_icosaedre_moins_1()
    #print(G)
    #for e in G.edges():
     #   print(e)
