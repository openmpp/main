### Première grossesse  

Comme nous l'avons décrit plus haut, la première grossesse est modélisée au moyen d'un risque à un âge de référence et des risques relatifs dépendant de la situation d'union et de la durée. Le tableau 1 qui suit donne les estimations des paramètres pour la Bulgarie et la Russie avant et après la transition politique et économique. 

**Tableau 1 : Risques d'une première grossesse** 

|table>
        | Bulgarie | Russie 
- | - | -
15 à 17,5 | 0,2869 | 0,2120 
17,5 à 20 | 0,7591 | 0,7606 
20 à 22,5 | 0,8458 | 0,8295 
22,5 à 25 | 0,8167 | 0,6505 
25 à 27,5 | 0,6727 | 0,5423 
27,5 à 30 | 0,5105 | 0,5787 
30 à 32,5 | 0,4882 | 0,4884 
32,5 à 35 | 0,2562 | 0,3237 
35 à 37,5 | 0,2597 | 0,3089 
37,5 à 40 | 0,1542 | 0,0909 
|<table

|table>
                              | Avant la transition | de 1989 | Dix ans après | la transition : 1999+
                              | Bulgarie    | Russie | Bulgarie           | Russie 
- | - | -
Pas dans une union                  | 0,0648 | 0,0893 | 0,0316 | 0,0664 
Trois premières années de la première union  | 1,0000 | 1,0000 | 0,4890 | 0,5067 
Première union après trois ans | 0,2523 | 0,2767 | 0,2652 | 0,2746 
Deuxième union                  | 0,8048 | 0,5250 | 0,2285 | 0,2698 
|<table
 
Les données du tableau 1 sont interprétées de la façon suivante dans le modèle. Aussi longtemps qu'une femme n'a pas formé une union, nous devons multiplier son risque de base variant en fonction de l'âge d'une première grossesse par le risque relatif de « ne pas être dans une union ». 
Par exemple, nous pouvons calculer le risque de grossesse d'une femme célibataire de 20 ans de la cohorte bulgare avant la transition comme étant `0,8458*0,0648 = 0,05481`. À ce taux de `λ = 0,05481` : 

- le temps d'attente moyen prévu de l'événement de grossesse est `1/λ = 1/0,05481 = 18,25` ans; 
- la probabilité qu'une femme ne devienne pas enceinte dans les 2,5 ans qui suivent (étant donné qu'elle demeure célibataire) est `exp(-λt) = exp(-0,05481*2,5) = 87,2 %`. 

Donc, à son 20e anniversaire, nous pouvons tirer une durée aléatoire jusqu'à la première grossesse sous forme d'un nombre aléatoire suivant une loi uniforme (un nombre qui peut prendre n'importe quelle valeur entre 0 et 1 avec la même probabilité) en utilisant la formule : 

`Durée aléatoire = -ln(RandomUniform) / λ`. 

Comme nous l'avons calculé plus haut, dans 87,2 % des cas, aucune conception n'aura lieu au cours des 2,5 années qui suivent. Par conséquent, si nous tirons un nombre aléatoire suivant une loi uniforme plus petit que 0,872, le temps d'attente correspondant sera plus long que 2,5 ans, puisque `-ln(RandomUniform) / λ = -ln(0,872)/0,05481 = 2,5` ans. Un tirage aléatoire supérieur à 0,872 résultera en un temps d'attente plus faible que 2,5 ans — dans cette situation, si la femme n'entre pas dans une union avant l'événement de grossesse, la grossesse a lieu dans notre simulation. 

Afin de poursuivre cet exemple, supposons que le premier événement qui se produit dans notre simulation est la formation d'une union à l'âge de 20,5 ans. Nous devons maintenant mettre à jour le risque de grossesse. Tandis que le risque de base reste le même pour les deux années qui suivent (c.-à-d. 0,8458), le risque relatif est maintenant égal à 1,0000 (conformément à la catégorie de référence du tableau 1) parce que la femme se trouve dans les trois premières années d'une union. Le nouveau taux de risque de grossesse (applicable aux deux années suivantes, jusqu'à l'âge de 22,5 ans) est maintenant considérablement plus élevé, soit `0,8458 * 1,0000 = 0,8458`. Le temps d'attente moyen à ce taux n'est donc que de `1/0,8458 = 1,18` an et, pour tout nombre aléatoire supérieur à `exp(-0,8458*2)=0,1842`, le temps d'attente simulé sera inférieur à deux ans. Autrement dit, 81,6 % (`1 - 0,1842`) de femmes connaîtront une première grossesse dans les deux premières années d'une première union ou d'un premier partenariat qui a débuté à l'âge de 20,5 ans. 


[[Accueil](#Home)] [[<< Précédent](#002-General-description)] [[Suivant >>](#004-First-union-formation)]