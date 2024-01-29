* Les valeurs dans `ProbMort` n'ont aucun effet si [`CanDie`](#CanDie) est `false`.
* La fonction « [GetLabel(Person::DeathEvent)](#DeathEvent) » dans `RiskPaths`
 traite les valeurs dans `ProbMort` comme étant des hasards.
 On pourrez bien dire que les probabiltés annuelles de mort dans `ProbMort`
 devrait être transformées en hasards
 en utilisant le formule 
    \(h_{x}=-\ln(1-q_{x})\).
 où 
 \(h_{x}\) est l'hasard de mort et \(q_{x}\) est le taux de mortalité annuel.
 