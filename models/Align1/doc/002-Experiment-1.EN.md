### Experiment 1

This experiment sets target mortality to 122 in each of the 20 years. 122 is the average annual mortality in the 20 years in the Default run with no alignment.  The first column shows mortality counts in the Default run with no alignment.  One observes a secular decrease (with some noise) because the population is progressively smaller so fewer die each year due to the lower base population (the morality hazard is constant).  The target column, in contrast, sets the number of deaths to a fixed value in each year. Compared to the Default run, deaths need to be decreased in early years and increased in later years to hit the target.

|table>

Year|Mortality (Default)|Target|Mortality (Aligned)|Events Deferred|Events Advanced
- | - | -
0   |141    |122    |122    |19 |0
1	|144	|122	|122	|41	|0
2	|114	|122	|122	|33	|0
3	|146	|122	|122	|57	|0
4	|142	|122	|122	|77	|0
5	|126	|122	|122	|81	|0
6	|129	|122	|122	|88	|0
7	|139	|122	|122	|105	|0
8	|111	|122	|122	|94	|0
9	|118	|122	|122	|90	|0
10	|112	|122	|122	|80	|0
11	|118	|122	|122	|76	|0
12	|102	|122	|122	|56	|0
13	|110	|122	|122	|44	|0
14	|104	|122	|122	|26	|0
15	|116	|122	|122	|20	|0
16	|114	|122	|122	|12	|0
17	|124	|122	|122	|14	|0
18	|112	|122	|122	|4	|0
19	|112	|122	|122	|0	|6
|<table
The algorithm attained the annual targets by deferring mortality events in each of the first 19 years and advancing 6 mortality events for the final year.  Mortality events were advanced only in the final year of the run because for all other years sufficient deaths had been deferred in previous years.
The annual targets are hit exactly because event times are not recalculated during the year-in-progress in this model.


[[Home](#Home)] [[<< Previous](#001-Introduction)] [[Next >>](#003-Experiment-2)]