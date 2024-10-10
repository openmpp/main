### First union formation 

Risks are given as piece-wise constant rates changing with age. Again, we model age intervals of 2.5 years. These are the rates for women prior to any conception, as such an event would stop our simulation.  

**Table 2: First union formation risks** 

|table>
                              | before 1989 | transition | 10 years after | transition: 1999+
                              | Bulgaria    | Russia | Bulgaria           | Russia 
- | - | -
15-17.5 | 0.0309 | 0.0297 | 0.0173 | 0.0303 
17.5-20 | 0.1341 | 0.1342 | 0.0751 | 0.1369 
20-22.5 | 0.1672 | 0.1889 | 0.0936 | 0.1926 
22.5-25 | 0.1656 | 0.1724 | 0.0927 | 0.1758 
25-27.5 | 0.1474 | 0.1208 | 0.0825 | 0.1232 
27.5-30 | 0.1085 | 0.1086 | 0.0607 | 0.1108 
30-32.5 | 0.0804 | 0.0838 | 0.0450 | 0.0855 
32.5-35 | 0.0339 | 0.0862 | 0.0190 | 0.0879 
35-37.5 | 0.0455 | 0.0388 | 0.0255 | 0.0396 
37.5-40 | 0.0400 | 0.0324 | 0.0224 | 0.0330 
|<table
 
The parameterization example given in Table 2 has the following interpretation: the first union formation hazard of Bulgarian women of the first cohort is 0 until the 15th birthday; afterwards it changes in time steps of 2.5 years from 0.0309 to 0.1341, then from 0.1341 to 0.1672, and so on. The risk is highest for the age group 20-22.5--at a rate of 0.1672, the expected time to union formation is 1/0.1672=6 years. A women who is single on her 20th birthday has a 34% probability of experiencing a first union formation in the following 2.5 years (p=1-exp(-0.1672*2.5)).  


[[Home](#Home)] [[<< Prev](#003-First-pregnancy)] [[Next >>](#005-Second-union-formation)]