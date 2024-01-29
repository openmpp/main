* The values in `ProbMort` have no effect if [`CanDie`](#CanDie) is `false`.
* The [GetLabel(Person::DeathEvent)](#DeathEvent) function in `RiskPaths`
 treats the values in `ProbMort` as though they are hazards.
 Arguably, the annual probabilities of death in `ProbMort` should be converted to hazards
 using the formula 
    \(h_{x}=-\ln(1-q_{x})\).
 where 
 \(h_{x}\) is the hazard of death and \(q_{x}\) is the annual mortality rate.
 