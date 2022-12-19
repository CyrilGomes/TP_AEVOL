# Introduction

Ce document a pour objectif de récapituler les diiférentes modifications effectuées sur les simulations en ajoutant une petite conclusion.

Nous avons choisi de nous concentrer sur de l'optimisation CPU avecOpen MP. Les simulations sont réalisées plusieurs fois pour éviter toutes valeurs potentiellement faussées.

Les optimisations sont réalisées principalement avec l'aide du profiler `Intel Advisor`

Nous les lançons avec 6 threads compilés en mode `Release`.

# Détails des optimisations

## Simulation_0 :

Version originale sans Open MP (séquentiel)

## Simulation_1 :

- Ajout d'une parallélisation/réduction de `geometric_area` dans les 2 constructeurs :
```
ExpManager::ExpManager(...)
```
avec 
```
#pragma omp parallel for reduction(+:geometric_area)
#pragma omp parallel for
```

Cette réduction nous est apparue rapidement, on a reconnu un calcul d'intégrale qui est facilement parallélisable parce qu'on peut calculer chaque intervalle séparémment. La réduction permet également d'accélérer le calcul en réduisant le nombre d'addition. On a utilisé le procédé vu en TP pour le calcul de pi.

## Simulation_2 :

- Vectorisation des boucles dans 
```
Dna::promoter_at(...)
Dna::terminator_at(...)
```
avec 
```
#pragma omp simd reduction(+:dist_lead)
```
Le vecteur `seq_` a été reconstruit de manière à ne le parcourir que dans un seul sens sans revenir à l'index 0 une fois arrivé au dernier index. Ce nouveau vecteur est : `seq_ext`. Cette modification permet de pouvoir utiliser des `pragma simd` plus des `reduction` lors des opérations effectuées.

Ces modifications ont légèrement améliorés les performances sur les dernières générations.

## Simulation_3 :

- Ajout d'une parallélisation et d'un pragma `simd` dans la fonction 
```
Organism::translate_protein()
```
avec 
```
#pragma omp simd
```

Cette vectorisation nous a été suggérée par `Intel Advisor`.
Cette modification n'a pas eu énormément d'impact sur les performances.

## Simulation_4 :

- Cette simulation a été lancée avec une modification de la fonction :
```
Organism::compute_fitness(...)
```
avec 
```
#pragma omp simd reduction(+:metaerror)
```

La réduction sur la variable `metaerror` ainsi qu'un pragma `simd` pour les opérations sur le tableau nous a permis d'avoir une bonne amélioration sur les performances pour l'ensemble des générations. La vectorisation nous a encore une fois été suggérée par `Intel Advisor`.

## Simulation_5 :

- Ajout de tasks Open MP dans 
```
Organism::look_for_new_promoters_starting_between(...)
```
sur les opérations 
```
#pragma omp parallel {
    #pragma omp single {
        #pragma omp task {
            look_for_new_promoters_starting_after(pos_1);
        }
        #pragma omp task {
            look_for_new_promoters_starting_before(pos_2);
        }
    }
}
```
Ces opérations sont indépendantes et peuvent donc être exécutées en parallèle : les `tasks` d'Open MP nous semblait très bien répondre à cette problématique.
Néanmoins, nous avons observé que les performances étaient largement moins bonnes que sur l'ensemble des simulations précédentes. Nous avons supposé que cette baisse était due aux `tasks` créées qui étaient trop nombreuses par rapport à la taille des opérations, ce qui augmentait le temps. Nous avons donc garder l'implémentation de la `simulation_4`, sans les tasks.


## Simulation_6 :

- Cette simulation est simplement la même que la `simulation_4` après avoir retiré les `tasks`. elle nous a permis de confirmer que cet ajout était bien la cause des pertes de performances. Cette simulation permet aussi de mettre en évidence que malgré le fait de lancer plusieurs fois de suite le programme, nous pouvons quand même avoir des différences de performances, pour une même configuration.


## Simulation_7 :

- Ajout de parallélisation sur les différentes boucles de la fonction 
```
ExpManager::run_a_step()
```
avec 
```
#pragma omp parallel for
```

Ces optimisations ont considérablement amélioré les performances du programme en divisant plus de 2 fois le temps d'éxécution. Ce gros gain de performances a pu être possible car toutes les boucles vont de 0 à `nb_indivs_` qui est égal par défaut à : 
```
nb_indivs_= grid_height * grid_width
```
Ce grand nombre d'opérations indépendantes effectué séquentiellement a pu être largement optimisé avec la parallélisation.


## Simulation_8 :
- Une autre boucle parcourant `nb_indiv_` se trouve dans 
```
ExpManager::run_evolution(...)
```
a été parallélisée avec 
```
#pragma omp parallel for
```

Cette parallélisation permet d'améliorer le temps d'exécution du programme.
