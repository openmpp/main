### Description générale 

Le modèle RiskPaths étant conçu pour étudier l'absence d'enfants, le principal événement est la première grossesse (qui est toujours supposé aboutir à une naissance). La grossesse peut survenir n'importe quand après le 15e anniversaire, le risque évoluant en fonction de l'âge ainsi que de la situation d'union. Les modèles statistiques sous-jacents sont des régressions à risque constant par morceaux. En ce qui concerne la fécondité, cela implique l'hypothèse d'un risque de grossesse constant pour un groupe d'âge (p. ex. 15 à 17,5 ans) et une situation d'union (p. ex. célibataire sans aucune union antérieure) donnés. 

Pour les unions, nous distinguons quatre niveaux d'état possibles : 

- célibataire; 
- les trois premières années d'une première union; 
- les années suivantes d'une première union; 
- toutes les années d'une deuxième union. 

(Après la dissolution d'une deuxième union, il est supposé que les femmes restent célibataires.) Par conséquent, nous modélisons cinq événements d'union distincts : 

- formation de la première union; 
- dissolution de la première union; 
- formation de la deuxième union; 
- dissolution de la deuxième union; 
- le changement de phase d'union qui survient après trois ans dans la première union. 

Le dernier événement (changement de phase de l'union) est un événement horloge, qui diffère des autres événements en ce sens que le moment où il survient n'est pas stochastique mais prédéfini. (Un autre événement horloge dans le modèle est le changement de l'indice d'âge, tous les 2,5 ans.) Outre les unions et la fécondité, nous modélisons la mortalité — une femme peut mourir à n'importe quel point dans le temps. Nous arrêtons la simulation de la grossesse et des événements d'union quand une femme est décédée ou qu'elle est enceinte (car nous cherchons seulement à étudier l'absence d'enfants) ou à son 40e anniversaire, parce que les premières grossesses après cet âge sont très rares en Russie et en Bulgarie et ne sont donc pas prises en compte dans le présent modèle). 

À 15 ans, une femme devient exposée à la fois au risque de grossesse et de formation d'une union. Il s'agit de risques concurrents. Nous tirons des durées aléatoires jusqu'à la première grossesse et jusqu'à la formation de la première union. À ce stade il existe deux autres événements concurrents — la mortalité et le changement de groupe d'âge. (Comme nous supposons que les risques de grossesse et de formation d'une union évoluent avec l'âge, les risques qui sous-tendent les durées aléatoires ne s'appliquent qu'à une période donnée — 2,5 ans dans notre modèle — et doivent être recalculés à chaque point dans le temps.) 

Autrement dit, le 15e anniversaire sera suivi par l'un des quatre événements possibles suivants : 

- la femme décède; 
- elle devient enceinte; 
- elle entre dans une union; 
- elle entre dans un nouveau groupe d'âge à 17,5 ans, parce qu'aucun des trois événements qui précèdent n'a eu lieu avant qu'elle n'atteigne 17,5 ans. 

Le décès ou la grossesse mettent fin à la simulation. Un indice de changement d'âge requiert que les temps d'attente pour les événements concurrents de formation d'une union et de grossesse soient mis à jour. L'événement de formation d'une union modifie le risque d'une première grossesse (le rendant beaucoup plus élevé) et change l'ensemble de risques concurrents. La femme n'est désormais plus exposée au risque de formation d'une première union, mais devient exposée au risque de dissolution de l'union. 


[[Accueil](#Home)] [[<< Précédent](#001-Introduction)] [[Suivant >>](#003-First-pregnancy)]