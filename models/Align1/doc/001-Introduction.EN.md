### Introduction

The `Align1` model is a proof of concept and testbed for dynamic alignment of time-based models using event queue look-ahead.  `Align1` steers itself to aggregate annual targets by reading and modifying the event queue dynamically at the beginning of each year.

`Align1` is based on the `NewTimeBased` model which is part of the OpenM++ distribution. 
It adds alignment apparatus in a new module `Alignment.mpp` and associated parameters in `Alignment.dat` but is otherwise unmodified. 
`NewTimeBased` has a `Ticker` entity with a timekeeping `TickEvent` and `Person` entities with `Mortality` events. 
`Align1` adds a new event `AlignmentEvent` to `Ticker`, 
which occurs at the same time as `TickEvent` but at lower priority. 
The input parameter 
[MortalityAlignmentTarget](#MortalityAlignmentTarget) 
contains target mortality counts by year. 
At the beginning of each year `AlignmentEvent` reads the event queue and counts the number of deaths scheduled to occur during that year. 
If the count is higher than the target for the year, 
enough scheduled events are deferred to the subsequent year to hit the target. 
If the count is lower than the target, 
enough scheduled events are advanced from subsequent years to the current year to hit the target. 
The exact rules are mechanical and described in code comments. 
This process is repeated at the beginning of each year.

The events which are deferred or advanced to hit alignment targets are those which are closest to the upper boundary of the current alignment year.

The Default run has 10,000 Persons and sets the parameter
[MortalityAlignmentTarget](#MortalityAlignmentTarget) 
to the mortality counts which would occur in the absence of alignment (using the table
[MortalityCounts](#MortalityCounts) 
from a previous run with 
[EnableAlignment](#EnableAlignment) 
turned off).


[[Home](#Home)] [[Next >>](#002-Experiment-1)]