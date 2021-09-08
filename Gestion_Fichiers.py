import copy
import time
from graph_tool.all import *


#Pour copier depuis une liste : Graph_dans_bon_fichier(g,n)
# et Graph_dans_bon_fichier_Special(g,n)

#Pour reccup la liste : def Lecture_Graph(n) def
#Lecture_Graph_speciale(n):


#################################
##### Fonctions de lecture ######
#################################


def Arrete(uv):
    """uv est une chaine de charactère "uv".
    La scinder et renvoyer les deux sommets adjacents"""
    arrete=list(uv)
    u=""
    v=""
    i=0
    while arrete[i]!=";":
        u=u+arrete[i]
        i=i+1
    i=i+1
    while i<len(arrete):
        v=v+arrete[i]
        i=i+1

    return int(u), int(v)

def reccup_graph(ligne_tableau):
    """ La premiere case de la ligne prend le nombre de sommets du graphs et les suivantes toutes les arrettes
    Attention, tout les tableau n'ont pas forcement le même nombre d'arrettes"""

    #Je crée le graphe avec le bon nombre de sommets 
    n=int(ligne_tableau[0])
    g=Graph(directed=False)
    g.add_vertex(n)
    #Je rajoute le bon nombre d'arretes 
    for i in range(1,len(ligne_tableau)):
        #Je vérifie quand même que l'arrete existe sinon je renvois NONE 
        u,v=Arrete(ligne_tableau[i])
        if u>=n or v>= n :
            print("Cette arrete ne peut pas exister")
            return None 
        g.add_edge(u,v)

    return g




def reccup_liste_graph(nom_fichier):
    """Renvois la liste des graphes lue dans le fichier"""
    #Je ne revois pas la taille des graphs
    tableau=[]
    #Ovrir le fichier 
    f=open(nom_fichier,"r")
    #lire toutes les lignes
    i=1
    for ligne in f:
        print("ligne", i)
        #pour chaques lignes extraire le graph et le mettre dans le tableau 
        lligne_en_liste=ligne.split()
        g=reccup_graph(lligne_en_liste)
        tableau.append(g)
        i=i+1
    f.close()
    #retourner le tableu
    return tableau 


def Lecture_Graph(n):
    """ n est les nombre de sommets du graph"""
    #Je dois vérifier si on a déjà le fichier 
    if Verifie_est_dans_sommaire(n):
        #Générer le nom du fichier 
        nombre=str(n)
        nom="graphedelataille"
        nom=nom+nombre+".txt"
        tableau=reccup_liste_graph(nom)
        return tableau 
    else :
        return None

def Lecture_Graph_speciale(n):
    """ n est les nombre de sommets du graph"""
    #Je dois vérifier si on a déjà le fichier 
    if Verifie_est_dans_sommaire_speciale(n):
        #Générer le nom du fichier 
        nombre=str(n)
        nom="graphe_special"
        nom=nom+nombre+".txt"
        tableau=reccup_liste_graph(nom)
        return tableau 
    else :
        return None 


def Verifie_est_dans_sommaire(n):
    """Vérifie si on a déjà crée le fichier"""
    f=open("sommaire_graphs.txt")
    text=f.readlines()
    f.close()
    if str(n)+"\n" in text:
        return True 
    else: 
        return False


def Verifie_est_dans_sommaire_speciale(n):
    """Vérifie si on a déjà crée le fichier"""
    f=open("sommaire_graphs_special.txt")
    text=f.readlines()
    f.close()
    if str(n)+"\n" in text:
        return True 
    else: 
        return False 

################################
#### FONCTIONS DE D'ECRITURE ###

def Nouveau_fichier(n):
    #Générer le nom du fichier
    nombre=str(n) 
    nom="graphedelataille"+nombre+".txt"
    f=open(nom,'w')
    f.close()
    Rajoute_aux_Sommaire(n)

def Nouveau_fichier_special(n):
    #Générer le nom du fichier
    nombre=str(n) 
    nom="graphe_special"+nombre+".txt"
    f=open(nom,'w')
    f.close()
    Rajoute_aux_Sommaire_Special(n)

def Graph_to_liste(g,n):
    """Cole un graph dans une liste dans le bon format pour l'écriture
    n est le nombre de sommets de g """
    t=[]
    #Reccuperation nombre de sommets de g 
    t.append(n)
    for v in g.vertices():
        for w in v.out_neighbors():
            if w>v:
                arrete=str(w)+";"+str(v)
                t.append(arrete)
    return t 

def Liste_to_Fichier(l,nom_fichier):
    #Ouvrir le fichier en lecture 
    f=open(nom_fichier,"r")
    #Copier tout le texte 
    texte=f.read()
    #Fermer le fichier 
    f.close()
    #Ouvrir le fichier en ecriture 
    f=open(nom_fichier,"w")
    #Reccopier tout le texte 
    f.write(texte) 
    #Transforme la liste en str 
    ligne_du_graph=""
    for i in range (len(l)):
        ligne_du_graph=ligne_du_graph+str(l[i])+" "
    ligne_du_graph=ligne_du_graph+"\n"
    #Rajouter le graph
    f.write(ligne_du_graph)
    #Fermer le fichier 
    f.close()

def Graph_dans_bon_fichier_Special(g,n):
    """Il faut faire attention si le fichier existe déjà ou non"""
    if Verifie_est_dans_sommaire_speciale(n)==False:
        Nouveau_fichier_special(n)
    t=Graph_to_liste(g,n)
    nombre=str(n)
    nom="graphe_special"+nombre+".txt"
    Liste_to_Fichier(t,nom)

def Graph_dans_bon_fichier(g,n):
    """Il faut faire attention si le fichier existe déjà ou non"""
    if Verifie_est_dans_sommaire(n)==False:
        Nouveau_fichier(n)
    t=Graph_to_liste(g,n)
    nombre=str(n)
    nom="graphedelataille"+nombre+".txt"
    Liste_to_Fichier(t,nom)


def Rajoute_aux_Sommaire(n):
    """rajoute au fichier du sommaire qu'on a un nouveau fichier avec tout les graphs de taille n"""
    #On vérrifie qu il n y est pas deja
    if Verifie_est_dans_sommaire(n)==False:
        #Ouvrir sommaire en lecture et copier tout son contenue et le fermer 
        f=open("sommaire_graphs.txt")
        texte=f.read()
        f.close()
        #Ouvrir le sommmaire en écriture 
        w=open("sommaire_graphs.txt","w")
        #Rajouter toute la copie 
        w.write(texte) 
        #Rajouter le dernier n 
        w.write(str(n)+"\n")
        #fermeer le fichier 
        w.close()



def Rajoute_aux_Sommaire_Special(n):
    """rajoute au fichier du sommaire qu'on a un nouveau fichier avec tout les graphs de taille n"""
    #On vérrifie qu il n y est pas deja
    if Verifie_est_dans_sommaire_speciale(n)==False:
        #Ouvrir sommaire en lecture et copier tout son contenue et le fermer 
        f=open("sommaire_graphs_special.txt")
        texte=f.read()
        f.close()
        #Ouvrir le sommmaire en écriture 
        w=open("sommaire_graphs_special.txt","w")
        #Rajouter toute la copie 
        w.write(texte) 
        #Rajouter le dernier n 
        w.write(str(n)+"\n")
        #fermeer le fichier 
        w.close()


