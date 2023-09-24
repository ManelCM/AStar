//Manel Carrillo Maíllo 1633426
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define R 6371
#define CONST_RADIANTS 0.01745329252
#define MAXARST 13
#define PI 3.1415926536

//----------ESTRUCTURES---------------

//ESTRUCRURA D'ELEMENT CUA
typedef struct element { 
  unsigned index; //estructura element cua té un index, posició.
  struct element *seguent;//senyala al seu seguent (implementació de cues)
} ElementCua;

//ESTRUCTURA D'UNA CUA
typedef struct {
  ElementCua *inicial, *final;//estructura cua indiquem quin és el primer i ultim element en aquesta
} Cua;

//ESTRUCTURA (INFORMACIÓ) D'UNA ARESTA O CARRER
typedef struct {
  char carrer[12];
  unsigned numnode; //índex en llista[i] d'un node connectat per aresta
  double llargada; //indica la llargada de l'aresta, (llargada de node a node o del carrer en aquest cas)
} infoaresta;

//ESTRUCTURA D'UN NODE
typedef struct {
  long long int id;//Identificador 
  double latitud, longitud; //Posició del node
  int narst; //Nombre d'arestes que arriben al node
  infoaresta arestes[MAXARST]; //llista d'arestes cridant a l'estructura anterior
  double cost;//cost que serveix per al càlcul de l'heuristica
  double dist;
  int dintre_cua;// indica si el node ha estat a la cua (podria ser un bool {0,1} o {True,False})
  unsigned anterior; //Senyala al seu node anterior 
} node;

//-------------DECLARACIÓ DE FUNCIONS------------------------
void desencua(Cua *);

int add_with_priority(unsigned,Cua *,node[]);
void requeue_with_priority(unsigned,Cua *,node[]);

unsigned buscapunt(long long int,node[],unsigned);
double distancia(node,node);
void mostracami(node[],unsigned,unsigned,unsigned);

//---------MAIN--------------------------------------------

int main(int argc, char *argv[]) {
  //argc com a minim val 1, de manera que si només introduim un o més de dos hem de saltar error
   if (argc!=3){
     printf("Error a la introducció de nodes entrats.");
     exit(23);
   }
  int ll, n_nodes = 0;
  long long int origen, desti; //variables per guardar els arguments entrats en l'execució
  origen = atol(argv[1]);//guardem el node origen
  desti = atol(argv[2]);//guardem el node de destí
  //-----------------LECTURA DEL FITXER NODE------(PRACTICA 9)------

  FILE *dades; 
  node *llista_punts;
  dades = fopen("Nodes.csv", "r");//obrim el fitxer de nodes i el guarem a dades
  if (dades == NULL) {
    printf("\nNo s'ha accedit al fitxer de dades\n");//si dades està buit, no hem entrat
    return 1;
  }
  while ((ll = fgetc(dades)) != EOF) {
    if (ll == '\n') {
      n_nodes++;//contem els nodes que hi ha al fitxer
    }
  }
  rewind(dades); //tornem al principi del fitxer

  if ((llista_punts = (node *)malloc(n_nodes * sizeof(node))) == NULL) { //reservem memoria per als nodes
    printf("\nNo es possible assignar la memoria necessaria...\n\n");
    return -1;
  }

  for (int i = 0; i < n_nodes; i++) {//guardem la informació de cada node
   if(fscanf(dades, "%lld;", &llista_punts[i].id)); 
   if(fscanf(dades, "%lf;", &llista_punts[i].latitud)); 
   if(fscanf(dades, "%lf", &llista_punts[i].longitud)); //tercer valor->longitud del node
   llista_punts[i].dist = 0;//inicialitzar la informació de cada node
   llista_punts[i].dintre_cua = 0;
   llista_punts[i].narst = 0;
   llista_punts[i].cost = UINT_MAX; 
  }    
  fclose(dades);

  //-----------------------LECTURA DEL FITXER CARRERS (PRACTICA 9)---------------------
  int n_carrers = 0;
  FILE *dades_carrers;
  dades_carrers = fopen("Carrers.csv", "r");
  if (dades_carrers == NULL) {
    printf("\nNo s'ha accedit al fitxer de dades\n");//comprobem si hem entrat al fitxer de carrers
    return -1;
  }
  while ((ll = fgetc(dades_carrers)) != EOF) {
    if (ll == '\n') {
      n_carrers++;//contem el nombre de carrers que hi ha al fitxer
    }
  }
  rewind(dades_carrers);//tornem a l'inici de carrers
//inicialització de les variables dels carrers
  char carrer_id[12];
  unsigned k_ant, k_act;
  long long int node_id;
  long long int ant;

  for (unsigned i = 0; i < n_carrers; i++) {
   if(fscanf(dades_carrers, "id=%[0-9];", carrer_id));
   ant = 0;

    while ((ll = fgetc(dades_carrers)) != '\n') {
      fseek(dades_carrers, -1, SEEK_CUR); // tirem una posició enrere
      if(fscanf(dades_carrers, "%lld;", &node_id));
      if (ant != 0) {
        k_ant = buscapunt(ant, llista_punts, n_nodes);
        k_act = buscapunt(node_id, llista_punts, n_nodes);
        llista_punts[k_ant].arestes[llista_punts[k_ant].narst].numnode = k_act;
        llista_punts[k_act].arestes[llista_punts[k_act].narst].numnode = k_ant;
        strcpy(llista_punts[k_ant].arestes[llista_punts[k_ant].narst].carrer,
               carrer_id);
        strcpy(llista_punts[k_act].arestes[llista_punts[k_act].narst].carrer,
               carrer_id);
        llista_punts[k_act].narst++;
        llista_punts[k_ant].narst++;
        ant = node_id;
        } 
      else {
        ant = node_id;
        k_act = buscapunt(node_id, llista_punts, n_nodes);
        llista_punts[k_act].narst++;
      }
    }
  }
  fclose(dades_carrers);

  //-------------------A* IMPLEMENTATION-----------------------

  // assignem els dos punts, el punt d'inici i el punt del final del nostre.
  // recurregut
  unsigned punt_sortida = buscapunt(origen, llista_punts, n_nodes);
  unsigned punt_arribada = buscapunt(desti, llista_punts, n_nodes);
  
  if (punt_sortida==punt_arribada){//comprobar si són els mateixos
    printf("Error: No hi ha camí a fer, el node de sortida i el node d'arribada són els mateixos"); 
    exit(22);
  }
  
  Cua Cua = {NULL, NULL};//creem cua buida
  //abans estava amb encua però no cal ja que només serviria pel primer node per tant aprofito aquesta funció.
  // Creem un element de cua el qual guardarà el primer valor de la cua.
  llista_punts[punt_sortida].cost = 0; //abans era UNIT MAX.
  llista_punts[punt_sortida].anterior = 0; //no té cap pare.
  
    // Encuem la primera posició
    //abans estava amb encua però no cal ja que només serviria pel primer node per tant aprofito aquesta funció.
   add_with_priority(punt_sortida,&Cua,llista_punts);
    while (Cua.inicial !=NULL) { // mirar si la cua és buida.
      unsigned index = (Cua.inicial)->index;//guardem index del node que mirem
      desencua(&Cua);
      if (index == punt_arribada) {  // si els index són iguals, hem arribat al destí.
        mostracami(llista_punts,n_nodes,punt_sortida,punt_arribada );//mostrem el camí fet.
        break;
    }
    
    //els adjunts es miren llista_punts[llista_punts[index].arestes[i].numnode)
      for (unsigned int i = 0; i < llista_punts[index].narst; i++) {//creem bucle per agafar el cami amb la distancia minima
        double dist_nodes=distancia(llista_punts[llista_punts[index].arestes[i].numnode],llista_punts[index]);//distancia entre nodes
        double heuristica= distancia(llista_punts[llista_punts[index].arestes[i].numnode],llista_punts[punt_arribada]);//distancia del node al node final.
        double distancia_total= llista_punts[index].dist +heuristica+dist_nodes;//suma de les distancies
      
        if (distancia_total < llista_punts[llista_punts[index].arestes[i].numnode].cost){//comparem totes les distancies dels adjunts amb el seu cost
          llista_punts[llista_punts[index].arestes[i].numnode].cost=distancia_total;//actualitzem el cost del node
          llista_punts[llista_punts[index].arestes[i].numnode].dist = llista_punts[index].dist +dist_nodes;//actualitzem distància del node
          llista_punts[llista_punts[index].arestes[i].numnode].anterior = index;

        
        if (llista_punts[llista_punts[index].arestes[i].numnode].dintre_cua==0) { // si el node adjunt no esta a la cua l'afegim amb prioritat
                add_with_priority(llista_punts[index].arestes[i].numnode,&Cua,llista_punts);
        }

        else{requeue_with_priority(llista_punts[index].arestes[i].numnode, &Cua,llista_punts);}//si ja estava a la cua fem un reencua amb prioritat
        }
      }
    }
  
   if (Cua.inicial == NULL) {//si recorrem tot i no trobem el fi, no hi ha camí a seguir
      printf("Error: No hi ha cap camí que connecti els nodes.\n");
      exit(69);
  }
  return 0;
}
  //----------------FUNCIONS-----------------------
  unsigned buscapunt(long long int ident, node l[], unsigned nnodes) {
    for (int k = 0; k < nnodes; k++) {
      if (l[k].id == ident) {
        return k;
      }
    }
    printf("ID no trobada, node inventat \n");
    exit(89);
  }

  double distancia(node origen,
                   node desti) { // la calculem en km. Si volem en m canviar R
    double x1, y1, z1, x2, y2, z2, modul;
    x1 = R * cos(origen.longitud * CONST_RADIANTS) *
         cos(origen.latitud * CONST_RADIANTS);
    y1 = R * sin(origen.longitud * CONST_RADIANTS) *
         cos(origen.latitud * CONST_RADIANTS);
    z1 = R * sin(origen.latitud * CONST_RADIANTS);

    x2 = R * cos(desti.longitud * CONST_RADIANTS) *
         cos(desti.latitud * CONST_RADIANTS);
    y2 = R * sin(desti.longitud * CONST_RADIANTS) *
         cos(desti.latitud * CONST_RADIANTS);
    z2 = R * sin(desti.latitud * CONST_RADIANTS);

    modul = sqrt(pow(fabs(x1 - x2), 2) + pow(fabs(y1 - y2), 2) +
                 pow(fabs(z1 - z2), 2));
    return modul;
  }

void desencua(Cua *Pendents) {
  ElementCua *node_inicial = Pendents->inicial;
  Pendents->inicial = Pendents->inicial->seguent;
  free(node_inicial);
}

void requeue_with_priority(unsigned v, Cua *Pq, node llista[]) {
  register ElementCua *prepv;

  if (((*Pq).inicial)->index == v) {
    return;
  }

  for (prepv = (*Pq).inicial; prepv->seguent->index != v; prepv = prepv->seguent)
    ;
  ElementCua *pv = prepv->seguent;
  prepv->seguent = pv->seguent;
  free(pv);
  add_with_priority(v, Pq, llista);
}

int add_with_priority(unsigned v, Cua *Pq, node llista_punts[]) {
  register ElementCua *q;
  ElementCua *aux = (ElementCua *)malloc(sizeof(ElementCua));
  if (aux == NULL) {
    return 0;
  
  }
  aux->index = v;
  double costv = llista_punts[v].cost;

  llista_punts[v].dintre_cua = 1;
  if ((*Pq).inicial == NULL || !(costv > llista_punts[((*Pq).inicial)->index].cost)) {
    aux->seguent = (*Pq).inicial;
    (*Pq).inicial = aux;

    return 1;
  }
  for (q = (*Pq).inicial; q->seguent && llista_punts[q->seguent->index].cost < costv;
       q = q->seguent);
  aux->seguent = q->seguent;
  q->seguent = aux;
  return 1;
}

//funció mostracami, mateixa que la de gerres
void mostracami(node llista_nodes[], unsigned n_nodes ,unsigned punt_inci, unsigned punt_final) {
  node  *llista_aux;
  llista_aux=llista_nodes;
  
  int i = 0;
  int node_recorregut=punt_final; //el guardem a una variable per poder anar recorrent i guardar els nodes anteriors del node final


  //Fem un while contar quants nodes hi ha fins al node final, recorrent els nodes des del final fins arribar a 
  while (node_recorregut != punt_inci) { 
    node_recorregut=llista_aux[node_recorregut].anterior; 
    i++; //sumem les i per poder crear un vector que carregarem amb i+1 nodes
  }
  node *vector[i+1]; 
  llista_aux=llista_nodes;
  i=0,
  node_recorregut=punt_final;//tornem a posar el punt final al node que hem creat per recorrer els nodes anteriors
  
  //carreguem un vector mentre el nod recorregu
  while (punt_inci!= node_recorregut) {
    vector[i] = &llista_aux[node_recorregut]; 
    node_recorregut = llista_aux[node_recorregut].anterior; 
    i++;
  }
  vector[i] = &llista_aux[node_recorregut];
  
printf("# La distancia de %lld a %lld es de %lf metres.\n",llista_nodes[punt_inci].id,llista_nodes[punt_final].id,vector[0]->dist*1000);
  printf("# Cami Optim:\n");
  for (int l = i; l>=0; l--){ printf("Id=%lld | %lf | %lf | Dist=%lf\n",
    vector[l]->id,vector[l]->latitud, vector[l]->longitud, vector[l]->dist*1000);} 
} 