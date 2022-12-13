# Optimisations

Nous avons choisi de nous concentrer sur de l'optimisation CPU avecOpen MP. Les simulations sont réalisées plusieurs fois pour éviter toutes valeurs potentiellement faussées.

Les optimisations sont réalisées principalement avec l'aide du profiler `Intel Advisor`

Nous les lançons avec 6 threads.

---
## Simulation_0 : 
Version originale sans Open MP (séquentiel)

## Simulation_1 :
- Ajout de tasks Open MP dans 
```
Organism::look_for_new_promoters_starting_between
```

- Ajout d'une parallélisation/réduction de `geometric_area` dans 
```
ExpManager::ExpManager
```

## Simulation_2 :
Nous avons observé que les performances étaient moins bonnes qu'en séquentiel, nous avons supposé que cette baisse était due aux tasks créées qui étaient trop nombreuses par rapport à la taille des opérations ce qui augmentait le temps. Nous avons donc garder l'implémentation de la `simulation_1` sans les tasks.

## Simulation_3 :
- Vectorisation des boucles dans 
```
Dna::promoter_at
Dna::terminator_at
```
avec 
```
#pragma omp simd reduction(+:dist_lead)
```
Le vecteur `seq_` a été reconstruit de manière à ne le parcourir que dans un seul sens sans revenir à l'index 0 une fois arrivé au dernier index: `seq_ext`. Cette modification permet de pouvoir utiliser des `pragma simd` plus des `reduction` lors des opérations effectuées.

Ces modifications ont légèrement améliorés les performances sur les premières générations.

## Simulation_4 :

