### Experiment 2

This second experiment explored computational cost and scaling behaviour, using a run with 10 million `Person` entities.
With this population size, there were over 100,000 mortality events per year.
Three runs were done.

|table>
Run|Description|Time
- | - | -
1	|No alignment	|1m25s
2	|Alignment with targets=actual	|1m27s
3	|Alignment with targets=+/- 5% actual	|1m26s
|<table

Run 1 had alignment disabled.
Run 2 had alignment enabled, but the targets were the same as the results without alignment.  So, no adjustment of the event queue was done, but the event queue was probed each year.
Run 3 had random targets within +/- 5% of the original mortality results.  So alignment was doing some work with the event queue to hit the targets in run 3.

The table shows that the run times were indistinguishable.  For this model, anyway, the incremental cost of alignment was barely detectable.


[[Home](#Home)] [[<< Previous](#002-Experiment-1)] [[Next >>](#004-Remarks)]